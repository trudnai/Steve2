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


uint8_t Apple2_Dummy_Page[ 1 * PG ];            // Dummy Page to discard data
uint8_t Apple2_Dummy_RAM[ 64 * KB ];            // Dummy RAM to discard data

uint8_t Apple2_64K_ROM[ 64 * KB ] = {0};        // ROM C0, C8, D0, D8, E0, E8, F0, F8

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
        memcpy(Apple2_64K_MEM + 0xC300, Apple2_64K_RAM + 0xC300, 0x100);
    }
    else {
        // load peripheral ROM to memory
        memcpy(Apple2_64K_MEM + 0xC300, Apple2_64K_ROM + 0xC300, 0x100);
    }
    
    MEMcfg = newMEMcfg;
}


void CxMemorySelect( MEMcfg_t newMEMcfg ) {
    
    if ( newMEMcfg.int_Cx_ROM ) {
        // load internal ROM to memory
        memcpy(Apple2_64K_MEM + 0xC100, Apple2_64K_ROM + 0xC100, 0xF00);
    }
    else {
        // load peripheral ROM to memory
        memcpy(Apple2_64K_MEM + 0xC100, Apple2_64K_RAM + 0xC100, 0xF00);
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
    memcpy(Apple2_64K_MEM + 0xC600, Apple2_64K_ROM + 0xC600, 0x100);
    
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
            
        case 0x7F: // BackSlash
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






