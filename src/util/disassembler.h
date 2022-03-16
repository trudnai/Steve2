//
//  disassembler.h
//  Steve ][
//
//  Created by Tamas Rudnai on 9/21/19.
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

#ifndef disassembler_h
#define disassembler_h

#include "6502.h"
#include "common.h"


typedef struct disassembly_s {
    uint64_t clk;                   // clock time
    char addr[5];                   // 4 digits + \0
    char opcode[4 * 3 + 1];         // max 4 bytes * (2 digits + 1 space) + \0
    char * pOpcode;                 // pointer for opcode string builder
    char inst[6 + 1];               // 3 char (unknown instr? -- give it 6 chars) + \0
    char oper[14 + 2 + 1 + 1 + 1];  // 4 digits + 2 brackets + 1 comma + 1 index + \0
    char comment[256];              // to be able to add some comments
} disassembly_t;

extern disassembly_t disassembly;

//#define DISASSEMBLER
//#undef DISASSEMBLER

#ifdef DISASSEMBLER

//extern unsigned long long int clktime;
extern unsigned long long discnt;

#define disHexB( to, b ) \
    if ( m6502.dbgLevel.trace ) { \
        snprintf((to), 4, "%02X ", (b)); \
        to += 3; \
    }

#define disHexW( to, w ) \
    if ( m6502.dbgLevel.trace ) { \
        snprintf((to), 6, "%04X ", (w)); \
        to += 5; \
    }

#define disPuts( to, from ) { \
    char * s = from; \
    if ( m6502.dbgLevel.trace ) { \
        while ( (*(to)++ = *s++) ); \
    } \
}

#define disPrintf( to, fmt, args... ) { \
    if ( m6502.dbgLevel.trace ) { \
        snprintf( (to), sizeof(to), fmt, ##args ); \
    } \
}

// TODO: We should add a new field for clk counter, so we can print that out _before_ execution, not after...
#define disNewInstruction() { \
    if ( m6502.dbgLevel.trace ) { \
        disassembly.clk = m6502.clktime + m6502.clkfrm; \
        snprintf(disassembly.addr, 5, "%04X ", m6502.PC); \
        disassembly.opcode[0] = '\0'; \
        disassembly.pOpcode = disassembly.opcode; \
        disassembly.inst[0] = '\0'; \
        disassembly.oper[0] = '\0'; \
        disassembly.comment[0] = '\0'; \
    } \
}

extern void printDisassembly( FILE * f );

#else // DISASSEMBLER

#define disHexB( to, b )
#define disHexW( to, w )
#define disPuts( to, from )
#define disPrintf( to, fmt, args... )
#define disNewInstruction()
#define printDisassembly( f )

#endif // DISASSEMBLER



#endif /* disassembler_h */
