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

#ifdef DISASSEMBLER

//extern unsigned long long int clktime;
unsigned long long discnt = 0;

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

INLINE flags_t getFlags2() {
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


INLINE void printDisassembly( FILE * f ) {
    if ( m6502.dbgLevel.trace && f ) {
//        fprintf( f, "%s: %-14s%-6s%-14s%-16s A:%02X X:%02X Y:%02X S:%02X P:%02X (%c%c%c%c%c%c%c%c)\n",
//            disassembly.addr,
//            disassembly.opcode,
//            disassembly.inst,
//            disassembly.oper,
//            disassembly.comment,
//            m6502.A,
//            m6502.X,
//            m6502.Y,
//            m6502.SP,
//            m6502.SR,
//            m6502.N ? 'N' : 'n',
//            m6502.V ? 'V' : 'v',
//            m6502.res ? 'R' : 'r',
//            m6502.B ? 'B' : 'b',
//            m6502.D ? 'D' : 'd',
//            m6502.I ? 'I' : 'i',
//            m6502.Z ? 'Z' : 'z',
//            m6502.C ? 'C' : 'c'
//        );

//        fprintf( f, "%llu\t%llu %s: %-11s%-4s%s\t0x%02X\t0x%02X\t0x%02X\t0x%02X\t0x%02X\t;\t%s\n", // Virtual ][ style
//            ++discnt,
//            m6502.clktime + clkfrm,
//            disassembly.addr,
//            disassembly.opcode,
//            disassembly.inst,
//            disassembly.oper,
//            m6502.A,
//            m6502.X,
//            m6502.Y,
//            0,
//            //getFlags2(),
//            m6502.SP,
//            disassembly.comment
//        );

//        // Virtual ][ Style
//        fprintf( f, "%llu\t%llu\t%s: %-11s%-4s%s\t0x%02X\t0x%02X\t0x%02X\t0x%02X\t0x%02X\n", // Virtual ][ style
//        ++discnt,
//        disassembly.clk,
//        disassembly.addr,
//        disassembly.opcode,
//        disassembly.inst,
//        disassembly.oper,
//        m6502.A,
//        m6502.X,
//        m6502.Y,
//        getFlags2().SR,
//        m6502.SP
//        );
//
//    }

        static char line[256];
        // Steve ][ Style
        snprintf( line, sizeof(line), "%10llu %10llu %s: %-11s%-4s%s", // Steve ][ style
                ++discnt,
                disassembly.clk,
                disassembly.addr,
                disassembly.opcode,
                disassembly.inst,
                disassembly.oper
        );
        
        fprintf( f, "%-55s; 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", // Steve ][ style
                line,
                m6502.A,
                m6502.X,
                m6502.Y,
                getFlags2().SR,
                m6502.SP
                );

    }
}

#else // DISASSEMBLER

#define disHexB( to, b )
#define disHexW( to, w )
#define disPuts( to, from )
#define disPrintf( to, fmt, args... )
#define disNewInstruction()
#define printDisassembly( f )

#endif // DISASSEMBLER



#endif /* disassembler_h */
