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

INLINE void printDisassembly() {
    if ( m6502.dbgLevel.trace ) {
        printf("%s: %-14s%-6s%-14s%s\n",
               disassembly.addr,
               disassembly.opcode,
               disassembly.inst,
               disassembly.oper,
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
#define printDisassembly()

#endif // DISASSEMBLER



#endif /* disassembler_h */
