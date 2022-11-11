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

#undef DEBUGGER
#define FETCH_ADDR m6502.PC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "6502.h"
#include "6502_bp.h"
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

//INLINE unsigned long long rdtsc(void)
//{
//    unsigned hi, lo;
//    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi) );
//    return ( (unsigned long long)lo) | ( ((unsigned long long)hi) << 32 );
//}

m6502_t m6502 = {
    0,      // A
    0,      // X
    0,      // Y
    
    0,      // C
    0,      // Z
    0,      // I
    0,      // D
    0,      // B
    0,      // res
    0,      // V
    0,      // N

    0,      // PC
    0,      // SP
    
    0,      // clktime
    0,      // clklast
    0,      // clkfrm
    
    0,      // clk_wrenable
    
    0,      // lastIO
    0,      // ecoSpindown
    
    0,      // trace
    0,      // step
    0,      // brk
    0,      // rts
    0,      // bra
    0,      // bra_true
    0,      // bra_false
    0,      // compile
    
    HALT,   // IF
};

const int ecoSpindown = 25; // initial value of ECO Spingdown Counter


#include "../util/disassembler.h"
#include "../dev/mem/mmio.h"


INLINE void set_flags_N( const uint8_t test ) {
    m6502.N = BITTEST(test, 7);
}

INLINE void set_flags_V( const uint8_t test ) {
    m6502.V = BITTEST(test, 6);
}

INLINE void set_flags_Z( const uint8_t test ) {
    m6502.Z = !test;
}

INLINE void set_flags_C( const int16_t test ) {
    m6502.C = test >= 0;
}

INLINE void set_flags_NZ( const uint8_t test ) {
    set_flags_N(test);
    set_flags_Z(test);
}

INLINE void set_flags_NV( const uint8_t test ) {
    set_flags_N(test);
    set_flags_V(test);
}

INLINE void set_flags_NVZ( const uint8_t test ) {
    set_flags_NZ(test);
    set_flags_V(test);
}

INLINE void set_flags_NZC( const int16_t test ) {
    set_flags_NZ(test);
    set_flags_C(test);
}


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

INLINE flags_t getFlags(void) {
    flags_t f = {
        m6502.C != 0,    // Carry Flag
        m6502.Z != 0,    // Zero Flag
        m6502.I != 0,    // Interrupt Flag
        m6502.D != 0,    // Decimal Flag
        m6502.B != 0,    // B Flag
        m6502.res != 0,  // reserved -- should be always 1
        m6502.V != 0,    // Overflow Flag ???
        m6502.N != 0,    // Negative Flag
    };

    return f;
}


INLINE void setFlags( uint8_t byte ) {
    flags_t flags = { .SR =  byte };
    
    m6502.C = flags.C;      // Carry Flag
    m6502.Z = flags.Z;      // Zero Flag
    m6502.I = flags.I;      // Interrupt Flag
    m6502.D = flags.D;      // Decimal Flag
    m6502.B = flags.B;      // B Flag
    m6502.res = flags.res;  // reserved -- should be always 1
    m6502.V = flags.V;      // Overflow Flag ???
    m6502.N = flags.N;      // Negative Flag
}

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
            m6502.interrupt = INV;
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



void interrupt_IRQ() {
    m6502.PC = memread16(IRQ_VECTOR);
    // TODO: PUSH things onto stack?
}

void interrupt_NMI() {
    m6502.PC = memread16(NMI_VECTOR);
    // TODO: PUSH things onto stack?
}

void hardReset() {
    m6502.PC = memread16(RESET_VECTOR);
    // make sure it will be a cold reset...
    _memwrite(0x3F4, 0);
    m6502.SP = 0xFF;
    // N V - B D I Z C
    // 0 0 1 0 0 1 0 1
    setFlags(0x25);
}

void softReset() {
//    m6502.PC = memread16(SOFTRESET_VECTOR);
    m6502.PC = memread16( RESET_VECTOR );
    
    m6502.SP = 0xFF;
    // N V - B D I Z C
    // 0 0 1 0 0 1 0 1
    setFlags(0x25);
    
    spkr_stopAll();
    
    resetMemory();
}


void m6502_Run() {

    // init time
//#ifdef CLK_WAIT
//    unsigned long long elpased = (unsigned long long)-1LL;
//#endif

    m6502.clktime += m6502.clkfrm;
    m6502.clkfrm = 0;
    m6502.lastIO = 0;
    // make sure we aare not debugging
    m6502.debugger.on = 0;
    m6502.debugger.wMask = 0;


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


void m6502_Debug(void) {
    m6502.clktime += m6502.clkfrm;
//    m6502.clkfrm = 0;
    m6502.lastIO = 0;
    m6502.interrupt = NO_INT; // TODO: This should be taken care by the interrupt handler

    m6502.debugger.on = 1;
    m6502.debugger.mask.hlt = 1;
    m6502.debugger.mask.brk = 1;
    m6502.debugger.mask.inv = 1;

    if( diskAccelerator_count ) {
        if( --diskAccelerator_count <= 0 ) {
            // make sure we only adjust clock once to get back to normal
            diskAccelerator_count = 0;
            clk_6502_per_frm = clk_6502_per_frm_set;
        }
    }

    clk_6502_per_frm_max = clk_6502_per_frm;

    for ( m6502.clkfrm = m6502_Step(); m6502.clkfrm < clk_6502_per_frm_max ; m6502.clkfrm += m6502_Step() ) {
        switch (m6502.interrupt) {
            case HALT:
                if (m6502.debugger.mask.hlt) {
                    cpuState = cpuState_halted;
                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case BREAK:
                if (m6502.debugger.mask.brk) {
                    cpuState = cpuState_halted;
                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case IRQ:
                if (m6502.debugger.mask.irq) {
                    cpuState = cpuState_halted;
                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case NMI:
                if (m6502.debugger.mask.nmi) {
                    cpuState = cpuState_halted;
                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case INV:
                if (m6502.debugger.mask.inv) {
                    cpuState = cpuState_halted;
                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case RET:
                // Step_Out & Step_Over: Return to caller
                if (m6502.debugger.mask.out) {
                    if ( m6502.SP >= m6502.debugger.SP ) {
                        cpuState = cpuState_halted;
                        m6502.debugger.wMask = 0;
                        return;
                    }
                }
                break;

            case HARDRESET:
                hardReset();
                break;

            case SOFTRESET:
                softReset();
                break;

            default:
                // Step_Out: If there was a POP (PLA, PLX, PLY, PLP), then we should update the monitoring stack pointer
                // (so we can return to the caller, not stopping at the POP)
                if ( m6502.SP > m6502.debugger.SP ) {
                    m6502.debugger.SP = m6502.SP;
                }
                break;
        }

        m6502.interrupt = NO_INT;

        if ( m6502_dbg_bp_is_exists(m6502.PC) ) {
            cpuState = cpuState_halted;
            m6502.debugger.wMask = 0;
            m6502.debugger.on = 0;
            return;
        }

    }

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
#ifdef DISASSEMBLER_LOG
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


void m6502_ColdReset( const char * bundlePath, const char * romFileName ) {
    inst_cnt = 0;
    
    spkr_init();
    
    clk_6502_per_frm     =
    clk_6502_per_frm_max =
    clk_6502_per_frm_set = 0;
    
    // wait 100ms to be sure simulation has been halted
    usleep(100000);
    
//    printf("Bundlepath: %s\n", bundlePath);

//    epoch = rdtsc();
//    sleep(1);
//    unsigned long long e = rdtsc();
//    tick_per_sec = e - epoch;
//    tick_6502_per_sec = tick_per_sec / MHz_6502;

    initMemory();

    
#ifdef FUNCTIONTEST
    read_rom( bundlePath, "6502_functional_test.bin", Apple2_64K_RAM, 0, 0);
    memcpy(Apple2_64K_MEM, Apple2_64K_RAM, 65536);
    
    m6502.PC = 0x400;
#else
    // Apple ][+ ROM
    
    rom_loadFile(bundlePath, romFileName);
    
    // Disk ][ ROM in Slot 6
    read_rom( bundlePath, "DISK_II_C600.ROM", EXP_64K_ROM, 0xC600, 0x100);
    memcpy(Apple2_64K_MEM + 0xC600, EXP_64K_ROM + 0xC600, 0x100);

    m6502.A = m6502.X = m6502.Y = 0xFF;
    // reset vector
    m6502.SP = 0xFF; //-3;
    
    // N V - B D I Z C
    // 0 0 1 0 0 1 0 0
    setFlags(0x24);
    
    m6502.IF = 0;
    
    // memory size
    //*((uint16_t*)(&RAM[0x73])) = 0xC000;

    m6502.PC = memread16( RESET_VECTOR );
#endif
    
    
#ifdef DEBUG_COUNTER_TEST
    uint8_t counter[] = {
                           // 1    * COUNTER2
                           // 2
                           // 3             ORG   $1000
                           // 4    SCREEN   EQU   $400
                           // 5    HOME     EQU   $FC58
                           // 6    DIGITS   EQU   $06
                           // 7    ZERO     EQU   $B0
                           // 8    CARRY    EQU   $BA
                           // 9    RDKEY    EQU   $FD0C
                           //10

        // I have placed NOP to keep addresses
        
        0xA0, 0x09, 0xEA,  //11            LDY   #$09 ; NOP
        0x84, 0x06,        //12            STY   #DIGITS
        0xEA, 0xEA,        //13            NOP NOP
        0xEA, 0xEA, 0xEA,  //14            NOP NOP NOP

        0xA6, 0x06,        //15            LDY   DIGITS
        0xA9, 0xB0,        //16   CLEAR    LDA   #ZERO
        0x99, 0x00, 0x04,  //17            STA   SCREEN,Y
        0x88,              //18            DEY
        0x10, 0xF8,        //19            BPL   CLEAR
        
        0xA4, 0x06,        //20   START    LDY   DIGITS
        0x20, 0x36, 0x10,  //21   ONES     JSR   INC
        0xB9, 0x00, 0x04,  //22            LDA   SCREEN,Y
        0xC9, 0xBA,        //23            CMP   #CARRY
        0xD0, 0xF6,        //24            BNE   ONES
        
        0xA9, 0xB0,        //25   NEXT     LDA   #ZERO
        0x99, 0x00, 0x04,  //26            STA   SCREEN,Y
        0x88,              //27            DEY
        0x30, 0x0D,        //28            BMI   END
        
        0x20, 0x36, 0x10,  //29            JSR   INC
        0xB9, 0x00, 0x04,  //30            LDA   SCREEN,Y
        0xC9, 0xBA,        //31            CMP   #CARRY
        0xD0, 0xE2,        //32            BNE   START
        
        0x4C, 0x20, 0x10,  //33            JMP   NEXT
        0x60,              //34   END      RTS

        0xB9, 0x00, 0x04,  //36   INC      LDA   SCREEN,Y
        0xAA,              //37            TAX
        0xE8,              //38            INX
        0x8A,              //39            TXA
        0x99, 0x00, 0x04,  //40            STA   SCREEN,Y
        0x60,              //41            RTS
    };

    
    uint8_t counter_fast[] = {
        // 1    * COUNTER2
        // 2
        // 3             ORG   $1000
        // 4    SCREEN   EQU   $400
        // 5    HOME     EQU   $FC58
        // 6    DIGITS   EQU   $06
        // 7    ZERO     EQU   $B0
        // 8    CARRY    EQU   $BA
        // 9    RDKEY    EQU   $FD0C
        //10

        // I have placed NOP to keep addresses
        
        0xA0, 0x06,        // 00            LDY   #$09
        0x84, 0x06,        // 02            STY   #DIGITS
        
        0xA6, 0x06,        // 04            LDY   DIGITS
        0xA9, 0xB0,        // 06   CLEAR    LDA   #ZERO
        0x99, 0x00, 0x04,  // 08            STA   SCREEN,Y
        0x88,              // 0B            DEY
        0x10, 0xF8,        // 0C            BPL   CLEAR
        
        0xA6, 0x06,        // 0E   START    LDX   DIGITS
        0xA9, 0xBA,        // 10            LDA   #CARRY
        0xFE, 0x00, 0x04,  // 12   ONES     INC   SCREEN,X
        0xDD, 0x00, 0x04,  // 15            CMP   SCREEN,X
        0xD0, 0xF8,        // 18            BNE   ONES
        
        0xA9, 0xB0,        // 1A   NEXT     LDA   #ZERO
        0x9D, 0x00, 0x04,  // 1C            STA   SCREEN,X
        0xCA,              // 1F            DEX
        0x30, 0x0C,        // 20            BMI   END
        
        0xFE, 0x00, 0x04,  // 22            INC   SCREEN,X
        0xBD, 0x00, 0x04,  // 25            LDA   SCREEN,X
        0xC9, 0xBA,        // 28            CMP   #CARRY
        0xD0, 0xE2,        // 2A            BNE   START
        
        0xF0, 0xEC,        // 2C            BEQ   NEXT
        
        0x60,              // 2E   END      RTS
        
    };
#endif

    // set the default speed
    clk_6502_per_frm_set = clk_6502_per_frm = FRAME(default_MHz_6502);
    
    
    // Initialize Paddle / Game Controller / Joystick / Mouse Buttons
    setIO(0xC061, 0);
    setIO(0xC062, 0);
    setIO(0xC063, 1 << 7); // inverted (bit 7: not pressed)

}


void tst6502() {
    // insert code here...
    printf("6502\n");
    
    m6502_ColdReset( "", "" );
    
    //    clock_t start = clock();
//    epoch = rdtsc();
    m6502_Run();
    //    clock_t end = clock();
    //    double execution_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    
#ifdef SPEEDTEST
    unsigned long long end = rdtsc();
    unsigned long long elapsed = end - epoch;
    double execution_time = (double)elapsed / tick_per_sec;
    
    double mips = inst_cnt / (execution_time * M);
    double mhz = m6502.clktime / (execution_time * M);
    printf("clk:%llu Elpased time: (%llu / %u / %llu), %.3lfs (%.3lf MIPS, %.3lf MHz)\n", iterations *3, tick_per_sec, MHz_6502, tick_6502_per_sec, execution_time, mips, mhz);
//    printf("  dd:%llu  ee:%llu  nn:%llu\n", dd, ee, ee - dd);
#endif
}

int ___main(int argc, const char * argv[]) {
    tst6502();
    return 0;
}

