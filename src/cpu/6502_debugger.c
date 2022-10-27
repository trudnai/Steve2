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
// Documentations:
//
// http://nesdev.com/6502_cpu.txt
// http://www.oxyron.de/html/opcodes02.html
// https://macgui.com/kb/article/46
// https://www.masswerk.at/6502/6502_instruction_set.html
//

#define CLK_WAIT

#define DEBUGGER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "6502.h"
#include "speaker.h"


void ViewController_spk_up_play(void);
void ViewController_spk_dn_play(void);


volatile cpuMode_s cpuMode = cpuMode_normal;
volatile cpuState_s cpuState = cpuState_unknown;


#include "../util/common.h"


#define SOFTRESET_VECTOR    0x3F2

#define NMI_VECTOR          0xFFFA
#define RESET_VECTOR        0xFFFC
#define IRQ_VECTOR          0xFFFE

const unsigned long long int iterations = G;
unsigned long long int inst_cnt = 0;

unsigned int video_fps_divider = DEF_VIDEO_DIV;
unsigned int fps = DEFAULT_FPS;

const double default_MHz_6502 = DEFAULT_MHZ_6502; // default_crystal_MHz / 14; // 1.023; // 2 * M; // 4 * M; // 8 * M; // 16 * M; // 128 * M; // 256 * M; // 512 * M;
const double iigs_MHz_6502 = 2.8;
const double iicplus_MHz_6502 = 4;
const double startup_MHz_6502 = 32;
double MHz_6502 = default_MHz_6502;
unsigned int clk_6502_per_frm =  FRAME_INIT( default_MHz_6502 );
unsigned int clk_6502_per_frm_set = FRAME_INIT( default_MHz_6502 );
unsigned int clk_6502_per_frm_max_sound = 4 * FRAME_INIT( default_MHz_6502 );
unsigned int clk_6502_per_frm_max = 0;


unsigned long long tick_per_sec = G;
unsigned long long tick_6502_per_sec = 0;


extern m6502_t m6502;

const int ecoSpindown = 25; // initial value of ECO Spingdown Counter


#include "../util/disassembler.h"
#include "../dev/mem/mmio.h"



typedef struct {
    uint8_t L;
    uint8_t H;
} bytes_t;


/**
 Instruction Implementations
 !!!! `his has to be here!!!
 This idea is that "INLINE" would work only if it is
 located in the same source file -- hence the include...
**/


#include "6502_instructions.h"

INLINE int m6502_Step(void) {

    
#ifdef DEBUG___
    switch ( m6502.PC ) {
        case 0x1E60:
            printf("Wavy Navy...\n");
            break;
            
        default:
            break;
    }
    
    
    switch ( m6502.PC ) {
        case 0xC600:
            printf("DISK...\n");
            break;
            
        case 0xC62F:
            printf("DISK IO...\n");
            break;
            
        default:
            break;
    }

    
    switch ( m6502.PC ) {
        case 0xE000:
            dbgPrintf("START...\n");
            break;
            
        case 0xF168:
            dbgPrintf("START...\n");
            break;
            
        case 0xF16B:
            dbgPrintf("START...\n");
            break;
            
        case 0xF195: // RAM size init
            dbgPrintf("START...\n");
            break;
            
        default:
            break;
    }
#endif
    
    disNewInstruction();
    
    switch ( fetch() ) {
#include "6502_std.h"       // Standard 6502 instructions
//#include "6502_und.h"       // Undocumented 6502 instructions
#include "6502_C.h"         // Extended 65C02 instructions

        default:
            dbgPrintf("%04X: Unimplemented Instruction 0x%02X\n", m6502.PC -1, memread( m6502.PC -1 ));
            return 2;
    } // switch fetch16
    
    return 2;
}

unsigned long long ee = 0;
unsigned long long dd = 0;

// nanosec does not work very well for some reason
struct timespec tim = { 0, 400L };

double mips = 0;
double mhz = 0;
unsigned long long epoch = 0;



void m6502_Run() {

    // init time
//#ifdef CLK_WAIT
//    unsigned long long elpased = (unsigned long long)-1LL;
//#endif

    m6502.clktime += m6502.clkfrm;
    m6502.clkfrm = 0;
    m6502.lastIO = 0;

    if( diskAccelerator_count ) {
        if( --diskAccelerator_count <= 0 ) {
            // make sure we only adjust clock once to get back to normal
            diskAccelerator_count = 0;
            clk_6502_per_frm = clk_6502_per_frm_set;
        }
    }

#ifdef SPEEDTEST
    for ( inst_cnt = 0; inst_cnt < iterations ; inst_cnt++ )
#elif defined( CLK_WAIT )
        // we clear the clkfrm from ViewController Update()
        // we will also use this to pause the simulation if not finished by the end of the frame
    for ( clk_6502_per_frm_max = clk_6502_per_frm; m6502.clkfrm < clk_6502_per_frm_max ; m6502.clkfrm += m6502_Step() )
#else
    // this is for max speed only -- WARNING! It works only if simulation runs in a completely different thread from the Update()
    for ( ; ; )
#endif
    {

    // TODO: clkfrm is already increamented!!!
    printDisassembly(outdev);
        
#ifdef INTERRUPT_CHECK_PER_STEP
        if ( m6502.IF ) {
            switch (m6502.interrupt) {
                case HALT:
                    // CPU is haletd, nothing to do here...
                    return;
                    
                case IRQ:
                    interrupt_IRQ();
                    break;
                    
                case NMI:
                    interrupt_NMI();
                    break;
                    
                case HARDRESET:
                    hardReset();
                    break;
                    
                case SOFTRESET:
                    softReset();
                    break;
                    
                default:
                    break;
            }
            
            m6502.IF = 0;
        }
#endif // INTERRUPT_CHECK_PER_STEP
        
    }
    
    if ( cpuMode == cpuMode_eco ) {
    // check if this is a busy keyboard poll (aka waiting for user input)
        if ( m6502.clkfrm - m6502.lastIO < 16 ) {
            if (m6502.ecoSpindown) {
                m6502.ecoSpindown--;
            }
            else {
                cpuState = cpuState_halting;
            }
        }
    }

    
    // play the entire sound buffer for this frame
    spkr_update();
    // this will take care of turning off disk motor sound when time is up
    spkr_update_disk_sfx();
}

void read_rom( const char * bundlePath, const char * filename, uint8_t * rom, const uint16_t addr, const uint16_t size ) {
    
    char fullPath[256];
    
    strcpy( fullPath, bundlePath );
    strcat( fullPath, "/");
    strcat( fullPath, filename );
    
    FILE * f = fopen(fullPath, "rb");
    if (f == NULL) {
        perror("Failed to read ROM image: ");
        return;
    }
    
    fseek(f, 0L, SEEK_END);
    uint16_t flen = ftell(f);
    fseek(f, 0L, SEEK_SET);

    if ( size && (size > flen) ) {
        printf("ROM image is too small (size:0x%04X  flen:0x%04X)\n", size, flen);
        return;
    }
    
    fread( rom + addr, 1, size, f);
    fclose(f);

}


size_t getFileSize ( const char * fullPath ) {
    FILE * f = fopen(fullPath, "rb");
    if (f == NULL) {
        perror("Failed to get filesize for ROM image: ");
        return 0;
    }
    
    fseek(f, 0L, SEEK_END);
    size_t flen = ftell(f);
    fseek(f, 0L, SEEK_SET);

    fclose(f);
    
    return flen;
}


void rom_loadFile( const char * bundlePath, const char * filename ) {
    char fullPath[256];
    
    strcpy( fullPath, bundlePath );
    strcat( fullPath, "/");
    strcat( fullPath, filename );

    printf("Loading ROM: %s\n", filename);
    
    size_t flen = getFileSize(fullPath);
    
    if ( flen == 0 ) {
        return; // there was an error
    }
    
    else if ( flen == 32 * KB ) {
        read_rom( bundlePath, filename, INT_64K_ROM + 0x8000, 0, 32 * KB);
        memcpy(Apple2_64K_MEM + 0xC000, INT_64K_ROM + 0xC000, 16 * KB); // activate the upper ROM
    }
    
    else if ( flen == 16 * KB ) {
        read_rom( bundlePath, filename, INT_64K_ROM + 0xC000, 0, 16 * KB);
        memcpy(Apple2_64K_MEM + 0xC000, INT_64K_ROM + 0xC000, 16 * KB);
    }
    
    else if ( flen == 12 * KB ) {
        read_rom( bundlePath, filename, INT_64K_ROM + 0xD000, 0x1000, 12 * KB);
        memcpy(Apple2_64K_MEM + 0xD000, INT_64K_ROM + 0xD000, 12 * KB);
    }
    

    // make a copy of the perfieral ROM area -- so it will fall back to this if no card inserted
    memcpy(EXP_64K_ROM + 0xC100, INT_64K_ROM + 0xC100, 0x0F00);
}


void openLog() {
#ifdef DISASSEMBLER
    outdev = fopen("/Users/trudnai/Library/Containers/com.trudnai.steveii/Data/disassembly_new.log", "w+");
#endif
    // for DEBUG ONLY!!! -- use stdout if could not create log file
//    if (outdev == NULL) {
//        outdev = stdout;
//    }
}


void closeLog() {
    if ( ( outdev ) && ( outdev != stdout ) && ( outdev != stderr ) ) {
        fclose(outdev);
    }
}


