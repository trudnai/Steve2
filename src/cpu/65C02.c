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

#define CLK_WAIT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "6502.h"
#include "woz.h"
#include "speaker.h"


void ViewController_spk_up_play(void);
void ViewController_spk_dn_play(void);


#include "../util/common.h"


#define SOFTRESET_VECTOR    0x3F2

#define NMI_VECTOR          0xFFFA
#define RESET_VECTOR        0xFFFC
#define IRQ_VECTOR          0xFFFE

const unsigned long long int iterations = G;
unsigned long long int inst_cnt = 0;

//const unsigned int fps = 30;
const unsigned long long default_MHz_6502 = 1.023 * M; // 2 * M; // 4 * M; // 8 * M; // 16 * M; // 128 * M; // 256 * M; // 512 * M;
const unsigned long long startup_MHz_6502 = 32 * M;
unsigned long long MHz_6502 = default_MHz_6502;
unsigned long long clk_6502_per_frm =  default_MHz_6502 / fps;
unsigned long long clk_6502_per_frm_set = default_MHz_6502 / fps;
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

    // flags_t
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

    // debugger_t
    0,      // on
    0,      // SP
    0,      // wMask

    // debugMask_t
    0,      // trace
    0,      // step
    0,      // hlt
    0,      // brk
    0,      // irq
    0,      // nmi
    0,      // inv
    0,      // out
    0,      // ret
    0,      // bra
    0,      // bra_true
    0,      // bra_false
    0,      // compile
    
    HALT,   // IF
    
};

disassembly_t disassembly;

#include "6502_dis_utils.h"
#include "../dev/mem/mmio.h"


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
    
#ifdef FUNCTIONTEST
    switch ( m6502.PC ) {
        case 0x400:
            dbgPrintf("START...\n");
            break;
        
        case 0x0438:
            dbgPrintf2("*** TEST 1 (%04X)\n", m6502.PC);
            break;
            
        case 0x0581:
            dbgPrintf2("*** TEST 2 (%04X)\n", m6502.PC);
            break;
            
        case 0x05C8:
            dbgPrintf2("*** TEST 3 (%04X)\n", m6502.PC);
            break;
            
        case 0x05FC:
            dbgPrintf2("*** TEST 4 (%04X)\n", m6502.PC);
            break;
            
        case 0x0776:
            dbgPrintf2("*** TEST 5 (%04X)\n", m6502.PC);
            break;
            
        case 0x0872:
            dbgPrintf2("*** TEST 6 (%04X)\n", m6502.PC);
            break;
            
        case 0x08A6:
            dbgPrintf2("*** TEST 7 (%04X)\n", m6502.PC);
            break;
            
        case 0x08F0:
            dbgPrintf2("*** TEST 8 (%04X)\n", m6502.PC);
            break;
            
        case 0x0946:
            dbgPrintf2("*** TEST 9 (%04X)\n", m6502.PC);
            break;
            
        case 0x0982:
            dbgPrintf2("*** TEST 10 (%04X)\n", m6502.PC);
            break;
            
        case 0x09B9:
            dbgPrintf2("*** TEST 11 (%04X)\n", m6502.PC);
            break;
            
        case 0x0A11:
            dbgPrintf2("*** TEST 12 (%04X)\n", m6502.PC);
            break;
            
        case 0x0AB7:
            dbgPrintf2("*** TEST 13 (%04X)\n", m6502.PC);
            break;
            
        case 0x0D7D:
            dbgPrintf2("*** TEST 14 (%04X)\n", m6502.PC);
            break;
            
        case 0x0E46:
            dbgPrintf2("*** TEST 15 (%04X)\n", m6502.PC);
            break;
            
        case 0x0F01:
            dbgPrintf2("*** TEST 16 (%04X)\n", m6502.PC);
            break;
            
        case 0x0F43:
            dbgPrintf2("*** TEST 17 (%04X)\n", m6502.PC);
            break;
            
        case 0x0FFA:
            dbgPrintf2("*** TEST 18 (%04X)\n", m6502.PC);
            break;
            
        case 0x103A:
            dbgPrintf2("*** TEST 19 (%04X)\n", m6502.PC);
            break;
            
        case 0x1330:
            dbgPrintf2("*** TEST 20 (%04X)\n", m6502.PC);
            break;
            
        case 0x162A:
            dbgPrintf2("*** TEST 21 (%04X)\n", m6502.PC);
            break;
            
        case 0x16DB:
            dbgPrintf2("*** TEST 22 (%04X)\n", m6502.PC);
            break;
            
        case 0x17FA:
            dbgPrintf2("*** TEST 23 (%04X)\n", m6502.PC);
            break;
            
        case 0x1899:
            dbgPrintf2("*** TEST 24 (%04X)\n", m6502.PC);
            break;
            
        case 0x1B63:
            dbgPrintf2("*** TEST 25 (%04X)\n", m6502.PC);
            break;
            
        case 0x1CB7:
            dbgPrintf2("*** TEST 26 (%04X)\n", m6502.PC);
            break;
            
        case 0x1DC5:
            dbgPrintf("*** TEST 27 (%04X)\n", m6502.PC);
            break;
            
        case 0x1ED3:
            dbgPrintf2("*** TEST 28 (%04X)\n", m6502.PC);
            break;
            
        case 0x22B7:
            dbgPrintf2("*** TEST 29 (%04X)\n", m6502.PC);
            break;
            
        case 0x23FB:
            dbgPrintf2("*** TEST 30 (%04X)\n", m6502.PC);
            break;
            
        case 0x257B:
            dbgPrintf2("*** TEST 31 (%04X)\n", m6502.PC);
            break;
            
        case 0x271F:
            dbgPrintf2("*** TEST 32 (%04X)\n", m6502.PC);
            break;
            
        case 0x289F:
            dbgPrintf2("*** TEST 33 (%04X)\n", m6502.PC);
            break;
            
        case 0x2A43:
            dbgPrintf2("*** TEST 34 (%04X)\n", m6502.PC);
            break;
            
        case 0x2AED:
            dbgPrintf2("*** TEST 35 (%04X)\n", m6502.PC);
            break;
            
        case 0x2BA7:
            dbgPrintf2("*** TEST 36 (%04X)\n", m6502.PC);
            break;
            
        case 0x2C55:
            dbgPrintf2("*** TEST 37 (%04X)\n", m6502.PC);
            break;
            
        case 0x2D13:
            dbgPrintf2("*** TEST 38 (%04X)\n", m6502.PC);
            break;
            
        case 0x3103:
            dbgPrintf2("*** TEST 40 (%04X)\n", m6502.PC);
            break;
            
        case 0x32FC:
            dbgPrintf2("*** TEST 41 (%04X)\n", m6502.PC);
            break;
            
        case 0x3361:
            dbgPrintf2("*** TEST 42 (%04X)\n", m6502.PC);
            break;
            
        case 0x3405:
            dbgPrintf2("*** TEST 43 (%04X)\n", m6502.PC);
            break;
            
        case 0x345D:
            dbgPrintf2("*** TEST 44 (%04X)\n", m6502.PC);
            break;
            
        case 0x3469:
            dbgPrintf2("*** TEST PASSED (%04X)\n", m6502.PC);
            break;
            
        default:
            break;
    }
#endif
    
    disNewInstruction();
    
    switch ( fetch() ) {
        case 0x00: BRK(); return 7;                                    // BRK
        case 0x01: ORA( src_X_ind() ); return 6;                       // ORA X,ind
//        case 0x02: // t jams
//        case 0x03: // SLO* (undocumented)
//        case 0x04: // NOP* (undocumented)
        case 0x05: ORA( src_zp() ); return 3;                          // ORA zpg
        case 0x06: ASL( addr_zp() ); return 5;                         // ASL zpg
//        case 0x07: // SLO* (undocumented)
        case 0x08: PHP(); return 3;                                    // PHP
        case 0x09: ORA( imm() ); return 2;                             // ORA imm
        case 0x0A: ASLA(); return 2;                                   // ASL A
//        case 0x0B: // ANC** (undocumented)
//        case 0x0C: // NOP* (undocumented)
        case 0x0D: ORA( src_abs() ); return 4;                         // ORA abs
        case 0x0E: ASL( addr_abs() ); return 6;                        // ASL abs
//            case 0x0F: // SLO* (undocumented)
        case 0x10: BPL( rel_addr() ); return 3;                        // BPL rel
        case 0x11: ORA( src_ind_Y() ); return 5;                       // ORA ind,Y
//        case 0x12: // t jams
//        case 0x13: // SLO* (undocumented)
//        case 0x14: // NOP* (undocumented)
        case 0x15: ORA( src_zp_X() ); return 4;                        // ORA zpg,X
        case 0x16: ASL( addr_zp_X() ); return 6;                       // ASL zpg,X
//            case 0x17: // SLO* (undocumented)
        case 0x18: CLC(); return 2;                                    // CLC
        case 0x19: ORA( src_abs_Y() ); return 4;                       // ORA abs,Y
//        case 0x1A: // NOP* (undocumented)
//        case 0x1B: // SLO* (undocumented)
//        case 0x1C: // NOP* (undocumented)
        case 0x1D: ORA( src_abs_X() ); return 4;                       // ORA abs,X
        case 0x1E: ASL( addr_abs_X() ); return 7;                      // ASL abs,X
//        case 0x1F: // SLO* (undocumented)
        case 0x20: JSR( abs_addr() ); return 6;                        // JSR abs
        case 0x21: AND( src_X_ind() ); return 6;                       // AND X,ind
//        case 0x22: HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
//        case 0x23: RLA izx 8
        case 0x24: BIT( src_zp() ); return 3;                          // BIT zpg
        case 0x25: AND( src_zp() ); return 3;                          // AND zpg
        case 0x26: ROL( addr_zp() ); return 5;                         // ROL zpg
//        case 0x27: RLA zp 5
        case 0x28: PLP(); return 4;                                    // PLP
        case 0x29: AND( imm() ); return 2;                             // AND imm
        case 0x2A: ROLA(); return 2;                                   // ROL A
//        case 0x2B: ANC imm 2
        case 0x2C: BIT( src_abs() ); return 4;                         // BIT abs
        case 0x2D: AND( src_abs() ); return 4;                         // AND abs
        case 0x2E: ROL( addr_abs() ); return 6;                        // ROL abs
//        case 0x2F: RLA abs 6
        case 0x30: BMI( rel_addr() ); return 3;                        // BMI rel
        case 0x31: AND( src_ind_Y() ); return 5;                       // AND ind,Y
//        case 0x32: HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
//        case 0x33: RLA izy 8
//        case 0x34: NOP zpx 4
        case 0x35: AND( src_zp_X() ); return 4;                        // AND zpg,X
        case 0x36: ROL( addr_zp_X() ); return 6;                       // ROL zpg,X
//        case 0x37: RLA zpx 6
        case 0x38: SEC(); return 2;                                    // SEC
        case 0x39: AND( src_abs_Y() ); return 4;                       // AND abs,Y
//        case 0x3A: NOP 2
//        case 0x3B: RLA aby 7
//        case 0x3C: NOP abx 4
        case 0x3D: AND( src_abs_X() ); return 4;                       // AND abs,X
        case 0x3E: ROL( addr_abs_X() ); return 7;                      // ROL abs,X
//            case 0x3F: RLA abx 7
        case 0x40: RTI(); return 6;                                    // RTI
        case 0x41: EOR( src_X_ind() ); return 6;                       // EOR X,ind
//        case 0x42: HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
//        case 0x43: SRE izx 8
//        case 0x44: NOP zp 3
        case 0x45: EOR( src_zp() ); return 3;                          // EOR zpg
        case 0x46: LSR( addr_zp() ); return 5;                         // LSR zpg
//        case 0x47: SRE zp 5
        case 0x48: PHA(); return 3;                                    // PHA
        case 0x49: EOR( imm() ); return 2;                             // EOR imm
        case 0x4A: LSRA(); return 2;                                   // LSR A
//        case 0x4B: ALR imm 2
        case 0x4C: JMP( abs_addr() ); return 3;                        // JMP abs
        case 0x4D: EOR( src_abs() ); return 4;                         // EOR abs
        case 0x4E: LSR( addr_abs() ); return 6;                        // LSR abs
//        case 0x4F: SRE abs 6
        case 0x50: BVC( rel_addr() ); return 3;                        // BVC rel
        case 0x51: EOR( src_ind_Y() ); return 5;                       // EOR ind,Y
//        case 0x52: HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
//        case 0x53: SRE izy 8
//        case 0x54: NOP zpx 4
        case 0x55: EOR( src_zp_X() ); return 4;                        // AND zpg,X
        case 0x56: LSR( addr_zp_X() ); return 6;                       // LSR zpg,X
//            case 0x57: SRE zpx 6
        case 0x58: CLI(); return 2;                                    // CLI
        case 0x59: EOR( src_abs_Y() ); return 4;                       // EOR abs,Y
//        case 0x5A: NOP 2
//        case 0x5B: SRE aby 7
//        case 0x5C: NOP abx 4
        case 0x5D: EOR( src_abs_X() ); return 4;                       // EOR abs,X
        case 0x5E: LSR( addr_abs_X() ); return 7;                      // LSR abs,X
//            case 0x5F: SRE abx 7
        case 0x60: RTS(); return 6;                                    // RTS
        case 0x61: ADC( src_X_ind() ); return 6;                       // ADC X,ind
//        case 0x62: HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
//        case 0x63: RRA izx 8
//        case 0x64: NOP zp 3
        case 0x65: ADC( src_zp() ); return 3;                          // ADC zpg
        case 0x66: ROR( addr_zp() ); return 5;                         // ROR zpg
//        case 0x67: RRA zp 5
        case 0x68: PLA(); break;                                       // PLA
        case 0x69: ADC( imm() ); return 2;                             // ADC imm
        case 0x6A: RORA(); return 2;                                   // ROR A
//        case 0x6B: ARR imm 2
        case 0x6C: JMP( ind_addr() ); return 5;                        // JMP ind
        case 0x6D: ADC( src_abs() ); return 4;                         // ADC abs
        case 0x6E: ROR( addr_abs() ); return 6;                        // ROR abs
//        case 0x6F: RRA abs 6
        case 0x70: BVS( rel_addr() ); return 3;                        // BVS rel
        case 0x71: ADC( src_ind_Y() ); return 5;                       // ADC ind,Y
//        case 0x72:
//        case 0x73:
//        case 0x74:
        case 0x75: ADC( src_zp_X() ); return 4;                        // ADC zpg,X
        case 0x76: ROR( addr_zp_X() ); return 6;                       // ROR zpg,X
//            case 0x77:
        case 0x78: SEI(); return 2;                                    // SEI
        case 0x79: ADC( src_abs_Y() ); return 4;                       // ADC abs,Y
//        case 0x7A:
//        case 0x7B:
//        case 0x7C:
        case 0x7D: ADC( src_abs_X() ); return 4;                       // ADC abs,X
        case 0x7E: ROR( addr_abs_X() ); return 7;                      // ROR abs,X
//        case 0x7F:
//        case 0x80:
        case 0x81: STA( addr_X_ind() ) ; return 6;                     // STA X,ind
//        case 0x82:
//        case 0x83:
        case 0x84: STY( addr_zp() ); return 3;                         // STY zpg
        case 0x85: STA( addr_zp() ); return 3;                         // STA zpg
        case 0x86: STX( addr_zp() ); return 3;                         // STX zpg
//        case 0x87:
        case 0x88: DEY(); return 2;                                    // DEY
//        case 0x89: NOP(); imm(); return 4;                             // NOP imm
        case 0x8A: TXA(); return 2;                                    // TXA
//        case 0x8B:
        case 0x8C: STY( addr_abs() ); return 4;                        // STY abs
        case 0x8D: STA( addr_abs() ); return 4;                        // STA abs
        case 0x8E: STX( addr_abs() ); return 4;                        // STX abs
//        case 0x8F:
        case 0x90: BCC( rel_addr() ); return 3;                        // BCC rel
        case 0x91: STA( addr_ind_Y() ); return 6;                      // STA ind,Y
//        case 0x92:
//        case 0x93:
        case 0x94: STY( addr_zp_X() ); return 4;                       // STY zpg,X
        case 0x95: STA( addr_zp_X() ); return 4;                       // STA zpg,X
        case 0x96: STX( addr_zp_Y() ); return 4;                       // STX zpg,Y
//        case 0x97:
        case 0x98: TYA(); return 2;                                    // TYA
        case 0x99: STA( addr_abs_Y() ); return 5;                      // STA abs,Y
        case 0x9A: TXS(); return 2;                                    // TXS
//        case 0x9B:
//        case 0x9C:
        case 0x9D: STA( addr_abs_X() ); return 5;                      // STA abs,X
//        case 0x9E:
//        case 0x9F:
        case 0xA0: LDY( imm() ); return 2;                             // LDY imm
        case 0xA1: LDA( src_X_ind() ) ; return 6;                      // LDA X,ind
        case 0xA2: LDX( imm() ); return 2;                             // LDX imm
//        case 0xA3:
        case 0xA4: LDY( src_zp() ); return 3;                          // LDY zpg
        case 0xA5: LDA( src_zp() ); return 3;                          // LDA zpg
        case 0xA6: LDX( src_zp() ); return 3;                          // LDX zpg
//        case 0xA7:
        case 0xA8: TAY(); return 2;                                    // TAY
        case 0xA9: LDA( imm() ); return 2;                             // LDA imm
        case 0xAA: TAX(); return 2;                                    // TAX
//        case 0xAB:
        case 0xAC: LDY( src_abs() ); return 4;                         // LDY abs
        case 0xAD: LDA( src_abs() ); return 4;                         // LDA abs
        case 0xAE: LDX( src_abs() ); return 4;                         // LDX abs
//        case 0xAF:
        case 0xB0: BCS( rel_addr() ); return 3;                        // BCS rel
        case 0xB1: LDA( src_ind_Y() ); return 5;                       // LDA ind,Y
//        case 0xB2:
//        case 0xB3:
        case 0xB4: LDY( src_zp_X() ); return 4;                        // LDY zpg,X
        case 0xB5: LDA( src_zp_X() ); return 4;                        // LDA zpg,X
        case 0xB6: LDX( src_zp_Y() ); return 4;                        // LDX zpg,Y
//        case 0xB7:
        case 0xB8: CLV(); return 2;                                    // CLV
        case 0xB9: LDA( src_abs_Y() ); return 4;                       // LDA abs,Y
        case 0xBA: TSX(); return 2;                                    // TSX
//        case 0xBB:
        case 0xBC: LDY( src_abs_X() ); return 4;                       // LDY abs,X
        case 0xBD: LDA( src_abs_X() ); return 4;                       // LDA abs,X
        case 0xBE: LDX( src_abs_Y() ); return 4;                       // LDX abs,Y
//        case 0xBF:
        case 0xC0: CPY( imm() ); return 2;                             // CPY imm
        case 0xC1: CMP( src_X_ind() ) ; return 6;                      // LDA X,ind
//        case 0xC2:
//        case 0xC3:
        case 0xC4: CPY( src_zp() ); return 3;                          // CPY zpg
        case 0xC5: CMP( src_zp() ); return 3;                          // CMP zpg
        case 0xC6: DEC( addr_zp() ); return 5;                         // DEC zpg
//        case 0xC7:
        case 0xC8: INY(); return 2;                                    // INY
        case 0xC9: CMP( imm() ); return 2;                             // CMP imm
        case 0xCA: DEX(); return 2;                                    // DEX
//        case 0xCB:
        case 0xCC: CPY( src_abs() ); return 4;                         // CPY abs
        case 0xCD: CMP( src_abs() ); return 4;                         // CMP abs
        case 0xCE: DEC( addr_abs() ); return 6;                        // DEC abs
//        case 0xCF:
        case 0xD0: BNE( rel_addr() ); return 3;                        // BNE rel
        case 0xD1: CMP( src_ind_Y() ); return 5;                       // CMP ind,Y
//        case 0xD2:
//        case 0xD3:
//        case 0xD4:
        case 0xD5: CMP( src_zp_X() ); return 4;                        // CMP zpg,X
        case 0xD6: DEC( addr_zp_X() ); return 6;                       // DEC zpg,X
//        case 0xD7:
        case 0xD8: CLD(); return 2;                                    // CLD
        case 0xD9: CMP( src_abs_Y() ); return 4;                       // CMP abs,Y
//        case 0xDA:
//        case 0xDB:
//        case 0xDC:
        case 0xDD: CMP( src_abs_X() ); return 4;                       // CMP abs,X
        case 0xDE: DEC( addr_abs_X() ); return 7;                      // DEC abs,X
//        case 0xDF:
        case 0xE0: CPX( imm() ); return 2;                             // CPX imm
        case 0xE1: SBC( src_X_ind() ) ; return 6;                      // SBC (X,ind)
//        case 0xE2:
//        case 0xE3:
        case 0xE4: CPX( src_zp() ); return 3;                          // CPX zpg
        case 0xE5: SBC( src_zp() ); return 3;                          // SBC zpg
        case 0xE6: INC( addr_zp() ); return 5;                         // INC zpg
//        case 0xE7:
        case 0xE8: INX(); return 2;                                    // INX
        case 0xE9: SBC( imm() ); return 2;                             // SBC imm
        case 0xEA: NOP(); return 2;                                    // NOP
//        case 0xEB:
        case 0xEC: CPX( src_abs() ); return 4;                         // CPX abs
        case 0xED: SBC( src_abs() ); return 4;                         // SBC abs
        case 0xEE: INC( addr_abs() ); return 6;                        // INC abs
//        case 0xEF:
        case 0xF0: BEQ( rel_addr() ); return 3;                        // BEQ rel
        case 0xF1: SBC( src_ind_Y() ); return 5;                       // SBC ind,Y
//        case 0xF2:
//        case 0xF3:
//        case 0xF4:
        case 0xF5: SBC( src_zp_X() ); return 4;                        // SBC zpg,X
        case 0xF6: INC( addr_zp_X() ); return 6;                       // INC zpg,X
//        case 0xF7:
        case 0xF8: SED(); return 2;                                    // SED
        case 0xF9: SBC( src_abs_Y() ); return 4;                       // SBC abs,Y
//        case 0xFA:
//        case 0xFB:
//        case 0xFC:
        case 0xFD: SBC( src_abs_X() ); return 4;                       // SBC abs,X
        case 0xFE: INC( addr_abs_X() ); return 7;                      // INC abs,X
//            case 0xFF:

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


unsigned int clkfrm = 0;

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
}

void m6502_Run() {
    
    // init time
//#ifdef CLK_WAIT
//    unsigned long long elpased = (unsigned long long)-1LL;
//#endif

#ifdef SPEEDTEST
    for ( inst_cnt = 0; inst_cnt < iterations ; inst_cnt++ )
#elif defined( CLK_WAIT )
        // we clear the clkfrm from ViewController Update()
        // we will also use this to pause the simulation if not finished by the end of the frame
        for ( clk_6502_per_frm_max = clk_6502_per_frm; clkfrm < clk_6502_per_frm_max ; clkfrm += m6502_Step() )
#else
    // this is for max speed only -- WARNING! It works only if simulation runs in a completely different thread from the Update()
    for ( ; ; )
#endif
    {
        
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
        read_rom( bundlePath, filename, Apple2_16K_ROM, 0);
        memcpy(Apple2_64K_MEM + 0xC000, Apple2_16K_ROM, 16 * KB);
    }
    
    else if ( flen == 12 * KB ) {
        read_rom( bundlePath, filename, Apple2_16K_ROM, 0x1000);
        memcpy(Apple2_64K_MEM + 0xD000, Apple2_16K_ROM + 0x1000, 12 * KB);
    }

}


void m6502_ColdReset( const char * bundlePath, const char * romFileName ) {
    inst_cnt = 0;
    mhz = (double)MHz_6502 / M;
    
    spkr_init();
    
    unsigned long long saved_frm_set = clk_6502_per_frm_set;
    clk_6502_per_frm     =
    clk_6502_per_frm_max =
    clk_6502_per_frm_set = 0;
    
    // wait 100ms to be sure simulation has been halted
    usleep(100000);
    
    printf("Bundlepath: %s\n", bundlePath);

//    epoch = rdtsc();
//    sleep(1);
//    unsigned long long e = rdtsc();
//    tick_per_sec = e - epoch;
//    tick_6502_per_sec = tick_per_sec / MHz_6502;

    resetMemory();

    outdev = fopen("/Users/trudnai/Library/Containers/com.gamealloy.A2Mac/Data/disassembly_new.log", "w+");
    if (outdev == NULL) {
        outdev = stdout;
    }


    
#ifdef FUNCTIONTEST
    FILE * f = fopen("/Users/trudnai/Library/Containers/com.gamealloy.A2Mac/Data/6502_functional_test.bin", "rb");
    if (f == NULL) {
        perror("Failed: ");
        return;
    }
    
    fread( RAM, 1, 65536, f);
    fclose(f);
    
    m6502.PC = 0x400;

#else
    // Apple ][+ ROM
    
    rom_loadFile(bundlePath, romFileName);
    
    // Disk ][ ROM in Slot 6
    read_rom( bundlePath, "DISK_II_C600.ROM", Apple2_64K_RAM, 0xC600);
    memcpy(Apple2_64K_MEM + 0xC600, Apple2_64K_RAM + 0xC600, 0x100);
    
//    read_rom( "/Users/trudnai/Library/Containers/com.gamealloy.A2Mac/Data/", "DISK_II_C600.ROM", Apple2_64K_MEM, 0xC600);
    
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
    clk_6502_per_frm_set = clk_6502_per_frm = default_MHz_6502 / fps;

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

