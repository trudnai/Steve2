//
//  disassembler.h
//  A2Mac
//
//  Created by Tamas Rudnai on 9/21/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
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

#define disNewInstruction() { \
    if ( m6502.dbgLevel.trace ) { \
        snprintf(disassembly.addr, 5, "%04X ", m6502.PC); \
        *disassembly.opcode = '\0'; \
        disassembly.pOpcode = disassembly.opcode; \
        *disassembly.inst = '\0'; \
        *disassembly.oper = '\0'; \
        *disassembly.comment = '\0'; \
    } \
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

        fprintf( f, "%llu\t%llu %s: %-11s%-4s%s\t0x%02X\t0x%02X\t0x%02X\t0x%02X\t0x%02X\t;\t%s\n", // Virtual ][ style
            ++discnt,
            m6502.clktime,
            disassembly.addr,
            disassembly.opcode,
            disassembly.inst,
            disassembly.oper,
            m6502.A,
            m6502.X,
            m6502.Y,
            m6502.SR,
            m6502.SP,
            disassembly.comment
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
