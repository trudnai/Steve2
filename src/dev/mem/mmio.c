//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019, 2020 Tamas Rudnai. All rights reserved.
//
// This file is part of Steve ][ -- The Apple ][ Emulator.
//
// Steve ][ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Steve ][ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Steve ][.  If not, see <https://www.gnu.org/licenses/>.
//


#include "mmio.h"
#include "common.h"
#include "6502.h"
#include "disk.h"
#include "woz.h"
#include "speaker.h"
#include "paddle.h"


videoMode_t videoMode = { 1 }; // 40 col text, page 1


uint8_t INT_64K_ROM[ 64 * KB ] = {0};           // ROM C0, C8, D0, D8, E0, E8, F0, F8
uint8_t AUX_64K_ROM[ 64 * KB ] = {0};           // ROM C0, C8, D0, D8, E0, E8, F0, F8
uint8_t EXP_64K_ROM[ 64 * KB ] = {0};           // ROM C0, C8, D0, D8, E0, E8, F0, F8

uint8_t Apple2_Dummy_Page[ 1 * PG ];            // Dummy Page to discard data
uint8_t Apple2_Dummy_RAM[ 64 * KB ];            // Dummy RAM to discard data

uint8_t Apple2_64K_AUX[ 64 * KB ] = {0};        // 64K Expansion Memory
uint8_t Apple2_64K_RAM[ 64 * KB ] = {0};        // Main Memory
uint8_t Apple2_64K_MEM[ 64 * KB ] = {0};        // Shadow Copy of Memory (or current memory content)

//uint8_t * AUX_VID_RAM = Apple2_VID_AUX;       // Pointer to Auxiliary Video Memory
uint8_t * const AUX = Apple2_64K_AUX;           // Pointer to the Auxiliary Memory so we can use this from Swift
uint8_t * const RAM = Apple2_64K_RAM;           // Pointer to the Main Memory so we can use this from Swift
uint8_t * const MEM = Apple2_64K_MEM;           // Pointer to the Shadow Memory Map so we can use this from Swift

uint8_t * const RDLOMEM = Apple2_64K_MEM;       // for Read $0000 - $BFFF (shadow memory)
uint8_t *       WRZEROPG= Apple2_64K_MEM;       // for Write $0000 - $0200 (shadow memory)
uint8_t *       WRLOMEM = Apple2_64K_MEM;       // for Write $0200 - $BFFF (shadow memory)
uint8_t * const RDHIMEM = Apple2_64K_MEM;       // for Read / Write $0000 - $BFFF (shadow memory)
uint8_t *       WRD0MEM = Apple2_Dummy_RAM;     // for writing $D000 - $DFFF
uint8_t *       WRHIMEM = Apple2_Dummy_RAM;     // for writing $E000 - $FFFF


uint8_t writeState = 0;                  // 1 if $C08D was written

uint8_t * current_RAM_bank = Apple2_64K_AUX + 0xC000;

uint8_t activeTextAuxPage = 0;
uint8_t * activeTextPage = Apple2_64K_RAM + 0x400;
uint8_t * shadowTextPage = Apple2_64K_MEM + 0x400;

unsigned int lastIO = 0;


#define INIT_MEMCFG { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

const MEMcfg_t initMEMcfg = INIT_MEMCFG;

MEMcfg_t MEMcfg = INIT_MEMCFG;
MEMcfg_t newMEMcfg = INIT_MEMCFG;


const uint8_t * const shadowLowMEM = Apple2_64K_MEM + 0x200;
const uint8_t * currentLowMEM = Apple2_64K_RAM + 0x200;


/// No writing (Readonly), and mark it as NO need to commit from Shadow RAM
INLINE void set_MEM_readonly() {
    printf("NOWR_AUX\n");
    
    MEMcfg.WR_RAM = 0;
    WRD0MEM = Apple2_Dummy_RAM;   // for Discarding any writes to $D000 - $DFFF - BANK X
    WRHIMEM = Apple2_Dummy_RAM;   // for Discarding any writes to $E000 - $FFFF
}


/// Make AUX RAM writeable -- This is when AUX is also readable, othwrwise use set_AUX_write...
INLINE void set_MEM_write() {
    // two consecutive read or write needs for write enable
    // Note: if it is already writeable and was previously a ROM read + RAM write, then we also need to bound AUX to MEM
    if ( ( (m6502.clktime + m6502.clkfrm - m6502.clk_wrenable) < 8 ) || (MEMcfg.WR_RAM) ) {
        printf("WR_AUX\n");
        
        // will write to Shadow RAM, and mark it as need to commit from Shadow RAM
        MEMcfg.WR_RAM = 1;
        WRD0MEM = Apple2_64K_MEM;   // for Write $D000 - $DFFF (shadow memory) - BANK X
        WRHIMEM = Apple2_64K_MEM;   // for Write $E000 - $FFFF (shadow memory)
    }

    m6502.clk_wrenable = m6502.clktime + m6502.clkfrm;
}


/// Make AUX RAM writeable -- This is when ROM is readable, othwrwise use set_MEM_write...
INLINE void set_AUX_write() {
    // will write directly to Auxiliary RAM, and mark it as NO need to commit from Shadow RAM
    // Note: if it is already writeable and was previously a RAM read + RAM write, then we also need to bound AUX to MEM
    if ( ( (m6502.clktime + m6502.clkfrm - m6502.clk_wrenable) < 8 ) || (MEMcfg.WR_RAM) ) {
        printf("WR_AUX\n");
        
        MEMcfg.WR_RAM = 1;
        if ( MEMcfg.RAM_BANK_2 ) {
            WRD0MEM = Apple2_64K_AUX;   // for Write $D000 - $DFFF (shadow memory) - BANK 2 at 0xD000
        }
        else {
            WRD0MEM = Apple2_64K_AUX - 0x1000;   // for Write $D000 - $DFFF (shadow memory) - BANK 1 at 0xC000
        }
        WRHIMEM = Apple2_64K_AUX;   // for Write $E000 - $FFFF (shadow memory)
    }
    
    m6502.clk_wrenable = m6502.clktime + m6502.clkfrm;
}


INLINE void io_RAM_EXP( uint16_t addr ) {
    
    if ( MEMcfg.RAM_16K || MEMcfg.RAM_128K ) {
        // TODO: store 0xD000 BANK 1 at 0xC000 -- might be a problem emulating 64k/128k Saturn cards
//        uint8_t * RAM_BANK = Apple2_64K_AUX + 0xC000;
        
        // save the content of Shadow Memory in needed
        if ( MEMcfg.WR_RAM && MEMcfg.RD_INT_RAM ) {
            //                    printf("Saving RAM Bank %d to %p\n", MEMcfg.RAM_BANK_2 + 1, current_RAM_bank);
            memcpy(current_RAM_bank, Apple2_64K_MEM + 0xD000, 0x1000);
            memcpy(Apple2_64K_AUX + 0xE000, Apple2_64K_MEM + 0xE000, 0x2000);
        }
        
        // RAM Bank 1 or 2?
        switch ((uint8_t)addr) {
            case (uint8_t)io_MEM_RDRAM_NOWR_2:
            case (uint8_t)io_MEM_RDROM_WRAM_2:
            case (uint8_t)io_MEM_RDROM_NOWR_2:
            case (uint8_t)io_MEM_RDRAM_WRAM_2:
                
            case (uint8_t)io_MEM_RDRAM_NOWR_2_:
            case (uint8_t)io_MEM_RDROM_WRAM_2_:
            case (uint8_t)io_MEM_RDROM_NOWR_2_:
            case (uint8_t)io_MEM_RDRAM_WRAM_2_:
                
                printf("RAM_BANK_2\n");
                
                MEMcfg.RAM_BANK_2 = 1;
                current_RAM_bank = Apple2_64K_AUX + 0xD000;
                break;
                
            default:
                printf("RAM_BANK_1\n");
                
                MEMcfg.RAM_BANK_2 = 0;
                current_RAM_bank = Apple2_64K_AUX + 0xC000;
                break;
        }
        
        // is RAM to read or ROM?
        switch ((uint8_t)addr) {
            case (uint8_t)io_MEM_RDRAM_NOWR_2:
            case (uint8_t)io_MEM_RDRAM_WRAM_2:
            case (uint8_t)io_MEM_RDRAM_NOWR_1:
            case (uint8_t)io_MEM_RDRAM_WRAM_1:
                
            case (uint8_t)io_MEM_RDRAM_NOWR_2_:
            case (uint8_t)io_MEM_RDRAM_WRAM_2_:
            case (uint8_t)io_MEM_RDRAM_NOWR_1_:
            case (uint8_t)io_MEM_RDRAM_WRAM_1_:
                
                printf("RD_RAM\n");
                
                MEMcfg.RD_INT_RAM = 1;
                
                // load the content of Aux Memory
                memcpy(Apple2_64K_MEM + 0xD000, current_RAM_bank, 0x1000);
                memcpy(Apple2_64K_MEM + 0xE000, Apple2_64K_AUX + 0xE000, 0x2000);
                
                // set the RAM extension to read on the upper memory area
                break;
                
            default:
                printf("RD_ROM\n");
                
                MEMcfg.RD_INT_RAM = 0;
                
                // load the content of ROM Memory
                memcpy(Apple2_64K_MEM + 0xD000, INT_64K_ROM + 0xD000, 0x3000);
                
                // set the ROM to read on the upper memory area
                break;
        }
        
        // is RAM Writeable?
        switch ((uint8_t)addr) {
            case (uint8_t)io_MEM_RDROM_WRAM_2:
            case (uint8_t)io_MEM_RDROM_WRAM_1:
                
            case (uint8_t)io_MEM_RDROM_WRAM_2_:
            case (uint8_t)io_MEM_RDROM_WRAM_1_:
                
                printf("RD_ROM + WR_AUX\n");

                set_AUX_write();
                
                break;
                
            case (uint8_t)io_MEM_RDRAM_WRAM_2:
            case (uint8_t)io_MEM_RDRAM_WRAM_1:
                
            case (uint8_t)io_MEM_RDRAM_WRAM_2_:
            case (uint8_t)io_MEM_RDRAM_WRAM_1_:
                
                printf("RD_RAM + WR_RAM\n");
                
                set_MEM_write();

                break;
                
            default:
                printf("RD_ROM + NO_WR\n");

                set_MEM_readonly();

                break;
        }
        
//        current_RAM_bank = RAM_BANK;
        //                printf("Set current_RAM_bank %d to %p\n", MEMcfg.RAM_BANK_2 + 1, current_RAM_bank);
        
        
    } // if there is RAM expansion card installed
    
}


INLINE uint8_t ioRead( uint16_t addr ) {
    //    if (outdev) fprintf(outdev, "ioRead:%04X\n", addr);
    
//    switch(addr) {
//        case io_KBD:
//        case io_KBDSTRB:
//        case io_SPKR:
//            break;
//        default:
//            printf("ioRead:%04X (PC:%04X)\n", addr, m6502.PC);
//    }
    
    unsigned int IOframe = m6502.clkfrm - lastIO;
    lastIO = m6502.clkfrm;
    
    // TODO: This is for speed demo only, should be either removed or the entire ioRead should based on binary search, whatever is faster
    if ( addr == io_KBD ) {
        //        clk_6502_per_frm_max = clk_6502_per_frm_max > 32768 ? clk_6502_per_frm_max - 32768 : 0; // ECO Mode!
        
        if ( cpuMode == cpuMode_eco ) {
            // check if this is a busy keyboard poll (aka waiting for user input)
            if ( IOframe < 16 ) {
                clk_6502_per_frm_max = 6502; // Let it run for a bit to display character -- nerd number :-)
                cpuState = cpuState_halting;
            }
        }
        
        return Apple2_64K_RAM[io_KBD];
    }
    
    switch ( (uint8_t)addr ) {
//        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
            printf("RD TAPEIO: %04X\n", addr);
            return 0;
            
        case (uint8_t)io_KBD:
            
            return Apple2_64K_RAM[io_KBD];
            
        case (uint8_t)io_KBDSTRB:
            Apple2_64K_RAM[io_KBD] &= ~(1 << 7);
            
            if ( cpuMode == cpuMode_eco ) {
                // check if this is a busy keyboard poll (aka waiting for user input)
                clk_6502_per_frm_max = clk_6502_per_frm; // Absolute low mode
                cpuState = cpuState_running;
            }
            
            return Apple2_64K_RAM[io_KBDSTRB];
            
        case (uint8_t)io_TAPEOUT:
        case (uint8_t)io_SPKR:
            spkr_toggle();
            return Apple2_64K_RAM[io_SPKR];
            
        case (uint8_t)io_VID_RDVBL:
            return (m6502.clkfrm < 4550) ? 0x80 : 0;
            
        case (uint8_t)io_VID_RDTEXT:
            return videoMode.text << 7;
            
        case (uint8_t)io_VID_ALTCHAR:
            return videoMode.altChr << 7;
            
        case (uint8_t)io_VID_RD80VID:
            return videoMode.col80 << 7;
            
        case (uint8_t)io_TAPEIN:
            // TODO: this should be only on //c
            return MEMcfg.txt_page_2 << 7;
            
        case (uint8_t)io_RDCXROM:
            return MEMcfg.int_Cx_ROM << 7;
            
        case (uint8_t)io_RDLCBNK2:
            return MEMcfg.RAM_BANK_2 << 7;
            
        case (uint8_t)io_RDLCRAM:
            return MEMcfg.RD_INT_RAM << 7;
            
        case (uint8_t)io_RDRAMRD:
            return MEMcfg.RD_AUX_MEM << 7;
            
        case (uint8_t)io_RDRAMWR:
            return MEMcfg.WR_AUX_MEM << 7;
            
        case (uint8_t)io_RDALTZP:
            return MEMcfg.ALT_ZP << 7;
                
        case (uint8_t)io_RDC3ROM:
            return MEMcfg.slot_C3_ROM << 7;
            
        case (uint8_t)io_RD80STORE:
            return MEMcfg.is_80STORE << 7;
            
        case (uint8_t)io_VID_TXTPAGE1:
            //            printf("io_VID_TXTPAGE1\n");
            MEMcfg.txt_page_2 = 0;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_TXTPAGE2:
            //            printf("io_VID_TXTPAGE2\n");
            MEMcfg.txt_page_2 = 1;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_RDPAGE2:
            return MEMcfg.txt_page_2 << 7;
            
        case (uint8_t)io_VID_Text_OFF:
            videoMode.text = 0;
            break;
            
        case (uint8_t)io_VID_Text_ON:
            videoMode.text = 1;
            break;
            
        case (uint8_t)io_VID_Mixed_OFF:
            videoMode.mixed = 0;
            break;
            
        case (uint8_t)io_VID_Mixed_ON:
            videoMode.mixed = 1;
            break;
            
        case (uint8_t)io_VID_RDMIXED:
            return videoMode.mixed << 7;
            
        case (uint8_t)io_VID_Hires_OFF:
            videoMode.hires = 0;
            break;
            
        case (uint8_t)io_VID_Hires_ON:
            videoMode.hires = 1;
            break;
            
        case (uint8_t)io_VID_RDHIRES:
            return videoMode.hires << 7;
            
        case (uint8_t)io_PDL0:
        case (uint8_t)io_PDL1:
        case (uint8_t)io_PDL2:
        case (uint8_t)io_PDL3:
            //            printf("PDL%d: %d\n", addr - io_PDL0, pdl_read( addr - io_PDL0 ));
            return pdl_read( addr - io_PDL0 );
            
        case (uint8_t)io_PDL_STROBE:
            return pdl_reset();
            
        case (uint8_t)io_RDMAINRAM:
            //            printf("io_RDMAINRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_RDCARDRAM:
            //            printf("io_RDCARDRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_WRMAINRAM:
            //            printf("io_WRMAINRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_WRCARDRAM:
            //            printf("io_WRCARDRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_MEM_RDRAM_NOWR_2:
        case (uint8_t)io_MEM_RDROM_WRAM_2:
        case (uint8_t)io_MEM_RDROM_NOWR_2:
        case (uint8_t)io_MEM_RDRAM_WRAM_2:
            
        case (uint8_t)io_MEM_RDRAM_NOWR_2_:
        case (uint8_t)io_MEM_RDROM_WRAM_2_:
        case (uint8_t)io_MEM_RDROM_NOWR_2_:
        case (uint8_t)io_MEM_RDRAM_WRAM_2_:
            
        case (uint8_t)io_MEM_RDRAM_NOWR_1:
        case (uint8_t)io_MEM_RDROM_WRAM_1:
        case (uint8_t)io_MEM_RDROM_NOWR_1:
        case (uint8_t)io_MEM_RDRAM_WRAM_1:
            
        case (uint8_t)io_MEM_RDRAM_NOWR_1_:
        case (uint8_t)io_MEM_RDROM_WRAM_1_:
        case (uint8_t)io_MEM_RDROM_NOWR_1_:
        case (uint8_t)io_MEM_RDRAM_WRAM_1_:
            io_RAM_EXP(addr);
            break;
            
            // TODO: Make code "card insertable to slot" / aka slot independent and dynamically add/remove
        case (uint8_t)io_DISK_PHASE0_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE1_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE2_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE3_OFF + SLOT6:
            disk_phase_off( (addr - io_DISK_PHASE0_OFF - SLOT6) / 2 );
            return disk_read();
            
        case (uint8_t)io_DISK_PHASE0_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE1_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE2_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE3_ON + SLOT6:
            disk_phase_on( (addr - io_DISK_PHASE0_ON - SLOT6) / 2 );
            return disk_read();
            
        case (uint8_t)io_DISK_POWER_OFF + SLOT6:
            dbgPrintf2("io_DISK_POWER_OFF (S%u)\n", 6);
            disk_motor_off();
            return disk_read();
            
        case (uint8_t)io_DISK_POWER_ON + SLOT6:
            dbgPrintf2("io_DISK_POWER_ON (S%u)\n", 6);
            disk_motor_on();
            return disk_read();
            
        case (uint8_t)io_DISK_SELECT_1 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_1 (S%u)\n", 6);
            disk.drive = 0;
            return disk_read();
            
        case (uint8_t)io_DISK_SELECT_2 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_2 (S%u)\n", 6);
            disk.drive = 1;
            return disk_read();
            
        case (uint8_t)io_DISK_READ + SLOT6:
            if ( writeState ) {
                writeState = 0;
                woz_write( Apple2_64K_RAM[io_DISK_WRITE + SLOT6] );
                return Apple2_64K_RAM[io_DISK_WRITE + SLOT6];
            }
            else {
                return disk_read();
            }
            
            
        case (uint8_t)io_DISK_WRITE + SLOT6:
            dbgPrintf2("io_DISK_WRITE (S%u)\n", 6);
            //            Apple2_64K_RAM[io_DISK_CLEAR + SLOT6] |= 1 << 7; // mark disk as write protected
            WOZwrite.latch = WOZread.latch = 0;
            Apple2_64K_RAM[io_DISK_CLEAR + SLOT6] &= ~(1 << 7); // mark disk as write enabled
            return Apple2_64K_RAM[io_DISK_WRITE + SLOT6];
            
        case (uint8_t)io_DISK_CLEAR + SLOT6:
            dbgPrintf2("io_DISK_CLEAR (S%u)\n", 6);
            return Apple2_64K_RAM[io_DISK_CLEAR + SLOT6];
            
        case (uint8_t)io_DISK_SHIFT + SLOT6:
            dbgPrintf2("io_DISK_SHIFT (S%u)\n", 6);
            return disk_read();
            
            
            
        default:
            //printf("mmio read:%04X\n", addr);
            break;
    }
    
    return Apple2_64K_RAM[addr];
}


INLINE void ioWrite( uint16_t addr, uint8_t val ) {
    //    if (outdev) fprintf(outdev, "ioWrite:%04X (A:%02X)\n", addr, m6502.A);
    
//    switch(addr) {
//        case io_KBD:
//        case io_KBDSTRB:
//        case io_SPKR:
//            break;
//        default:
//            printf("ioWrite:%04X (PC:%04X, val:%02X)\n", addr, m6502.PC, val);
//    }
    

    switch ( (uint8_t)addr ) {
//        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
            printf("WR TAPEIO: %04X\n", addr);
            return;
            
        case (uint8_t)io_KBDSTRB:
            Apple2_64K_RAM[io_KBD] &= 0x7F;
            break;
            
        case (uint8_t)io_TAPEOUT:
        case (uint8_t)io_SPKR:
            spkr_toggle();
            break;
            
        case (uint8_t)io_RDMAINRAM:
            //            printf("io_RDMAINRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_RDCARDRAM:
            //            printf("io_RDCARDRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_WRMAINRAM:
            //            printf("io_WRMAINRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_WRCARDRAM:
            //            printf("io_WRCARDRAM\n");
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETSTDZP:
            MEMcfg.ALT_ZP = 0;
            // TODO: set zero page table to RAM
            break;
            
        case (uint8_t)io_SETALTZP:
            MEMcfg.ALT_ZP = 1;
            // TODO: set zero page table to AUX
            break;
            
        case (uint8_t)io_SETSLOTCXROM:
            //            printf("io_SETSLOTCXROM\n");
            MEMcfg.int_Cx_ROM = 0;
            // TODO: set Cx00 ROM area table to SLOT
            break;
            
        case (uint8_t)io_SETINTCXROM:
            //            printf("io_SETINTCXROM\n");
            MEMcfg.int_Cx_ROM = 1;
            // TODO: set Cx00 ROM area table to INT
            break;
            
        case (uint8_t)io_SETSLOTC3ROM:
            //            printf("io_SETSLOTC3ROM\n");
            MEMcfg.slot_C3_ROM = 1;
            // TODO: set C300 ROM area table to SLOT
            break;
            
        case (uint8_t)io_SETINTC3ROM:
            //            printf("io_SETINTC3ROM\n");
            MEMcfg.slot_C3_ROM = 0;
            // TODO: set C300 ROM area table to INT
            break;
            
        case (uint8_t)io_VID_CLR80VID:
            //            printf("io_VID_CLR80VID\n");
            videoMode.col80 = 0;
            break;
            
        case (uint8_t)io_VID_SET80VID:
            videoMode.col80 = 1;
            break;
            
        case (uint8_t)io_VID_CLRALTCHAR:
            videoMode.altChr = 0;
            break;
            
        case (uint8_t)io_VID_SETALTCHAR:
            videoMode.altChr = 1;
            break;
            
        case (uint8_t)io_80STOREOFF:
            //            printf("io_80STOREOFF\n");
            MEMcfg.is_80STORE = 0;
            textPageSelect();
            break;
            
        case (uint8_t)io_80STOREON:
            //            printf("io_80STOREON\n");
            MEMcfg.is_80STORE = 1;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_TXTPAGE1:
            //            printf("io_VID_TXTPAGE1\n");
            MEMcfg.txt_page_2 = 0;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_TXTPAGE2:
            //            printf("io_VID_TXTPAGE2\n");
            MEMcfg.txt_page_2 = 1;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_Text_OFF:
            videoMode.text = 0;
            break;
            
        case (uint8_t)io_VID_Text_ON:
            videoMode.text = 1;
            break;
            
        case (uint8_t)io_VID_Mixed_OFF:
            videoMode.mixed = 0;
            break;
            
        case (uint8_t)io_VID_Mixed_ON:
            videoMode.mixed = 1;
            break;
            
        case (uint8_t)io_VID_Hires_OFF:
            videoMode.hires = 0;
            break;
            
        case (uint8_t)io_VID_Hires_ON:
            videoMode.hires = 1;
            break;
            
        case (uint8_t)io_MEM_RDRAM_NOWR_2:
        case (uint8_t)io_MEM_RDROM_WRAM_2:
        case (uint8_t)io_MEM_RDROM_NOWR_2:
        case (uint8_t)io_MEM_RDRAM_WRAM_2:
            
        case (uint8_t)io_MEM_RDRAM_NOWR_2_:
        case (uint8_t)io_MEM_RDROM_WRAM_2_:
        case (uint8_t)io_MEM_RDROM_NOWR_2_:
        case (uint8_t)io_MEM_RDRAM_WRAM_2_:
            
        case (uint8_t)io_MEM_RDRAM_NOWR_1:
        case (uint8_t)io_MEM_RDROM_WRAM_1:
        case (uint8_t)io_MEM_RDROM_NOWR_1:
        case (uint8_t)io_MEM_RDRAM_WRAM_1:
            
        case (uint8_t)io_MEM_RDRAM_NOWR_1_:
        case (uint8_t)io_MEM_RDROM_WRAM_1_:
        case (uint8_t)io_MEM_RDROM_NOWR_1_:
        case (uint8_t)io_MEM_RDRAM_WRAM_1_:
            io_RAM_EXP(addr);
            break;
            
            // TODO: Make code "card insertable to slot" / aka slot independent and dynamically add/remove
        case (uint8_t)io_DISK_PHASE0_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE1_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE2_OFF + SLOT6:
        case (uint8_t)io_DISK_PHASE3_OFF + SLOT6:
            disk_phase_off( (addr - io_DISK_PHASE0_OFF - SLOT6) / 2 );
            break;
            
        case (uint8_t)io_DISK_PHASE0_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE1_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE2_ON + SLOT6:
        case (uint8_t)io_DISK_PHASE3_ON + SLOT6:
            disk_phase_on( (addr - io_DISK_PHASE0_ON - SLOT6) / 2 );
            break;
            
        case (uint8_t)io_DISK_POWER_OFF + SLOT6:
            dbgPrintf2("io_DISK_POWER_OFF (S%u)\n", 6);
            disk_motor_off();
            break;
            
        case (uint8_t)io_DISK_POWER_ON + SLOT6:
            dbgPrintf2("io_DISK_POWER_ON (S%u)\n", 6);
            disk_motor_on();
            break;
            
        case (uint8_t)io_DISK_SELECT_1 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_1 (S%u)\n", 6);
            disk.drive = 0;
            break;
            
        case (uint8_t)io_DISK_SELECT_2 + SLOT6:
            dbgPrintf2("io_DISK_SELECT_2 (S%u)\n", 6);
            disk.drive = 1;
            break;
            
        case (uint8_t)io_DISK_READ + SLOT6:
            Apple2_64K_RAM[io_DISK_READ + SLOT6] = val;
            woz_write( Apple2_64K_RAM[io_DISK_WRITE + SLOT6] );
            writeState = 0;
            break;
            
        case (uint8_t)io_DISK_WRITE + SLOT6:
            dbgPrintf2("io_DISK_WRITE (S%u)\n", 6);
            Apple2_64K_RAM[io_DISK_WRITE + SLOT6] = val;
            writeState = 1;
            break;
            
        case (uint8_t)io_DISK_CLEAR + SLOT6:
            dbgPrintf2("io_DISK_CLEAR (S%u)\n", 6);
            break;
            
        case (uint8_t)io_DISK_SHIFT + SLOT6:
            dbgPrintf2("io_DISK_SHIFT (S%u)\n", 6);
            break;
            
            
        default:
            break;
    }
    return;
}


/**
 Naive implementation of RAM read from address
 **/
INLINE uint8_t memread8_low( uint16_t addr ) {
    return Apple2_64K_MEM[addr];
}
INLINE uint8_t memread8_high( uint16_t addr ) {
    return RDHIMEM[addr];
}
INLINE uint8_t memread8( uint16_t addr ) {
    if (addr >= 0xC000) {
        return memread8_high(addr);
    }
    
    return memread8_low(addr);
}
/**
 Naive implementation of RAM read from address
 **/
INLINE uint16_t memread16_low( uint16_t addr ) {
    return * (uint16_t*) ( Apple2_64K_MEM + addr );
}
//INLINE uint16_t memread16_high( uint16_t addr ) {
//    return * (uint16_t*) ( RDHIMEM + addr );
//}
INLINE uint16_t memread16( uint16_t addr ) {
    
    //    if (addr >= 0xC000) {
    //        return memread16_high(addr);
    //    }
    
    return memread16_low(addr);
}

INLINE uint8_t memread( uint16_t addr ) {
    if (addr >= 0xC000) {
        if (addr < 0xC100) {
            return ioRead(addr);
        }
        
        //        return memread8_paged(addr);
        return memread8_high(addr);
    }
    
    //    return memread8_paged(addr);
    return memread8_low(addr);
    
    //    return memread8(addr);
}


/**
 Naive implementation of RAM read from address
 **/
//INLINE uint16_t memioread16( uint16_t addr ) {
//    return (uint16_t)mmio_read[ addr ](addr);
//}


/**
 Naive implementation of RAM write to address
 **/
//static  void memwrite_zp( uint8_t addr, uint8_t byte ) {
//    RAM[ addr ] = byte;
//}


/**
 Naive implementation of RAM write to address
 **/

INLINE void memwrite8_low( uint16_t addr, uint8_t data ) {
    WRLOMEM[addr] = data;
}
INLINE void memwrite8_bank( uint16_t addr, uint8_t data ) {
    WRD0MEM[addr] = data;
}
INLINE void memwrite8_high( uint16_t addr, uint8_t data ) {
    WRHIMEM[addr] = data;
}
INLINE void memwrite( uint16_t addr, uint8_t data ) {
    if (addr >= 0xC000) {
        if (addr < 0xC100) {
            ioWrite(addr, data);
        }
        else if (addr < 0xD000) {
            // this could be either Peripherial ROM or Internal ROM
            memwrite8_high(addr, data);
        }
        else if (addr < 0xE000) {
            // Aux RAM Bank 1 or 2
            memwrite8_bank(addr, data);
        }
        else {
            // ROM (dummy memory to screape writings) or Aux RAM
            memwrite8_high(addr, data);
        }
    }
    else {
        // RAM
        memwrite8_low(addr, data);
    }
    
}

/**
 Fetching 1 byte from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint8_t fetch() {
    disHexB( disassembly.pOpcode, memread(m6502.PC) );
#ifdef CLK_ABSOLUTE_PRECISE
    if ( (m6502.PC & 0xFF) >= 0xFF ) {
        m6502.clktime++;
    }
#endif
    return memread8_low( m6502.PC++ );
}

/**
 Fetching 2 bytes as a 16 bit number from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint16_t fetch16() {
    uint16_t word = memread16( m6502.PC );
    // disPrintf(disassembly.comment, "fetch16:%04X", word);
#ifdef CLK_ABSOLUTE_PRECISE
    if ( (m6502.PC & 0xFF) >= 0xFE ) {
        m6502.clktime++;
    }
#endif
    m6502.PC += 2;
    //    disHexW( disassembly.pOpcode, word );
    
    // Virtual ][ Style
    disHexB( disassembly.pOpcode, (uint8_t)word );
    disHexB( disassembly.pOpcode, (uint8_t)(word >> 8));
    
    return word;
}




//INLINE uint8_t * dest( uint8_t * mem, uint16_t addr ) {
//    return mem + addr;
//}


/**
 abs        ....    absolute         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t addr_abs() {
    dbgPrintf("abs:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]), RAM[*((uint16_t*)&RAM[m6502.PC])]);
    disPrintf(disassembly.oper, "$%04X", memread16(m6502.PC))
    return fetch16();
}
INLINE uint8_t src_abs() {
    return memread( addr_abs() );
}
//INLINE uint8_t * dest_abs() {
//    return WRLOMEM + addr_abs();
//}


INLINE int8_t rel_addr() {
    disPrintf(disassembly.oper, "$%04X", m6502.PC + 1 + (int8_t)memread8(m6502.PC))
    return fetch();
}
INLINE uint16_t abs_addr() {
    disPrintf(disassembly.oper, "$%04X", memread16(m6502.PC))
    return fetch16();
}
INLINE uint16_t ind_addr() {
    disPrintf(disassembly.oper, "($%04X)", memread16(m6502.PC))
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16(memread16(m6502.PC)))
    return memread16( fetch16() );
}

/**
 abs,X        ....    absolute, X-indexed         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t addr_abs_X() {
    dbgPrintf("abs,X:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]) + m6502.X, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.X]);
    disPrintf(disassembly.oper, "$%04X,X", memread16(m6502.PC));
    return fetch16() + m6502.X;
}
INLINE uint8_t src_abs_X() {
    return memread( addr_abs_X() );
}
//INLINE uint8_t * dest_abs_X() {
//    return WRLOMEM + addr_abs_X();
//}


/**
 abs,Y        ....    absolute, Y-indexed         OPC $LLHH,Y
 operand is address; effective address is address incremented by Y with carry **
 **/
INLINE uint16_t addr_abs_Y() {
    dbgPrintf("abs,Y:%04X(%02X) ", *((uint16_t*)&RAM[m6502.PC]) + m6502.Y, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.Y]);
    disPrintf(disassembly.oper, "$%04X,Y", memread16(m6502.PC))
    return fetch16() + m6502.Y;
}
INLINE uint8_t src_abs_Y() {
    return memread(addr_abs_Y());
}
//INLINE uint8_t * dest_abs_Y() {
//    return WRLOMEM + addr_abs_Y();
//}

INLINE uint8_t imm() {
    disPrintf(disassembly.oper, "#$%02X", memread8(m6502.PC))
    return fetch();
}


/**
 zpg        ....    zeropage         OPC $LL
 operand is zeropage address (hi-byte is zero, address = $00LL)
 **/
INLINE uint8_t addr_zp() {
    dbgPrintf("zp:%02X(%02X) ", RAM[m6502.PC], RAM[ RAM[m6502.PC]] );
    disPrintf(disassembly.oper, "$%02X", memread8(m6502.PC))
    return fetch();
}
INLINE uint8_t src_zp() {
    return memread8_low(addr_zp());
}
//INLINE uint8_t * dest_zp() {
//    return WRLOMEM + addr_zp();
//}

/**
 get a 16 bit address from the zp:zp+1
 **/
//INLINE uint16_t addr_zp_ind( uint8_t addr ) {
//    dbgPrintf("zpi:%02X:%04X(%02X) ", RAM[m6502.PC], *((uint16_t*)&RAM[m6502.PC]), RAM[*((uint16_t*)&RAM[m6502.PC])]);
//    disPrintf(disassembly.oper, "($%02X)", memread8(m6502.PC) );
//    disPrintf(disassembly.comment, "ind_addr:%04X", memread16( memread8(m6502.PC) ) );
//    return memread16(addr);
//}

/**
 X,ind        ....    X-indexed, indirect         OPC ($LL,X)
 operand is zeropage address;
 effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
 **/
INLINE uint16_t addr_ind_X() {
    dbgPrintf("zpXi:%02X:%04X(%02X) ", RAM[m6502.PC], *((uint16_t*)&RAM[m6502.PC]) + m6502.X, RAM[*((uint16_t*)&RAM[m6502.PC]) + m6502.X]);
    disPrintf(disassembly.oper, "($%02X,X)", memread8(m6502.PC) )
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16( memread8(m6502.PC) + m6502.X) );
    return memread16( fetch() + m6502.X );
}
INLINE uint8_t src_X_ind() {
    return memread( addr_ind_X() );
}
//INLINE uint8_t * dest_X_ind() {
//    return WRLOMEM + addr_ind_X();
//}

/**
 ind,Y        ....    indirect, Y-indexed         OPC ($LL),Y
 operand is zeropage address;
 effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
 **/
INLINE uint16_t addr_ind_Y() {
    //    uint8_t a = fetch();
    //    dbgPrintf("addr_ind_Y: %04X + %02X = %04X ", addr_zpg_ind( a ), m6502.Y, addr_zpg_ind( a ) + m6502.Y);
    disPrintf(disassembly.oper, "($%02X),Y", memread8(m6502.PC) )
    disPrintf(disassembly.comment, "ind_addr:%04X", memread16( memread8(m6502.PC) ) + m6502.Y );
    return memread16( fetch() ) + m6502.Y;
}
INLINE uint8_t src_ind_Y() {
    return memread( addr_ind_Y() );
}
//INLINE uint8_t * dest_ind_Y() {
//    return WRLOMEM + addr_ind_Y();
//}

/**
 zpg,X        ....    zeropage, X-indexed         OPC $LL,X
 operand is zeropage address;
 effective address is address incremented by X without carry **
 **/
INLINE uint8_t addr_zp_X() {
    disPrintf(disassembly.oper, "$%02X,X", memread8(m6502.PC))
    return fetch() + m6502.X;
}
INLINE uint8_t src_zp_X() {
    return memread8_low(addr_zp_X());
}
//INLINE uint8_t * dest_zp_X() {
//    return WRLOMEM + addr_zp_X();
//}

/**
 zpg,Y        ....    zeropage, Y-indexed         OPC $LL,Y
 operand is zeropage address;
 effective address is address incremented by Y without carry **
 **/
INLINE uint8_t addr_zp_Y() {
    disPrintf(disassembly.oper, "$%02X,Y", memread8(m6502.PC))
    return fetch() + m6502.Y;
}
INLINE uint8_t src_zp_Y() {
    return memread8_low(addr_zp_Y());
}
//INLINE uint8_t * dest_zp_Y() {
//    return WRLOMEM + addr_zp_Y();
//}


void auxMemorySelect( MEMcfg_t newMEMcfg ) {
    const uint8_t * newLowMEM = currentLowMEM;
    
    if ( newMEMcfg.is_80STORE ) {
        if ( newMEMcfg.RD_AUX_MEM ) {
            newLowMEM = Apple2_64K_AUX + 0x200;
        }
        else {
            newLowMEM = Apple2_64K_RAM + 0x200;
        }
        
        if ( newMEMcfg.WR_AUX_MEM ) {
            if ( newMEMcfg.RD_INT_RAM ) {
                WRLOMEM = Apple2_64K_AUX;
            }
            else {
                WRLOMEM = Apple2_64K_MEM;
            }
        }
        else {
            if ( newMEMcfg.RD_INT_RAM ) {
                WRLOMEM = Apple2_64K_MEM;
            }
            else {
                WRLOMEM = Apple2_64K_RAM;
            }
        }
    }
    else {
        newLowMEM = Apple2_64K_RAM + 0x200;
        WRLOMEM = Apple2_64K_MEM;
    }
    
    // load new content to shadow memory
    if ( newLowMEM != currentLowMEM ) {
        // save the content of Shadow Memory
        memcpy( (void*) currentLowMEM, shadowLowMEM, 0xBE00);
        // page in the new memory area
        memcpy( (void*) shadowLowMEM, newLowMEM, 0xBE00);
        // mark new as the current one
        currentLowMEM = newLowMEM;
    }
    
    MEMcfg = newMEMcfg;
}


void C3MemorySelect( MEMcfg_t newMEMcfg ) {
    
    if ( newMEMcfg.slot_C3_ROM ) {
        // load internal ROM to memory
        memcpy(Apple2_64K_MEM + 0xC300, INT_64K_ROM + 0xC300, 0x100);
    }
    else {
        // load peripheral ROM to memory
        memcpy(Apple2_64K_MEM + 0xC300, EXP_64K_ROM + 0xC300, 0x100);
    }
    
    MEMcfg = newMEMcfg;
}


void CxMemorySelect( MEMcfg_t newMEMcfg ) {
    
    if ( newMEMcfg.int_Cx_ROM ) {
        // load internal ROM to memory
        memcpy(Apple2_64K_MEM + 0xC100, INT_64K_ROM + 0xC100, 0xF00);
    }
    else {
        // load peripheral ROM to memory
        memcpy(Apple2_64K_MEM + 0xC100, EXP_64K_ROM + 0xC100, 0xF00);
        //        memcpy(Apple2_64K_MEM + 0xC600, Apple2_64K_RAM + 0xC600, 0x100);
    }
    
    C3MemorySelect( newMEMcfg );
    
    MEMcfg = newMEMcfg;
}


void resetMemory() {
    newMEMcfg = initMEMcfg;
    
    WRZEROPG= Apple2_64K_MEM;       // for Write $0000 - $0200 (shadow memory)
    WRLOMEM = Apple2_64K_MEM;       // for Write $0200 - $BFFF (shadow memory)
    WRD0MEM = Apple2_Dummy_RAM;     // for writing $D000 - $DFFF
    WRHIMEM = Apple2_Dummy_RAM;     // for writing $E000 - $FFFF
    
    auxMemorySelect(MEMcfg);
    CxMemorySelect(MEMcfg);

    // initializing disk controller
    memcpy(Apple2_64K_MEM + 0xC600, EXP_64K_ROM + 0xC600, 0x100);
    
    MEMcfg = newMEMcfg;
    
    videoMode.text = 1;
    videoMode.col80 = 0;
}


void initMemory() {
    // Aux Video Memory
    memset( Apple2_64K_AUX, 0, sizeof(Apple2_64K_AUX) );
    // 64K Main Memory Area
    memset( Apple2_64K_RAM, 0, sizeof(Apple2_64K_RAM) );
    memset( Apple2_64K_MEM, 0, sizeof(Apple2_64K_MEM) );
    // text memory should be filled by spaces
    memset( Apple2_64K_AUX + 0x400, 0xA0, 0x800 );
    memset( Apple2_64K_RAM + 0x400, 0xA0, 0x800 );
    memset( Apple2_64K_MEM + 0x400, 0xA0, 0x800 );
    // I/O area should be 0 -- just in case we decide to init RAM with a different pattern...
    memset( Apple2_64K_RAM + 0xC000, 0, 0x1000 );
    
    resetMemory();
}


inline uint8_t *extracted() {
    uint8_t * shadow = Apple2_64K_MEM + 0x400;
    return shadow;
}


void textPageSelect() {
    uint8_t textAuxPage = MEMcfg.is_80STORE && MEMcfg.txt_page_2;
    
    if ( activeTextAuxPage != textAuxPage ) {
        activeTextAuxPage = textAuxPage;
        uint8_t * newTextPage = ( textAuxPage ? Apple2_64K_AUX : Apple2_64K_RAM ) + 0x400;

        if ( activeTextPage ) {
            // save the content of Shadow Memory
            memcpy(activeTextPage, shadowTextPage, 0x400);
        }
        
        // load the content of new Video Page
        memcpy(shadowTextPage, newTextPage, 0x400);
        
        activeTextPage = newTextPage;
    }
}


// TODO:
uint8_t getIO ( uint16_t ioaddr ) {
    return Apple2_64K_RAM[ioaddr];
}

void setIO ( uint16_t ioaddr, uint8_t val ) {
    Apple2_64K_RAM[ioaddr] = val;
}

uint8_t getMEM ( uint16_t addr ) {
    return Apple2_64K_MEM[addr];
}

uint16_t getMEM16 ( uint16_t addr ) {
    return *(uint16_t*)(&Apple2_64K_MEM[addr]);
}

uint32_t getMEM32 ( uint16_t addr ) {
    return *(uint32_t*)(&Apple2_64K_MEM[addr]);
}

void setMEM ( uint16_t addr, uint8_t val ) {
    Apple2_64K_MEM[addr] = val;
}

void setMEM16 ( uint16_t addr, uint16_t val ) {
    *(uint16_t*)(&Apple2_64K_MEM[addr]) = val;
}

void setMEM32 ( uint16_t addr, uint32_t val ) {
    *(uint32_t*)(&Apple2_64K_MEM[addr]) = val;
}

void kbdInput ( uint8_t code ) {
    //    printf("kbdInput: %02X ('%c')\n", code, isprint(code) ? code : ' ');
    switch ( code ) {
        case '\n':
            code = 0x0D;
            break;
            
        case 0x7F: // BackSpace
            code = 0x08;
            break;
            
        default:
            break;
    }
    
    code |= 0x80;
    
    // timeout with linearly increasing sleep
    for( int i = 1; i < 100 && ( RAM[io_KBD] > 0x7F ); i++ ) {
        usleep( i * 2 );
    }
    
    RAM[io_KBD] = RAM[io_KBDSTRB] = code;
}


void kbdUp () {
    RAM[io_KBDSTRB] &= 0x7F;
}






