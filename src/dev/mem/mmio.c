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


#include <stdlib.h>

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

#define INIT_MEMCFG { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

const MEMcfg_t initMEMcfg = INIT_MEMCFG;

MEMcfg_t MEMcfg = INIT_MEMCFG;
MEMcfg_t newMEMcfg = INIT_MEMCFG;


const uint8_t * const shadowZPSTCKMEM = Apple2_64K_MEM;
const uint8_t * currentZPSTCKMEM = Apple2_64K_RAM;
const uint8_t * const shadowLowMEM = Apple2_64K_MEM + 0x200;
const uint8_t * currentLowRDMEM = Apple2_64K_RAM + 0x200;
uint8_t * currentLowWRMEM = Apple2_64K_RAM;


/// No writing (Readonly), and mark it as NO need to commit from Shadow RAM
INLINE void set_MEM_readonly(void) {
    dbgPrintf2("NOWR_AUX (pc:$%04X)\n", m6502.PC);
    
    MEMcfg.WR_RAM = 0;
    MEMcfg.WR_RAM_cntr = 0;
    WRD0MEM = Apple2_Dummy_RAM;   // for Discarding any writes to $D000 - $DFFF - BANK X
    WRHIMEM = Apple2_Dummy_RAM;   // for Discarding any writes to $E000 - $FFFF
}


/// Returns TRUE if already writeable or second of the "two consecutive" reads on appropriate soft switches
INLINE int is_wr_enabled(void) {
    uint64_t clk = m6502.clktime + m6502.clkfrm;
//    uint64_t elapsed = clk - m6502.clk_wrenable;
//    int is_enabled = ( elapsed < 16 ) || MEMcfg.WR_RAM;
    int is_enabled = ++MEMcfg.WR_RAM_cntr >= 1 || MEMcfg.WR_RAM;

//    printf("is_wr_enabled elapsed:%llu  was_enabled:%i  to_be_enabled:%i\n", elapsed, MEMcfg.WR_RAM, is_enabled);
    dbgPrintf2("is_wr_enabled WR_RAM_cntr:%u  was_enabled:%i  to_be_enabled:%i\n", MEMcfg.WR_RAM_cntr, MEMcfg.WR_RAM, is_enabled);

    m6502.clk_wrenable = clk;
    return is_enabled;
}


/// Make AUX RAM writeable -- This is when AUX is also readable, othwrwise use set_AUX_write...
/// Note: Need to save the content back from the shadow memory
INLINE void set_AUX_read_write(void) {
    // two consecutive read or write needs for write enable
    // Note: if it is already writeable and was previously a ROM read + RAM write, then we also need to bound AUX to MEM
    if ( is_wr_enabled() ) {
        dbgPrintf2("WR_MEM (pc:$%04X)\n", m6502.PC);
        
        // will write to Shadow RAM, and mark it as need to commit from Shadow RAM
        MEMcfg.WR_RAM = 1;
        WRD0MEM = Apple2_64K_MEM;   // for Write $D000 - $DFFF (shadow memory) - BANK X
        WRHIMEM = Apple2_64K_MEM;   // for Write $E000 - $FFFF (shadow memory)
    }
}


/// Make AUX RAM writeable -- This is when ROM is readable, othwrwise use set_MEM_write...
/// Note: NO NEED to write back the content since it writes everything directly to AUX memory
INLINE void set_AUX_write(void) {
    // will write directly to Auxiliary RAM, and mark it as NO need to commit from Shadow RAM
    // Note: if it is already writeable and was previously a RAM read + RAM write, then we also need to bound AUX to MEM
    if ( is_wr_enabled() ) {
        dbgPrintf2("WR_AUX (pc:$%04X)\n", m6502.PC);
        
        MEMcfg.WR_RAM = 1;
        if ( MEMcfg.RAM_BANK_2 ) {
            WRD0MEM = Apple2_64K_AUX;   // for Write $D000 - $DFFF (shadow memory) - BANK 2 at 0xD000
        }
        else {
            WRD0MEM = Apple2_64K_AUX - 0x1000;   // for Write $D000 - $DFFF (shadow memory) - BANK 1 at 0xC000
        }
        WRHIMEM = Apple2_64K_AUX;   // for Write $E000 - $FFFF (shadow memory)
    }
}


// save the content of Shadow Memory in needed
INLINE void save_AUX(void) {
    if ( MEMcfg.WR_RAM && MEMcfg.RD_INT_RAM ) {
        dbgPrintf2("Saving RAM Bank %d to %d (pc:$%04X)\n", MEMcfg.RAM_BANK_2 + 1, (current_RAM_bank == Apple2_64K_AUX + 0xD000) + 1, m6502.PC);
        // save LC Bank 1 or 2
        memcpy(current_RAM_bank, Apple2_64K_MEM + 0xD000, 0x1000);
        // save rest of LC RAM
        memcpy(Apple2_64K_AUX + 0xE000, Apple2_64K_MEM + 0xE000, 0x2000);
    }
}


/// Save entire
void save_RAM() {
    // save the content of Shadow ZP + Stack
    memcpy( (void*) currentZPSTCKMEM, shadowZPSTCKMEM, 0x200);
    // save LoMem
    memcpy( (void*) currentLowWRMEM + 0x200, WRLOMEM + 0x200, 0xBE00);

    save_AUX();

    if ( activeTextPage ) {
        // save the content of Shadow Memory
        memcpy(activeTextPage, shadowTextPage, 0x400);
    }

}





INLINE void select_RAM_BANK( uint16_t addr ) {
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
            
            dbgPrintf2("RAM_BANK_2 (pc:$%04X)\n", m6502.PC);
            
            MEMcfg.RAM_BANK_2 = 1;
            current_RAM_bank = Apple2_64K_AUX + 0xD000;
            break;
            
        default:
            dbgPrintf2("RAM_BANK_1 (pc:$%04X)\n", m6502.PC);
            
            MEMcfg.RAM_BANK_2 = 0;
            current_RAM_bank = Apple2_64K_AUX + 0xC000;
            break;
    }
}


INLINE void read_RAM_or_ROM( uint16_t addr ) {
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
            
            dbgPrintf2("RD_RAM (pc:$%04X)\n", m6502.PC);
            
            MEMcfg.RD_INT_RAM = 1;
            
            // load the content of Aux Memory
            memcpy(Apple2_64K_MEM + 0xD000, current_RAM_bank, 0x1000);
            memcpy(Apple2_64K_MEM + 0xE000, Apple2_64K_AUX + 0xE000, 0x2000);
            
            // set the RAM extension to read on the upper memory area
            break;
            
        default:
            dbgPrintf2("RD_ROM (pc:$%04X)\n", m6502.PC);
            
            MEMcfg.RD_INT_RAM = 0;
            
            // TODO: What about CX (Slot) ROM?
            // load the content of ROM Memory
            memcpy(Apple2_64K_MEM + 0xD000, INT_64K_ROM + 0xD000, 0x3000);
            
            // set the ROM to read on the upper memory area
            break;
    }
}


INLINE void write_RAM_or_NOT( uint16_t addr ) {
    // is RAM Writeable?
    switch ((uint8_t)addr) {
        case (uint8_t)io_MEM_RDROM_WRAM_2:
        case (uint8_t)io_MEM_RDROM_WRAM_1:
            
        case (uint8_t)io_MEM_RDROM_WRAM_2_:
        case (uint8_t)io_MEM_RDROM_WRAM_1_:
            
            dbgPrintf2("RD_ROM + WR_AUX (pc:$%04X)\n", m6502.PC);
            
            set_AUX_write();
            
            break;
            
        case (uint8_t)io_MEM_RDRAM_WRAM_2:
        case (uint8_t)io_MEM_RDRAM_WRAM_1:
            
        case (uint8_t)io_MEM_RDRAM_WRAM_2_:
        case (uint8_t)io_MEM_RDRAM_WRAM_1_:
            
            dbgPrintf2("RD_RAM + WR_RAM (pc:$%04X)\n", m6502.PC);
            
            set_AUX_read_write();
            
            break;
            
        default:
            dbgPrintf2("NO_WR (pc:$%04X)\n", m6502.PC);
            
            set_MEM_readonly();
            
            break;
    }
}


/// Switch between Memory Banks and ROM and Internal / Aux RAM
INLINE void io_RAM_EXP( uint16_t addr ) {
    
    if ( MEMcfg.RAM_16K || MEMcfg.RAM_128K ) {
        // TODO: store 0xD000 BANK 1 at 0xC000 -- might be a problem emulating 64k/128k Saturn cards
        
        save_AUX();
        select_RAM_BANK(addr);
        read_RAM_or_ROM(addr);
        write_RAM_or_NOT(addr);
        
    } // if there is RAM expansion card installed
}


INLINE int is_io_interesting( uint16_t addr ) {
    switch(addr) {
        case io_KBD:
        case io_KBDSTRB:
        case io_TAPEOUT:
        case io_SPKR:
        case io_VID_ALTCHAR:
        case io_VID_RD80VID:
        case io_RDCXROM:
            
        // Ignore Disk IO
        case 0xC0E0:
        case 0xC0E1:
        case 0xC0E2:
        case 0xC0E3:
        case 0xC0E4:
        case 0xC0E5:
        case 0xC0E6:
        case 0xC0E7:
        case 0xC0E8:
        case 0xC0E9:
        case 0xC0EA:
        case 0xC0EB:
        case 0xC0EC:
        case 0xC0ED:
        case 0xC0EE:
        case 0xC0EF:
            return 0;
            
        default:
            break;
    }

    return 1;
}


const int pasteBufferSize = 100;
int pasteBufferIdx = 0;
uint8_t pasteBuffer[pasteBufferSize];


uint8_t kbdCodeConvert( uint8_t code ) {
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

    // mark as valie keyboard input
    code |= 1<<7;

    return code;
}


void kbdClearPasteBuffer(void) {
    pasteBufferIdx = 0;
}


void kbdPaste ( uint8_t code ) {
    while (pasteBufferIdx >= pasteBufferSize) {
        usleep(100);
    }

    pasteBuffer[pasteBufferIdx++] = code;
}


uint8_t pasted = 0;
INLINE uint8_t kbdRead(void) {
//    if ( cpuMode == cpuMode_eco ) {
        // check if this is a busy keyboard poll (aka waiting for user input)
//        if ( IOframe < 16 ) {
//            clk_6502_per_frm_max = 6502; // Let it run for a bit to display character -- nerd number :-)
//            cpuState = cpuState_halting;
//        }
//    }

    // check pasted buffer
    if ( pasted ) {
        pasted--;
    }
    // check paste buffer
    else if (pasteBufferIdx) {
        if ( Apple2_64K_RAM[io_KBD] < 0x80 ) {
            kbdInput(pasteBuffer[0]);
//            memcpy(pasteBuffer, pasteBuffer + 1, pasteBufferSize -1);
            for (int i = 0; i < pasteBufferSize - 1; i++) {
                pasteBuffer[i] = pasteBuffer[i+1];
            }
            pasteBufferIdx--;
            // delay keyboard input to avoid weird character loss (KBDSTRB called many times for example when RETURN pressed)
            pasted = 10;
            // to make paste even faster
            disk_accelerator_speedup();
        }
    }

    // we have to return keybard not only for $C000 but for ports all the way till $C00F
    return Apple2_64K_RAM[io_KBD];
}


INLINE uint8_t kbdStrobe(void) {
    Apple2_64K_RAM[io_KBD] &= ~(1 << 7);

//    if ( cpuMode == cpuMode_eco ) {
//        // check if this is a busy keyboard poll (aka waiting for user input)
//        clk_6502_per_frm_max = clk_6502_per_frm; // Absolute low mode
//        cpuState = cpuState_halting; // cpuState_running;
//    }

    return Apple2_64K_RAM[io_KBDSTRB];
}


INLINE uint8_t ioRead( uint16_t addr ) {
    //    if (outdev) fprintf(outdev, "ioRead:%04X\n", addr);

//    if ( is_io_interesting(addr) ) {
//        printf("ioRead:%04X (PC:%04X)\n", addr, m6502.PC);
//    }
    
    m6502.lastIO = m6502.clkfrm;
    
//    // TODO: This is for speed demo only, should be either removed or the entire ioRead should based on binary search, whatever is faster
//    if ( addr == io_KBD ) {
//        //        clk_6502_per_frm_max = clk_6502_per_frm_max > 32768 ? clk_6502_per_frm_max - 32768 : 0; // ECO Mode!
//
//        if ( cpuMode == cpuMode_eco ) {
//            // check if this is a busy keyboard poll (aka waiting for user input)
//            if ( IOframe < 16 ) {
//                clk_6502_per_frm_max = 6502; // Let it run for a bit to display character -- nerd number :-)
//                cpuState = cpuState_halting;
//            }
//        }
//
//        return Apple2_64K_RAM[io_KBD];
//    }
    
    switch ( (uint8_t)addr ) {
        case (uint8_t)io_KBD:
        case (uint8_t)io_80STOREON:
        case (uint8_t)io_SETSLOTCXROM:
        case (uint8_t)io_SETINTCXROM:
        case (uint8_t)io_SETSTDZP:
        case (uint8_t)io_SETALTZP:
        case (uint8_t)io_SETINTC3ROM:
        case (uint8_t)io_SETSLOTC3ROM:
            return kbdRead();

        case (uint8_t)io_KBDSTRB:
            return kbdStrobe();

        case (uint8_t)io_TAPEOUT:
            // TODO: 1. Implement Tape
            return rand(); // Floating I/O -- used for random number generation in Games

        case (uint8_t)io_SPKR:
            spkr_toggle();
            return rand(); // Floating I/O -- used for random number generation in Games
            
        case (uint8_t)io_STROBE:
        case (uint8_t)io_CLRAN0:
        case (uint8_t)io_SETAN0:
        case (uint8_t)io_CLRAN1:
        case (uint8_t)io_SETAN1:
        case (uint8_t)io_CLRAN2:
        case (uint8_t)io_SETAN2:
        case (uint8_t)io_CLRAN3:
        case (uint8_t)io_SETAN3:
            // TODO: Simulate Attenuator
            return rand(); // Apple2_64K_RAM[io_SPKR];
            
        case (uint8_t)io_VID_RDVBL:
            return (m6502.clkfrm > 4550 ? 0x80 : 0) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_VID_RDTEXT:
            return (videoMode.text << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_VID_ALTCHAR:
            return (videoMode.altChr << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_VID_RD80VID:
            return (videoMode.col80 << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_TAPEIN:
            // TODO: this should be only on //c
            return MEMcfg.txt_page_2 << 7;
            
        case (uint8_t)io_RDCXROM:
            // TODO: Implement Reset Mouse X0 Interrupt (io_RSTXINT)
            return (MEMcfg.int_Cx_ROM << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_RDLCBNK2:
            return (MEMcfg.RAM_BANK_2 << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_RDLCRAM:
            return (MEMcfg.RD_INT_RAM << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_RDRAMRD:
            return (MEMcfg.RD_AUX_MEM << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_RDRAMWR:
            return (MEMcfg.WR_AUX_MEM << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_RDALTZP:
            // TODO: Implement Reset Mouse Y0 Interrupt (io_RSTYINT)
            return (MEMcfg.ALT_ZP << 7) | (kbdStrobe() & 0x7F);
                
        case (uint8_t)io_RDC3ROM:
            return MEMcfg.slot_C3_ROM << 7;
            
        case (uint8_t)io_RD80STORE:
            return (MEMcfg.is_80STORE << 7) | (kbdStrobe() & 0x7F);
            
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
            return (MEMcfg.txt_page_2 << 7) | (kbdStrobe() & 0x7F);
            
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
            return (videoMode.mixed << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_VID_Hires_OFF:
            videoMode.hires = 0;
            break;
            
        case (uint8_t)io_VID_Hires_ON:
            videoMode.hires = 1;
            break;
            
        case (uint8_t)io_VID_RDHIRES:
            return (videoMode.hires << 7) | (kbdStrobe() & 0x7F);
            
        case (uint8_t)io_PDL0:
        case (uint8_t)io_PDL1:
        case (uint8_t)io_PDL2:
        case (uint8_t)io_PDL3:
//            printf("PDL%d: %d\n", addr - io_PDL0, pdl_read( addr - io_PDL0 ));
            return pdl_read( addr - io_PDL0 );
            
        case (uint8_t)io_PDL_STROBE:
            return pdl_reset();
            
        case (uint8_t)io_RDMAINRAM:
            dbgPrintf2("R:io_RDMAINRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);

            // still need to return keyboard
            return kbdRead();

            break;
            
        case (uint8_t)io_RDCARDRAM:
            dbgPrintf2("R:io_RDCARDRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            
            // still need to return keyboard
            return kbdRead();

            break;
            
        case (uint8_t)io_WRMAINRAM:
            dbgPrintf2("R:io_WRMAINRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);

            // still need to return keyboard
            return kbdRead();

            break;
            
        case (uint8_t)io_WRCARDRAM:
            dbgPrintf2("R:io_WRCARDRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            
            // still need to return keyboard
            return kbdRead();

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
    
//    if ( is_io_interesting(addr) ) {
//        printf("ioWrite:%04X (PC:%04X, val:%02X)\n", addr, m6502.PC, val);
//    }
    

    switch ( (uint8_t)addr ) {
        case (uint8_t)io_KBDSTRB:
            kbdStrobe();
            break;
            
        case (uint8_t)io_TAPEOUT:
            // TODO: 1. Sound problem in Castle Wolfensein if we output this to speaker all the time
            //       2. Implement Tape
            break;
            
        case (uint8_t)io_SPKR:
            spkr_toggle();
            // TODO: Theoretically it toggles the speaker twice
//            m6502.clkfrm++; // to simulate RMW
//            spkr_toggle();
            break;
            
        case (uint8_t)io_RDMAINRAM:
            dbgPrintf2("W:io_RDMAINRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_RDCARDRAM:
            dbgPrintf2("W:io_RDCARDRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.RD_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_WRMAINRAM:
            dbgPrintf2("W:io_WRMAINRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_WRCARDRAM:
            dbgPrintf2("W:io_WRCARDRAM (pc:$%04X)\n", m6502.PC);
            newMEMcfg = MEMcfg;
            newMEMcfg.WR_AUX_MEM = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETSTDZP:
            dbgPrintf2("INT ZP (pc:$%04X)\n", m6502.PC);

            newMEMcfg = MEMcfg;
            newMEMcfg.ALT_ZP = 0;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETALTZP:
            dbgPrintf2("AUX ZP (pc:$%04X)\n", m6502.PC);

            newMEMcfg = MEMcfg;
            newMEMcfg.ALT_ZP = 1;
            auxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETSLOTCXROM:
            dbgPrintf2("io_SETSLOTCXROM\n");

            newMEMcfg = MEMcfg;
            newMEMcfg.int_Cx_ROM = 0;
            auxMemorySelect(newMEMcfg);
            CxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETINTCXROM:
            dbgPrintf2("io_SETINTCXROM\n");

            newMEMcfg = MEMcfg;
            newMEMcfg.int_Cx_ROM = 1;
            auxMemorySelect(newMEMcfg);
            CxMemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETSLOTC3ROM:
            dbgPrintf2("io_SETSLOTC3ROM\n");

            newMEMcfg = MEMcfg;
            newMEMcfg.slot_C3_ROM = 1;
            auxMemorySelect(newMEMcfg);
            C3MemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_SETINTC3ROM:
            dbgPrintf2("io_SETINTC3ROM\n");

            newMEMcfg = MEMcfg;
            newMEMcfg.slot_C3_ROM = 0;
            auxMemorySelect(newMEMcfg);
            C3MemorySelect(newMEMcfg);
            break;
            
        case (uint8_t)io_VID_CLR80VID:
            dbgPrintf2("io_VID_CLR80VID\n");
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
            dbgPrintf2("io_80STOREOFF (pc:$%04X)\n", m6502.PC);
            MEMcfg.is_80STORE = 0;
            textPageSelect();
            break;
            
        case (uint8_t)io_80STOREON:
            dbgPrintf2("io_80STOREON (pc:$%04X)\n", m6502.PC);
            MEMcfg.is_80STORE = 1;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_TXTPAGE1:
            dbgPrintf2("io_VID_TXTPAGE1 (pc:$%04X)\n", m6502.PC);
            MEMcfg.txt_page_2 = 0;
            textPageSelect();
            break;
            
        case (uint8_t)io_VID_TXTPAGE2:
            dbgPrintf2("io_VID_TXTPAGE2 (pc:$%04X)\n", m6502.PC);
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

INLINE uint8_t _memread( uint16_t addr ) {
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
INLINE uint8_t _memread_dbg( uint16_t addr ) {
    if (addr >= 0xC000) {
        //        return memread8_paged(addr);
        return memread8_high(addr);
    }
    //    return memread8_paged(addr);
    return memread8_low(addr);

    //    return memread8(addr);
}
INLINE uint8_t _memread_dis( uint16_t addr ) {
    if (addr >= 0xC000) {
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

INLINE void _memwrite8_low( uint16_t addr, uint8_t data ) {
    WRLOMEM[addr] = data;
}
INLINE void _memwrite8_bank( uint16_t addr, uint8_t data ) {
    WRD0MEM[addr] = data;
}
INLINE void _memwrite8_high( uint16_t addr, uint8_t data ) {
    WRHIMEM[addr] = data;
}
INLINE void _memwrite( uint16_t addr, uint8_t data ) {
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
INLINE uint8_t _fetch() {
    disHexB( disassembly.pOpcode, memread8_low(m6502.PC) );
#ifdef CLK_ABSOLUTE_PRECISE
    if ( (m6502.PC & 0xFF) >= 0xFF ) {
        m6502.clktime++;
    }
#endif
    return memread8_low( m6502.PC++ );
}

INLINE uint8_t _fetch_dis() {
    _disHexB( &disassembly.pOpcode, memread8_low(m6502.PC) );
    return memread8_low( m6502.PC++ );
}

/**
 Fetching 2 bytes as a 16 bit number from memory address pc (program counter)
 increase pc by one
 **/
INLINE uint16_t _fetch16() {
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

INLINE uint16_t _fetch16_dis() {
    uint16_t word = memread16( m6502.PC );
// disPrintf(disassembly.comment, "fetch16:%04X", word);
    m6502.PC += 2;

    _disHexB( &disassembly.pOpcode, (uint8_t)word );
    _disHexB( &disassembly.pOpcode, (uint8_t)(word >> 8));

    return word;
}




//INLINE uint8_t * dest( uint8_t * mem, uint16_t addr ) {
//    return mem + addr;
//}


/**
 abs        ....    absolute         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t _addr_abs() {
    return _fetch16();
}
INLINE uint16_t _addr_abs_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%04X", memread16(m6502.PC));
    return _fetch16_dis();
}
INLINE uint8_t _src_abs() {
    return _memread( _addr_abs() );
}
INLINE uint8_t _src_abs_dis() {
    return _memread_dis( _addr_abs_dis() );
}
//INLINE uint8_t * dest_abs() {
//    return WRLOMEM + addr_abs();
//}


INLINE int8_t _rel_addr() {
    return _fetch();
}
INLINE int8_t _rel_addr_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%04X", m6502.PC + 1 + (int8_t)memread8(m6502.PC));
    return _fetch_dis();
}
INLINE uint16_t _abs_addr() {
    return _fetch16();
}
INLINE uint16_t _abs_addr_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%04X", memread16(m6502.PC));
    return _fetch16_dis();
}
INLINE uint16_t _ind_addr() {
    return memread16( _fetch16() );
}
INLINE uint16_t _ind_addr_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "($%04X)", memread16(m6502.PC));
    _disPrintf(disassembly.comment, sizeof(disassembly.comment), "ind_addr:%04X", memread16(memread16(m6502.PC)));

    return memread16( _fetch16_dis() );
}

/**
 abs,X        ....    absolute, X-indexed         OPC $LLHH,X
 operand is address; effective address is address incremented by X with carry **
 **/
INLINE uint16_t _addr_abs_X() {
    return _fetch16() + m6502.X;
}
INLINE uint16_t _addr_abs_X_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%04X,X", memread16(m6502.PC));
    return _fetch16_dis() + m6502.X;
}
INLINE uint8_t _src_abs_X() {
    return _memread( _addr_abs_X() );
}
INLINE uint8_t _src_abs_X_dis() {
    return _memread_dis( _addr_abs_X_dis() );
}
//INLINE uint8_t * dest_abs_X() {
//    return WRLOMEM + addr_abs_X();
//}


/**
 abs,Y        ....    absolute, Y-indexed         OPC $LLHH,Y
 operand is address; effective address is address incremented by Y with carry **
 **/
INLINE uint16_t _addr_abs_Y() {
    return _fetch16() + m6502.Y;
}
INLINE uint16_t _addr_abs_Y_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%04X,Y", memread16(m6502.PC));
    return _fetch16_dis() + m6502.Y;
}
INLINE uint8_t _src_abs_Y() {
    return _memread(_addr_abs_Y());
}
INLINE uint8_t _src_abs_Y_dis() {
    return _memread_dis(_addr_abs_Y_dis());
}
//INLINE uint8_t * dest_abs_Y() {
//    return WRLOMEM + addr_abs_Y();
//}

INLINE uint8_t _imm() {
    return _fetch();
}
INLINE uint8_t _imm_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "#$%02X", memread8(m6502.PC));
    return _fetch_dis();
}


/**
 zpg        ....    zeropage         OPC $LL
 operand is zeropage address (hi-byte is zero, address = $00LL)
 **/
INLINE uint8_t _addr_zp() {
    return _fetch();
}
INLINE uint8_t _addr_zp_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%02X", memread8(m6502.PC));
    return _fetch_dis();
}
INLINE uint8_t _src_zp() {
    return memread8_low(_addr_zp());
}
INLINE uint8_t _src_zp_dis() {
    return memread8_low(_addr_zp_dis());
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
 ind        ....    indirect         OPC ($LL)
 operand is zeropage address;
 effective address is word in (LL, LL + 1), inc. without carry: C.w($00LL)
 **/
INLINE uint16_t _addr_ind() {
    return memread16( _fetch() );
}
INLINE uint16_t _addr_ind_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "($%02X,X)", memread8(m6502.PC) );
    _disPrintf(disassembly.comment, sizeof(disassembly.comment), "ind_addr:%04X", memread16( memread8(m6502.PC)) );

    return memread16( _fetch_dis() );
}
INLINE uint8_t _src_ind() {
    return _memread( _addr_ind() );
}
INLINE uint8_t _src_ind_dis() {
    return _memread_dis( _addr_ind_dis() );
}

/**
 X,ind        ....    X-indexed, indirect         OPC ($LL,X)
 operand is zeropage address;
 effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
 **/
INLINE uint16_t _addr_ind_X() {
    return memread16( _fetch() + m6502.X );
}
INLINE uint16_t _addr_ind_X_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "($%02X,X)", memread8(m6502.PC) );
    _disPrintf(disassembly.comment, sizeof(disassembly.comment), "ind_addr:%04X", memread16( memread8(m6502.PC) + m6502.X) );

    return memread16( _fetch_dis() + m6502.X );
}
INLINE uint8_t _src_X_ind() {
    return _memread( _addr_ind_X() );
}
INLINE uint8_t _src_X_ind_dis() {
    return _memread_dis( _addr_ind_X_dis() );
}
//INLINE uint8_t * dest_X_ind() {
//    return WRLOMEM + addr_ind_X();
//}

/**
 ind,Y        ....    indirect, Y-indexed         OPC ($LL),Y
 operand is zeropage address;
 effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
 **/
INLINE uint16_t _addr_ind_Y() {
    return memread16( _fetch() ) + m6502.Y;
}
INLINE uint16_t _addr_ind_Y_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "($%02X),Y", memread8(m6502.PC) );
    _disPrintf(disassembly.comment, sizeof(disassembly.comment), "ind_addr:%04X", memread16( memread8(m6502.PC) ) + m6502.Y );

    return memread16( _fetch_dis() ) + m6502.Y;
}
INLINE uint8_t _src_ind_Y() {
    return _memread( _addr_ind_Y() );
}
INLINE uint8_t _src_ind_Y_dis() {
    return _memread_dis( _addr_ind_Y_dis() );
}
//INLINE uint8_t * dest_ind_Y() {
//    return WRLOMEM + addr_ind_Y();
//}


/**
 zpg,X        ....    zeropage, X-indexed         OPC $LL,X
 operand is zeropage address;
 effective address is address incremented by X without carry **
 **/
INLINE uint8_t _addr_zp_X() {
    return _fetch() + m6502.X;
}
INLINE uint8_t _addr_zp_X_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%02X,X", memread8(m6502.PC));

    return _fetch_dis() + m6502.X;
}
INLINE uint8_t _src_zp_X() {
    return memread8_low(_addr_zp_X());
}
INLINE uint8_t _src_zp_X_dis() {
    return memread8_low(_addr_zp_X_dis());
}
//INLINE uint8_t * dest_zp_X() {
//    return WRLOMEM + addr_zp_X();
//}

/**
 zpg,Y        ....    zeropage, Y-indexed         OPC $LL,Y
 operand is zeropage address;
 effective address is address incremented by Y without carry **
 **/
INLINE uint8_t _addr_zp_Y() {
    return _fetch() + m6502.Y;
}
INLINE uint8_t _addr_zp_Y_dis() {
    _disPrintf(disassembly.oper, sizeof(disassembly.oper), "$%02X,Y", memread8(m6502.PC));

    return _fetch_dis() + m6502.Y;
}
INLINE uint8_t _src_zp_Y() {
    return memread8_low(_addr_zp_Y());
}
INLINE uint8_t _src_zp_Y_dis() {
    return memread8_low(_addr_zp_Y_dis());
}
//INLINE uint8_t * dest_zp_Y() {
//    return WRLOMEM + addr_zp_Y();
//}


void auxMemorySelect( MEMcfg_t newMEMcfg ) {
    const uint8_t * newReadMEM = currentLowRDMEM;
    uint8_t * newWriteMEM = currentLowWRMEM;

    // TODO: Check if this is supposed to be the opposite
    if ( newMEMcfg.is_80STORE ) {
        if ( newMEMcfg.RD_AUX_MEM ) {
            newReadMEM = Apple2_64K_AUX + 0x200;
        }
        else {
            newReadMEM = Apple2_64K_RAM + 0x200;
        }
        
        if ( newMEMcfg.WR_AUX_MEM ) {
            newWriteMEM = Apple2_64K_AUX;
        }
        else {
            newWriteMEM = Apple2_64K_RAM;
        }
    }
    else {
        newReadMEM = Apple2_64K_RAM + 0x200;
        newWriteMEM = Apple2_64K_RAM;
    }
    
    
    // save old content to shadow memory
    if ( ( newWriteMEM != currentLowWRMEM ) && (WRLOMEM == Apple2_64K_MEM) ) {
        // save the content of Shadow Memory
        memcpy( (void*) currentLowWRMEM + 0x200, WRLOMEM + 0x200, 0xBE00);
    }
    
    //    else {
    //        // page in the new memory area
    //        memcpy( (void*) shadowLowMEM + 0x200, newWriteMEM + 0x200, 0xBE00);
    //        // mark new as the current one
    //    }
    
    currentLowWRMEM = newWriteMEM;
    
//    printf("nrm:%p  nwm:%p\n", newReadMEM, newWriteMEM + 0x200);
    
    // we are reading and writing to the same memory (either Internal or Aux)
    if ( newReadMEM == newWriteMEM + 0x200 ) {
        WRLOMEM = Apple2_64K_MEM;
    }
    else {
        WRLOMEM = newWriteMEM;
    }
    
    // load new content to shadow memory
    if ( newReadMEM != currentLowRDMEM ) {
        // page in the new memory area
        memcpy( (void*) shadowLowMEM, newReadMEM, 0xBE00);
        // mark new as the current one
        currentLowRDMEM = newReadMEM;
    }

    
    // save old content to shadow memory
    if ( newMEMcfg.ALT_ZP != MEMcfg.ALT_ZP ) {
        // save the content of Shadow ZP + Stack
        memcpy( (void*) currentZPSTCKMEM, shadowZPSTCKMEM, 0x200);
        
        // which ZP & Stack shall we use now?
        if ( newMEMcfg.ALT_ZP ) {
            currentZPSTCKMEM = Apple2_64K_AUX;
        }
        else {
            currentZPSTCKMEM = Apple2_64K_RAM;
        }
        
        // load content of SP & Stack
        memcpy( (void*) shadowZPSTCKMEM, (void*) currentZPSTCKMEM, 0x200);
    }
    
    // finally we can mark change
    MEMcfg = newMEMcfg;
}




void auxMemorySelect_old( MEMcfg_t newMEMcfg ) {
    const uint8_t * newLowMEM = currentLowRDMEM;
    
    if ( newMEMcfg.is_80STORE ) {
        if ( newMEMcfg.RD_AUX_MEM ) {
            newLowMEM = Apple2_64K_AUX + 0x200;
        }
        else {
            newLowMEM = Apple2_64K_RAM + 0x200;
        }
        
        if ( newMEMcfg.WR_AUX_MEM ) {
            if ( newMEMcfg.RD_AUX_MEM ) {
                WRLOMEM = Apple2_64K_MEM;
            }
            else {
                WRLOMEM = Apple2_64K_AUX;
            }
        }
        else {
            if ( newMEMcfg.RD_AUX_MEM ) {
                WRLOMEM = Apple2_64K_AUX;
            }
            else {
                WRLOMEM = Apple2_64K_MEM;
            }
        }
    }
    else {
        newLowMEM = Apple2_64K_RAM + 0x200;
        WRLOMEM = Apple2_64K_MEM;
    }
    
    // load new content to shadow memory
    if ( newLowMEM != currentLowRDMEM ) {
        // save the content of Shadow Memory
        memcpy( (void*) currentLowRDMEM, shadowLowMEM, 0xBE00);
        // page in the new memory area
        memcpy( (void*) shadowLowMEM, newLowMEM, 0xBE00);
        // mark new as the current one
        currentLowRDMEM = newLowMEM;
    }
    
    MEMcfg = newMEMcfg;
}


//void (*auxMemorySelect)( MEMcfg_t newMEMcfg ) = & auxMemorySelect_new;


void C3MemorySelect( MEMcfg_t newMEMcfg ) {
    
    if ( newMEMcfg.slot_C3_ROM ) {
        // load peripheral ROM to memory
        memcpy(Apple2_64K_MEM + 0xC300, EXP_64K_ROM + 0xC300, 0x100);
    }
    else {
        // load internal ROM to memory
        memcpy(Apple2_64K_MEM + 0xC300, INT_64K_ROM + 0xC300, 0x100);
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

void setMEMarray ( uint16_t addr, uint8_t * arr, int len ) {
    while (len--) {
        Apple2_64K_MEM[addr++] = *arr++;
    }
}

void kbdInput ( uint8_t code ) {
    code = kbdCodeConvert(code);

    for (int i = 0; i <= 0xF; i++) {
        Apple2_64K_RAM[io_KBD + i] = code;
        // most significant bit is a status bit of other things
        Apple2_64K_RAM[io_KBDSTRB + i] = (Apple2_64K_RAM[io_KBDSTRB + i] & (1<<7)) | (code & ~(1<<7));
    }
    
//    // mark key pressed
//    Apple2_64K_RAM[io_KBDSTRB] |= 1<<7;
}


void kbdUp () {
    // mark key depressed
    Apple2_64K_RAM[io_KBDSTRB] &= 0x7F;
}





