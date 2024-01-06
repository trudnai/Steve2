//
//  jit_6502.c
//  Steve ][
//
//  Created by Tamas Rudnai on 4/14/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

#include "jit_6502.h"
#include "speaker.h"


#define CLK_WAIT


const size_t JIT_MEM_SIZE = 1024;
typedef __attribute((noinline, naked)) long (*jit_function)(uint8_t*, uint8_t*, m6502_t*);
uint8_t * jit_mem = NULL;

static const uint8_t jit_prologue [] = {
    0xC3
};

static const uint8_t jit_epilogue [] = {
    0xC3
};


void ViewController_spk_up_play(void);
void ViewController_spk_dn_play(void);


volatile cpuMode_s cpuMode = cpuMode_normal;
volatile cpuState_s cpuState = cpuState_unknown;


#include "../../util/common.h"


#define SOFTRESET_VECTOR    0x3F2

#define NMI_VECTOR          0xFFFA
#define RESET_VECTOR        0xFFFC
#define IRQ_VECTOR          0xFFFE

const unsigned long long int iterations = G;
unsigned long long int inst_cnt = 0;

unsigned int video_fps_divider = DEF_VIDEO_DIV;
unsigned int fps = DEFAULT_FPS;

const double default_crystal_MHz = 14.31818;
const double default_MHz_6502 = default_crystal_MHz / 14; // 1.023; // 2 * M; // 4 * M; // 8 * M; // 16 * M; // 128 * M; // 256 * M; // 512 * M;
const double iigs_MHz_6502 = 2.8;
const double iicplus_MHz_6502 = 4;
const double startup_MHz_6502 = 32;
double MHz_6502 = default_MHz_6502;
unsigned long long clk_6502_per_frm =  FRAME_INIT( default_MHz_6502 );
unsigned long long clk_6502_per_frm_set = FRAME_INIT( default_MHz_6502 );
unsigned long long clk_6502_per_frm_max_sound = 4 * FRAME_INIT( default_MHz_6502 );
unsigned long long clk_6502_per_frm_max = 0;


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

disassembly_t disassembly;

#include "../../util/disassembler.h"
#include "../../dev/mem/mmio.h"


INLINE void set_flags_N( const uint8_t test ) {
    m6502.N = BITTEST(test, 7);
}

INLINE void set_flags_V( const uint8_t test ) {
    m6502.V = BITTEST(test, 6);
}

INLINE void set_flags_Z( const uint8_t test ) {
    m6502.Z = test == 0;
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

INLINE flags_t getFlags() {
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


typedef enum {
    jit_state_init = 0,
    jit_state_prologue,
    jit_state_compiling,
    jit_state_epilogue,
    
    jit_state_max,
    jit_state_invalid = 999999
} jit_state_t;


static jit_state_t jit_state = jit_state_init;

#include "6502_instructions.h"

INLINE int m6502_Step() {
    
    
#ifdef DEBUG___
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
        case 0x00: BRK(); return 7;                                    // BRK
        case 0x01: ORA( src_X_ind() ); return 6;                       // ORA X,ind
        case 0x02: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x03: SLO( addr_zp_X() ); return 8;                       // SLO* zpg,X (undocumented)
        case 0x04: NOP(); src_zp(); return 3;                          // NOP* zpg (undocumented)
        case 0x05: ORA( src_zp() ); return 3;                          // ORA zpg
        case 0x06: ASL( addr_zp() ); return 5;                         // ASL zpg
        case 0x07: SLO( addr_zp() ); return 5;                         // SLO* zpg (undocumented)
        case 0x08: PHP(); return 3;                                    // PHP
        case 0x09: ORA( imm() ); return 2;                             // ORA imm
        case 0x0A: ASLA(); return 2;                                   // ASL A
        case 0x0B: ANC( imm() ); return 2;                             // ANC** imm (undocumented)
        case 0x0C: NOP(); src_abs(); return 4;                         // NOP* (undocumented)
        case 0x0D: ORA( src_abs() ); return 4;                         // ORA abs
        case 0x0E: ASL( addr_abs() ); return 6;                        // ASL abs
        case 0x0F: SLO( addr_abs() ); return 6;                        // SLO* (undocumented)
        case 0x10: BPL( rel_addr() ); return 2;                        // BPL rel
        case 0x11: ORA( src_ind_Y() ); return 5;                       // ORA ind,Y
        case 0x12: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x13: SLO( addr_zp_Y() ); return 8;                       // SLO* zpg,Y (undocumented)
        case 0x14: NOP(); addr_zp_X(); return 4;                       // NOP* zpg,X (undocumented)
        case 0x15: ORA( src_zp_X() ); return 4;                        // ORA zpg,X
        case 0x16: ASL( addr_zp_X() ); return 6;                       // ASL zpg,X
        case 0x17: SLO( addr_zp_X() ); return 6;                       // SLO* zpg,X (undocumented)
        case 0x18: CLC(); return 2;                                    // CLC
        case 0x19: ORA( src_abs_Y() ); return 4+1;                       // ORA abs,Y
        case 0x1A: NOP(); return 2;                                    // NOP* (undocumented)
        case 0x1B: SLO( addr_abs_Y() ); return 7;                      // SLO* abs,Y (undocumented)
        case 0x1C: NOP(); src_abs_X(); return 4;                       // NOP* (undocumented)
        case 0x1D: ORA( src_abs_X() ); return 4+1;                       // ORA abs,X
        case 0x1E: ASL( addr_abs_X() ); return 7;                      // ASL abs,X
        case 0x1F: SLO( addr_abs_X() ); return 7;                      // SLO* abs,X (undocumented)
        case 0x20: JSR( abs_addr() ); return 6;                        // JSR abs
        case 0x21: AND( src_X_ind() ); return 6;                       // AND X,ind
        case 0x22: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x23: RLA( addr_ind_X() ); return 8;                      // RLA* ind,X 8 (undocumented)
        case 0x24: BIT( src_zp() ); return 3;                          // BIT zpg
        case 0x25: AND( src_zp() ); return 3;                          // AND zpg
        case 0x26: ROL( addr_zp() ); return 5;                         // ROL zpg
        case 0x27: RLA( addr_zp() ); return 5;                         // RLA* zpg 5 (undocumented)
        case 0x28: PLP(); return 4;                                    // PLP
        case 0x29: AND( imm() ); return 2;                             // AND imm
        case 0x2A: ROLA(); return 2;                                   // ROL A
        case 0x2B: ANC( imm() ); return 2;                             // ANC* imm 2 (undocumented)
        case 0x2C: BIT( src_abs() ); return 4;                         // BIT abs
        case 0x2D: AND( src_abs() ); return 4;                         // AND abs
        case 0x2E: ROL( addr_abs() ); return 6;                        // ROL abs
        case 0x2F: RLA( addr_abs() ); return 6;                        // RLA* abs 6 (undocumented)
        case 0x30: BMI( rel_addr() ); return 2;                        // BMI rel
        case 0x31: AND( src_ind_Y() ); return 5;                       // AND ind,Y
        case 0x32: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x33: RLA( addr_ind_Y() ); return 8;                      // RLA* izy 8 (undocumented)
        case 0x34: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)
        case 0x35: AND( src_zp_X() ); return 4;                        // AND zpg,X
        case 0x36: ROL( addr_zp_X() ); return 6;                       // ROL zpg,X
        case 0x37: RLA( addr_zp_X() ); return 6;                       // RLA* zpx 6 (undocumented)
        case 0x38: SEC(); return 2;                                    // SEC
        case 0x39: AND( src_abs_Y() ); return 4+1;                     // AND abs,Y
        case 0x3A: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0x3B: RLA( addr_abs_Y() ); return 7;                      // RLA* aby 7 (undocumented)
        case 0x3C: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)
        case 0x3D: AND( src_abs_X() ); return 4+1;                     // AND abs,X
        case 0x3E: ROL( addr_abs_X() ); return 7;                      // ROL abs,X
        case 0x3F: RLA( addr_abs_X() ); return 7;                      // RLA* abx 7 (undocumented)
        case 0x40: RTI(); return 6;                                    // RTI
        case 0x41: EOR( src_X_ind() ); return 6;                       // EOR X,ind
        case 0x42: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x43: SRE( addr_ind_X() ); return 8;                      // SRE* izx 8 (undocumented)
        case 0x44: NOP(); src_zp(); return 3;                          // NOP* zp 3 (undocumented)
        case 0x45: EOR( src_zp() ); return 3;                          // EOR zpg
        case 0x46: LSR( addr_zp() ); return 5;                         // LSR zpg
        case 0x47: SRE( addr_zp() ); return 5;                         // SRE* zp 5 (undocumented)
        case 0x48: PHA(); return 3;                                    // PHA
        case 0x49: EOR( imm() ); return 2;                             // EOR imm
        case 0x4A: LSRA(); return 2;                                   // LSR A
        case 0x4B: ASR( imm() ); return 2;                             // TODO: ALR / ASR* imm 2 (undocumented)
        case 0x4C: JMP( abs_addr() ); return 3;                        // JMP abs
        case 0x4D: EOR( src_abs() ); return 4;                         // EOR abs
        case 0x4E: LSR( addr_abs() ); return 6;                        // LSR abs
        case 0x4F: SRE( abs_addr() ); return 6;                        // SRE* abs 6 (undocumented)
        case 0x50: BVC( rel_addr() ); return 2;                        // BVC rel
        case 0x51: EOR( src_ind_Y() ); return 5;                       // EOR ind,Y
        case 0x52: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x53: SRE( addr_ind_Y() ); return 8;                      // SRE* izy 8 (undocumented)
        case 0x54: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)
        case 0x55: EOR( src_zp_X() ); return 4;                        // AND zpg,X
        case 0x56: LSR( addr_zp_X() ); return 6;                       // LSR zpg,X
        case 0x57: SRE( addr_ind_X() ); return 6;                      // SRE* zpx 6 (undocumented)
        case 0x58: CLI(); return 2;                                    // CLI
        case 0x59: EOR( src_abs_Y() ); return 4+1;                     // EOR abs,Y
        case 0x5A: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0x5B: SRE( addr_abs_Y() ); return 7;                      // SRE* aby 7 (undocumented)
        case 0x5C: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)
        case 0x5D: EOR( src_abs_X() ); return 4+1;                     // EOR abs,X
        case 0x5E: LSR( addr_abs_X() ); return 7;                      // LSR abs,X
        case 0x5F: SRE( addr_abs_X() ); return 7;                      // SRE* abx 7 (undocumented)
        case 0x60: RTS(); return 6;                                    // RTS
        case 0x61: ADC( src_X_ind() ); return 6;                       // ADC X,ind
        case 0x62: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x63: RRA( addr_ind_X() ); return 8;                      // RRA* izx 8 (undocumented)
        case 0x64: NOP(); src_zp(); return 3;                          // NOP* zp 3 (undocumented)
        case 0x65: ADC( src_zp() ); return 3;                          // ADC zpg
        case 0x66: ROR( addr_zp() ); return 5;                         // ROR zpg
        case 0x67: RRA( addr_zp() ); return 5;                         // RRA* zp 5 (undocumented)
        case 0x68: PLA(); return 4;                                    // PLA
        case 0x69: ADC( imm() ); return 2;                             // ADC imm
        case 0x6A: RORA(); return 2;                                   // ROR A
        case 0x6B: ARC( imm() ); return 2;                             // ARR/ARC* imm 2 (undocumented)
        case 0x6C: JMP( ind_addr() ); return 5;                        // JMP ind
        case 0x6D: ADC( src_abs() ); return 4;                         // ADC abs
        case 0x6E: ROR( addr_abs() ); return 6;                        // ROR abs
        case 0x6F: RRA( abs_addr() ); return 6;                        // RRA* abs 6 (undocumented)
        case 0x70: BVS( rel_addr() ); return 2;                        // BVS rel
        case 0x71: ADC( src_ind_Y() ); return 5;                       // ADC ind,Y
        case 0x72: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x73: RRA( addr_ind_Y() ); return 8;                      // RRA* izy 8 (undocumented)
        case 0x74: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)
        case 0x75: ADC( src_zp_X() ); return 4;                        // ADC zpg,X
        case 0x76: ROR( addr_zp_X() ); return 6;                       // ROR zpg,X
        case 0x77: RRA( addr_zp_X() ); return 6;                       // RRA* zpx 6 (undocumented)
        case 0x78: SEI(); return 2;                                    // SEI
        case 0x79: ADC( src_abs_Y() ); return 4+1;                     // ADC abs,Y
        case 0x7A: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0x7B: RRA( addr_abs_Y() ); return 7;                      // RRA* aby 7 (undocumented)
        case 0x7C: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)
        case 0x7D: ADC( src_abs_X() ); return 4+1;                     // ADC abs,X
        case 0x7E: ROR( addr_abs_X() ); return 7;                      // ROR abs,X
        case 0x7F: RRA( addr_abs_X() ); return 7;                      // RRA* abx 7 (undocumented)
        case 0x80: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0x81: STA( addr_ind_X() ) ; return 6;                     // STA X,ind
        case 0x82: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0x83: SAX( addr_ind_X() ); return 6;                      // SAX* izx 6 (undocumented)
        case 0x84: STY( addr_zp() ); return 3;                         // STY zpg
        case 0x85: STA( addr_zp() ); return 3;                         // STA zpg
        case 0x86: STX( addr_zp() ); return 3;                         // STX zpg
        case 0x87: SAX( addr_zp() ); return 3;                         // SAX* izx 6 (undocumented)
        case 0x88: DEY(); return 2;                                    // DEY
        case 0x89: NOP(); imm(); return 2;                             // NOP* imm (undocumented)
        case 0x8A: TXA(); return 2;                                    // TXA
        case 0x8B: XAA( imm() ); return 2;                             // ANE / XAA* imm 2 (undocumented, highly unstable!)
        case 0x8C: STY( addr_abs() ); return 4;                        // STY abs
        case 0x8D: STA( addr_abs() ); return 4;                        // STA abs
        case 0x8E: STX( addr_abs() ); return 4;                            // STX abs
        case 0x8F: SAX( addr_abs() ); return 4;                        // SAX* abs 4 (undocumented)
        case 0x90: BCC( rel_addr() ); return 2;                        // BCC rel
        case 0x91: STA( addr_ind_Y() ); return 6;                      // STA ind,Y
        case 0x92: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x93: SHA( addr_ind_Y() ); return 6;                      // SHA* izy 6 (undocumented, unstable)
        case 0x94: STY( addr_zp_X() ); return 4;                       // STY zpg,X
        case 0x95: STA( addr_zp_X() ); return 4;                       // STA zpg,X
        case 0x96: STX( addr_zp_Y() ); return 4;                       // STX zpg,Y
        case 0x97: SAX( addr_zp_Y() ); return 4;                       // SAX* izy 4 (undocumented)
        case 0x98: TYA(); return 2;                                    // TYA
        case 0x99: STA( addr_abs_Y() ); return 5;                      // STA abs,Y
        case 0x9A: TXS(); return 2;                                    // TXS
        case 0x9B: SAS( addr_abs_Y() ); return 5;                      // TAS / XAS / SHS / SAS* aby 5 (undocumented, unstable)
        case 0x9C: SHY( addr_abs_X() ); return 5;                      // SHY* abx 5 (undocumented, unstable)
        case 0x9D: STA( addr_abs_X() ); return 5;                      // STA abs,X
        case 0x9E: SHX( addr_abs_Y() ); return 5;                      // SHX* aby 5 (undocumented, unstable)
        case 0x9F: SAX( addr_abs_Y() ); return 5;                      // TODO: SHA / SAX* aby 5 (undocumented, unstable)
        case 0xA0: LDY( imm() ); return 2;                             // LDY imm
        case 0xA1: LDA( src_X_ind() ) ; return 6;                      // LDA X,ind
        case 0xA2: LDX( imm() ); return 2;                             // LDX imm
        case 0xA3: LAX( src_X_ind() ); return 6;                       // LAX* izx 6 (undocumented)
        case 0xA4: LDY( src_zp() ); return 3;                          // LDY zpg
        case 0xA5: LDA( src_zp() ); return 3;                          // LDA zpg
        case 0xA6: LDX( src_zp() ); return 3;                          // LDX zpg
        case 0xA7: LAX( src_zp() ); return 3;                          // LAX* zpg 3 (undocumented)
        case 0xA8: TAY(); return 2;                                    // TAY
        case 0xA9: LDA( imm() ); return 2;                             // LDA imm
        case 0xAA: TAX(); return 2;                                    // TAX
        case 0xAB: LAX( imm() ); return 2;                             // LAX* imm 2 (undocumented, highly unstable)
        case 0xAC: LDY( src_abs() ); return 4;                         // LDY abs
        case 0xAD: LDA( src_abs() ); return 4;                         // LDA abs
        case 0xAE: LDX( src_abs() ); return 4;                         // LDX abs
        case 0xAF: LAX( src_abs() ); return 4;                         // LAX* abs 4 (undocumented)
        case 0xB0: BCS( rel_addr() ); return 2;                        // BCS rel
        case 0xB1: LDA( src_ind_Y() ); return 5;                       // LDA ind,Y
        case 0xB2: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0xB3: LAX( src_ind_Y() ); return 5;                       // LAX* izy 5 (undocumented)
        case 0xB4: LDY( src_zp_X() ); return 4+1;                        // LDY zpg,X
        case 0xB5: LDA( src_zp_X() ); return 4+1;                        // LDA zpg,X
        case 0xB6: LDX( src_zp_Y() ); return 4+1;                        // LDX zpg,Y
        case 0xB7: LAX( src_zp_Y() ); return 4+1;                        // LAX* zpy 4 (undocumented)
        case 0xB8: CLV(); return 2;                                    // CLV
        case 0xB9: LDA( src_abs_Y() ); return 4;                       // LDA abs,Y
        case 0xBA: TSX(); return 2;                                    // TSX
        case 0xBB: LAS( src_abs_Y() ); return 4;                       // TODO: LAS / LAR / LAX* aby 4 (undocumented)
        case 0xBC: LDY( src_abs_X() ); return 4;                       // LDY abs,X
        case 0xBD: LDA( src_abs_X() ); return 4;                       // LDA abs,X
        case 0xBE: LDX( src_abs_Y() ); return 4;                       // LDX abs,Y
        case 0xBF: LAX( src_abs_Y() ); return 4;                       // LAX* aby 4 (undocumented)
        case 0xC0: CPY( imm() ); return 2;                             // CPY imm
        case 0xC1: CMP( src_X_ind() ) ; return 6;                      // LDA X,ind
        case 0xC2: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0xC3: DCP( addr_ind_X() ); return 8;                      // DCP* izx 8 (undocumented)
        case 0xC4: CPY( src_zp() ); return 3;                          // CPY zpg
        case 0xC5: CMP( src_zp() ); return 3;                          // CMP zpg
        case 0xC6: DEC( addr_zp() ); return 5;                         // DEC zpg
        case 0xC7: DCP( addr_zp() ); return 5;                         // DCP* zpg 5 (undocumented)
        case 0xC8: INY(); return 2;                                    // INY
        case 0xC9: CMP( imm() ); return 2;                             // CMP imm
        case 0xCA: DEX(); return 2;                                    // DEX
        case 0xCB: SBX( imm() ); return 2;                             // SBX* imm 2 (undocumented)
        case 0xCC: CPY( src_abs() ); return 4;                         // CPY abs
        case 0xCD: CMP( src_abs() ); return 4;                         // CMP abs
        case 0xCE: DEC( addr_abs() ); return 6;                        // DEC abs
        case 0xCF: DCP( addr_abs() ); return 6;                        // DCP* abs 6 (undocumented)
        case 0xD0: BNE( rel_addr() ); return 2;                        // BNE rel
        case 0xD1: CMP( src_ind_Y() ); return 5;                       // CMP ind,Y
        case 0xD2: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0xD3: DCP( addr_ind_Y() ); return 8;                      // DCP* izy 8 (undocumented)
        case 0xD4: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)
        case 0xD5: CMP( src_zp_X() ); return 4;                        // CMP zpg,X
        case 0xD6: DEC( addr_zp_X() ); return 6;                       // DEC zpg,X
        case 0xD7: DCP( addr_zp_X() ); return 6;                       // DCP* zpx 6 (undocumented)
        case 0xD8: CLD(); return 2;                                    // CLD
        case 0xD9: CMP( src_abs_Y() ); return 4;                       // CMP abs,Y
        case 0xDA: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0xDB: DCP( addr_abs_Y() ); return 7;                      // DCP* aby 7 (undocumented)
        case 0xDC: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)
        case 0xDD: CMP( src_abs_X() ); return 4;                       // CMP abs,X
        case 0xDE: DEC( addr_abs_X() ); return 7;                      // DEC abs,X
        case 0xDF: DCP( addr_abs_X() ); return 7;                      // DCP* abx 7 (undocumented)
        case 0xE0: CPX( imm() ); return 2;                             // CPX imm
        case 0xE1: SBC( src_X_ind() ) ; return 6;                      // SBC (X,ind)
        case 0xE2: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0xE3: ISB( addr_ind_X() ); return 8;                      // ISC / INS / ISB* izx 8 (undocumented)
        case 0xE4: CPX( src_zp() ); return 3;                          // CPX zpg
        case 0xE5: SBC( src_zp() ); return 3;                          // SBC zpg
        case 0xE6: INC( addr_zp() ); return 5;                         // INC zpg
        case 0xE7: ISB( addr_zp() ); return 5;                         // ISC / INS / ISB* zpg 5 (undocumented)
        case 0xE8: INX(); return 2;                                    // INX
        case 0xE9: SBC( imm() ); return 2;                             // SBC imm
        case 0xEA: NOP(); return 2;                                    // NOP
        case 0xEB: SBC( imm() ); return 2;                             // USBC / SBC* imm 2 (undocumented)
        case 0xEC: CPX( src_abs() ); return 4;                         // CPX abs
        case 0xED: SBC( src_abs() ); return 4;                         // SBC abs
        case 0xEE: INC( addr_abs() ); return 6;                        // INC abs
        case 0xEF: ISB( addr_abs() ); return 6;                        // ISC / INS / ISB* abs 6 (undocumented)
        case 0xF0: BEQ( rel_addr() ); return 2;                        // BEQ rel
        case 0xF1: SBC( src_ind_Y() ); return 5;                       // SBC ind,Y
        case 0xF2: HLT(); return 0;                                     // JAM / KIL / HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0xF3: ISB( addr_ind_Y() ); return 8;                      // ISC / INS / ISB* izy 8 (undocumented)
        case 0xF4: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)
        case 0xF5: SBC( src_zp_X() ); return 4;                        // SBC zpg,X
        case 0xF6: INC( addr_zp_X() ); return 6;                       // INC zpg,X
        case 0xF7: ISB( addr_zp_X() ); return 6;                       // ISC / INS / ISB* zpx 6 (undocumented)
        case 0xF8: SED(); return 2;                                    // SED
        case 0xF9: SBC( src_abs_Y() ); return 4+1;                       // SBC abs,Y
        case 0xFA: NOP(); return 2;                                    // NOP (undocumented)
        case 0xFB: ISB( addr_abs_Y() ); return 7;                      // ISB* aby 7 (undocumented)
        case 0xFC: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)
        case 0xFD: SBC( src_abs_X() ); return 4+1;                       // SBC abs,X
        case 0xFE: INC( addr_abs_X() ); return 7;                      // INC abs,X
        case 0xFF: ISB( addr_abs_X() ); return 7;                      // ISC / INS / ISB* abx 7 (undocumented)
            
        default:
            dbgPrintf("%04X: Unimplemented Instruction 0x%02X\n", m6502.PC -1, memread( m6502.PC -1 ));
            return 2;
    }
    //    } // fetch16
    
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
    memwrite(0x3F4, 0);
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

#include "jit_6502.h"

void m6502_Run() {
    
    // Testin JIT
    //    run_from_rwx(5);
    //    run_from_rwx(15);
    //    test_asm();
    
    // init time
    //#ifdef CLK_WAIT
    //    unsigned long long elpased = (unsigned long long)-1LL;
    //#endif
    
#define run_1_times m6502_Step()
#define run_2_times run_1_times + run_1_times
#define run_4_times run_2_times + run_2_times
#define run_8_times run_4_times + run_4_times
    
    
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
    
    // TODO: What if we dynamically reduce or increace CPU speed?
    m6502.clktime += clk_6502_per_frm;
    
    if( diskAccelerator_count ) {
        if( --diskAccelerator_count <= 0 ) {
            // make sure we only adjust clock once to get back to normal
            diskAccelerator_count = 0;
            clk_6502_per_frm = clk_6502_per_frm_set;
        }
    }
    
    // play the entire sound buffer for this frame
    spkr_update();
    // this will take care of turning off disk motor sound when time is up
    spkr_update_disk_sfx();
}

void read_rom( const char * bundlePath, const char * filename, uint8_t * rom, const uint16_t addr ) {
    
    char fullPath[256];
    
    strcpy( fullPath, bundlePath );
    strcat( fullPath, "/");
    strcat( fullPath, filename );
    
    FILE * f = fopen(fullPath, "rb");
    if (f == NULL) {
        perror("Failed to read ROM: ");
        return;
    }
    
    fseek(f, 0L, SEEK_END);
    uint16_t flen = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    fread( rom + addr, 1, flen, f);
    fclose(f);
    
}


size_t getFileSize ( const char * fullPath ) {
    FILE * f = fopen(fullPath, "rb");
    if (f == NULL) {
        perror("Failed to read ROM: ");
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
    
    size_t flen = getFileSize(fullPath);
    
    if ( flen == 0 ) {
        return; // there was an error
    }
    
    else if ( flen == 16 * KB ) {
        read_rom( bundlePath, filename, Apple2_64K_ROM + 0xC000, 0);
        memcpy(Apple2_64K_MEM + 0xC000, Apple2_64K_ROM + 0xC000, 16 * KB);
    }
    
    else if ( flen == 12 * KB ) {
        read_rom( bundlePath, filename, Apple2_64K_ROM + 0xD000, 0x1000);
        memcpy(Apple2_64K_MEM + 0xD000, Apple2_64K_ROM + 0xD000, 12 * KB);
    }
    
}


void openLog() {
#ifdef DISASSEMBLER
    outdev = fopen("/Users/trudnai/Library/Containers/com.trudnai.steveii/Data/disassembly_woz.log", "w+");
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
    read_rom( bundlePath, "6502_functional_test.bin", Apple2_64K_RAM, 0);
    memcpy(Apple2_64K_MEM, Apple2_64K_RAM, 65536);
    
    m6502.PC = 0x400;
#else
    // Apple ][+ ROM
    
    rom_loadFile(bundlePath, romFileName);
    
    // Disk ][ ROM in Slot 6
    read_rom( bundlePath, "DISK_II_C600.ROM", Apple2_64K_ROM, 0xC600);
    memcpy(Apple2_64K_MEM + 0xC600, Apple2_64K_ROM + 0xC600, 0x100);
    
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




// Allocates RWX memory of given size and returns a pointer to it. On failure,
// prints out the error and returns NULL.
void* jit_malloc(size_t size) {
    void* ptr = mmap(0, size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (ptr == (void*)-1) {
        perror("mmap");
        return NULL;
    }
    
    return ptr;
}

// Allocates RWX memory of given size and returns a pointer to it. On failure,
// prints out the error and returns NULL.
void jit_free(void* ptr, size_t size) {
    // Release the mapped memory
    munmap(ptr, size);
}


// Sets a RX permission on the given memory, which must be page-aligned. Returns
// 0 on success. On failure, prints out the error and returns -1.
int jit_mprotect_exec(void* mem, size_t size) {
    if (mprotect(mem, size, PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect RX");
        return -1;
    }
    return 0;
}


// Sets a RW permission on the given memory, which must be page-aligned. Returns
// 0 on success. On failure, prints out the error and returns -1.
int jit_mprotect_write(void* mem, size_t size) {
    if (mprotect(mem, size, PROT_READ | PROT_WRITE) == -1) {
        perror("mprotect RW");
        return -1;
    }
    return 0;
}


#define JIT_LDA_MEM     0x48, 0x89, 0xf8                    // mov rax, rdi
#define JIT_ADC(imm)    0x48, 0x83, 0xc0, imm               // add rax, #imm
#define JIT_RTS         0xc3                                // ret

#define JIT_MOV_ECX_32099   0xB9, 0x63, 0x7D, 0x00, 0x00
#define JIT_MOV_RCX_32099   0x48, 0xC7, 0xC1, 0x63, 0x7D, 0x00, 0x00
#define JIT_DEC_ECX         0xFF, 0xC9
#define JIT_JNE_START       0x75, 0xF8


//unsigned char code[] = {
//    JIT_LDA_MEM,
//
//    JIT_MOV_ECX_32099,
//
//    JIT_ADC(4),
//    JIT_DEC_ECX,
//    JIT_JNE_START,
//
//    JIT_RTS
//};

//    int ret;
//    int x = 9;
//    int y = 8;
//    asm volatile (
//        "add %w[ret], %w[x], %w[y]"
//        // outputs
//        : [ret]"=r"(ret)
//        // inputs
//        : [x]"r"(x), [y]"r"(y)
//    );


// moves content of an 8 bit variable to memory e.g. an immediate operand
#define jit_var2mem8(val, code, ofs) *(uint8_t*)((code) + (ofs)) = (uint8_t)(val)
// moves content of a 16 bit variable to memory e.g. an immediate operand
#define jit_var2mem16(val, code, ofs) *(uint16_t*)((code) + (ofs)) = (uint16_t)(val)
// moves content of a 32 bit variable to memory e.g. an immediate operand
#define jit_var2mem32(val, code, ofs) *(uint32_t*)((code) + (ofs)) = (uint32_t)(val)
// moves content of a 64 bit variable to memory e.g. an immediate operand
#define jit_var2mem64(val, code, ofs) *(uint64_t*)((code) + (ofs)) = (uint64_t)(val)
// converts absolute mmeory address to relative RIP address
#define jit_abs2rip(var, code, ofs, rip) jit_var2mem32( (uint8_t*)&(var) - (code) - (rip), code, ofs )

// get offset of a struct member
#define jit_getOffsetOf(str, mem) ((uint64_t)((void*)&(mem) - (void*)&(str)))
// inserts an 8 bit offset to code
#define jit_putoffs8(str, mem, code, ofs, rip) jit_var2mem8( jit_getOffsetOf( str, mem ), code, ofs )
// inserts an 16 bit offset to code
#define jit_putoffs16(str, mem, code, ofs, rip) jit_var2mem16( jit_getOffsetOf( str, mem ), code, ofs )
// inserts an 32 bit offset to code
#define jit_putoffs32(str, mem, code, ofs, rip) jit_var2mem32( jit_getOffsetOf( str, mem ), code, ofs )
// inserts an 64 bit offset to code
#define jit_putoffs64(str, mem, code, ofs, rip) jit_var2mem64( jit_getOffsetOf( str, mem ), code, ofs )



void jit_emit_prologue() {
    //    static uint8_t code[] = { 0xC6, 0x05, 0, 0, 0, 0, 0xAB, 0xC3 };
    
    static const uint8_t code[] = {
        0x9c,                           // 00: pushf
        0x50,                           // 01: push   rax
        0x53,                           // 02: push   rbx
        0x51,                           // 03: push   rcx
        0x52,                           // 04: push   rdx
        0x57,                           // 05: push   rdi
        
        0x48, 0xbf, 0,0,0,0,0,0,0,0,    // 06: movabs rdi,0x1122334455667788
        
        0x8a, 0x47, 0x01,               // 10: mov    al,BYTE PTR [rdi+0x1]     ; A
        0x8a, 0x5f, 0x02,               // 13: mov    bl,BYTE PTR [rdi+0x2]     ; X
        0x8a, 0x57, 0x03,               // 16: mov    dl,BYTE PTR [rdi+0x3]     ; Y
        
        0x8a, 0x77, 0x04,               // 19: mov    dh,BYTE PTR [rdi+0x4]     ; D flag
        
        0x66, 0x31, 0xc9,               // 1C: xor    cx,cx                     ; prepare x86 flags
        
        0x80, 0x4f, 0x01, 0xff,         // 1F: or     BYTE PTR [rdi+0x1],0xff
        0x74, 0x04,                     // 23: je     28 <C0>
        0x66, 0x83, 0xc9, 0x01,         // 25: or     cx,0x1
                                        // 29: <C0>:
        
        0x80, 0x4f, 0x02, 0xff,         // 29: or     BYTE PTR [rdi+0x2],0xff
        0x74, 0x04,                     // 2D: je     32 <Z0>
        0x66, 0x83, 0xc9, 0x40,         // 2F: or     cx,0x40
                                        // 33: <Z0>:
        
        0x80, 0x4f, 0x03, 0xff,         // 33: or     BYTE PTR [rdi+0x3],0xff
        0x74, 0x05,                     // 37: je     3d <N0>
        0x66, 0x81, 0xc9, 0x80, 0x00,   // 39: or     cx,0x80
                                        // 3E: <N0>:
        
        0x80, 0x4f, 0x04, 0xff,         // 3E: or     BYTE PTR [rdi+0x4],0xff
        0x74, 0x05,                     // 42: je     48 <O0>
        0x66, 0x81, 0xc9, 0x00, 0x08,   // 44 or     cx,0x800
                                        // 49: <O0>:
        
        0x66, 0x51,                     // 49: push   cx
        0x66, 0x9d,                     // 4B: popfw
        0x8b, 0x0d, 0,0,0,0             // 4D: mov    ecx,DWORD PTR [rip+0x0]        # 52 <O0+0xa>    };
    };
    
    //    // address of m6502.D in relative RIP addressing mode
    //    jit_abs2rip(m6502.A, code, 0x02, 0x06);
    //    jit_abs2rip(m6502.X, code, 0x08, 0x0C);
    //    jit_abs2rip(m6502.Y, code, 0x0E, 0x12);
    //    jit_abs2rip(m6502.D, code, 0x1A, 0x1E); // Decimal Flag is simulated through DH register
    //    jit_abs2rip(m6502.clkfrm, code, 0x14, 0x18);
    //
    //    jit_mem = jit_malloc(JIT_MEM_SIZE);
    
    dbgPrintf("CLD ");
    disPrintf(disassembly.inst, "CLD");
    m6502.D = 0;
    
    //    jit_emit_code(jit_mem, code, sizeof(code));
}


// PROLOGUE
// - Copies over registers from the virtual CPU (6502) to host CPU (x64)
//      - A -> AL
//      - X -> BL
//      - Y -> BH
//      - Read MEM -> eSI
//      - Write MEM -> eDI
// - Restores flags from the Virtual 6502 to x64
// - Sets clkfrm -> eCX

// START
//      LDX #$20        MOV BL, [MEM + 20]
//      LDA $26         MOV AL, 26h
//      CLC             CLC
//      ADC #$5         ADC AL, 05h
//      DEX             DEC BL
//      BNE START       BEQ __CONT              ; compile
//                      SUB eCX, 4+2+2+3...     ; all cycles for the block...
//                      BNE START
// __END                CALL EQPILOGUE
// __CONT
//      ...

// EPILOGUE
// - Copies back registers from the host CPU (x64) to the virtual CPU (6502)
//      - AL -> A
//      - BL -> X
//      - BH -> Y
// - Save flags from the x64 to Virtual 6502
// - Copies eCX -> clkfrm
//
void jit_emit_code(void* mem, const void* code, const size_t size) {
    size_t offset = 0;
    
    switch (jit_state) {
        case jit_state_init:
            if ( jit_mem ) {
                jit_free(jit_mem, JIT_MEM_SIZE);
            }
            mem = jit_mem = jit_malloc(JIT_MEM_SIZE);
            
            jit_state = jit_state_prologue;
            // no break needed here, should flow to jit_state_prologue
            
        case jit_state_prologue:
            memcpy(mem, jit_prologue, sizeof(jit_prologue));
            //            offset += sizeof(jit_prologue);
            jit_state = jit_state_compiling;
            // no break needed here, should flow to jit_state_compiling
            
        case jit_state_compiling:
            memcpy(mem + offset, code, size);
            offset += size;
            break;
            
        case jit_state_epilogue:
            memcpy(mem, jit_epilogue, sizeof(jit_epilogue));
            offset += sizeof(jit_epilogue);
            jit_state = jit_state_prologue;
            break;
            
        default:
            // something is wrong!
            fprintf(stderr, "Error: Invalid jit_state (%i)\n", jit_state);
            break;
    }
    
}




__attribute((noinline, naked)) long jit_prologue2(uint8_t*, uint8_t*, m6502_t* m6502);


// Allocates RWX memory directly.
void run_from_rwx(long i) {
    jit_mem = jit_malloc(JIT_MEM_SIZE);
    //    jit_emit_code(jit_mem, );
    jit_mprotect_exec(jit_mem, JIT_MEM_SIZE);
    
    jit_function func = jit_prologue2; //jit_mem;
    long result = func( Apple2_64K_MEM, Apple2_64K_MEM, &m6502);
    printf("result = %ld\n", result);
    
    jit_free(jit_mem, JIT_MEM_SIZE);
}


unsigned long long test_asm(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ (
                          "ret \n\t"
                          "ret \n\t"
                          : "=r"(lo)
                          : "r"(lo)
                          );
    return ( (unsigned long long)lo) | ( ((unsigned long long)hi) << 32 );
}


