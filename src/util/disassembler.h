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

#if defined(DISASSEMBLER) || defined(DEBUGGER)

//extern unsigned long long int clktime;
extern unsigned long long discnt;

extern void _disHexB( char ** s, const uint8_t b );
#define disHexB( s, b ) _disHexB( &(s), (b) )

extern void _disHexW( char ** s, const uint16_t w );
#define disHexW( s, w ) _disHexW( &(s), (w) )

extern void _disPuts( char ** s, const char * from );
#define disPuts( s, from ) _disPuts( &(s), (from) )

extern void _disPrintf( char * s, const size_t n, const char * fmt, ... );
#define disPrintf( s, fmt, args... ) _disPrintf( (s), sizeof(s), (fmt), ##args )


// TODO: We should add a new field for clk counter, so we can print that out _before_ execution, not after...
extern void _disNewInstruction(void);
#define disNewInstruction() _disNewInstruction()


#ifdef DISASSEMBLER
extern void printDisassembly( FILE * f );
#endif


#else // DISASSEMBLER

#define disHexB( to, b )
#define disHexW( to, w )
#define disPuts( to, from )
#define disPrintf( s, fmt, args... )
#define disNewInstruction()
#define printDisassembly( f )

#endif // DISASSEMBLER


extern const char * disassemblyLine(void);


#endif /* disassembler_h */
