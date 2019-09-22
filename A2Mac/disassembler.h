//
//  disassembler.h
//  A2Mac
//
//  Created by Tamas Rudnai on 9/21/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef disassembler_h
#define disassembler_h

#define NO_DISASSEMBLER


#ifdef DISASSEMBLER

#define disHexB( to, b ) \
    if ( m6502.dbgLevel >= DISASSEMBLY ) { \
        snprintf((to), 4, "%02X ", (b)); \
        to += 3; \
    }

#define disHexW( to, w ) \
    if ( m6502.dbgLevel >= DISASSEMBLY ) { \
        snprintf((to), 6, "%04X ", (w)); \
        to += 5; \
    }

#define disPuts( to, from ) { \
    char * s = from; \
    if ( m6502.dbgLevel >= DISASSEMBLY ) { \
        while ( (*(to)++ = *s++) ); \
    } \
}

#define disPrintf( to, fmt, args... ) { \
    if ( m6502.dbgLevel >= DISASSEMBLY ) { \
        snprintf( (to), sizeof(to), fmt, ##args ); \
    } \
}

#define disNewInstruction() { \
    if ( m6502.dbgLevel >= DISASSEMBLY ) { \
        disassembly.pHex = disassembly.hex; \
        *disassembly.hex = '\0'; \
        *disassembly.inst = '\0'; \
        *disassembly.addr = '\0'; \
        *disassembly.comment = '\0'; \
        snprintf(disassembly.codeAddr, 5, "%04X ", m6502.PC); \
    } \
}

INLINE void printDisassembly() {
    if ( m6502.dbgLevel >= DISASSEMBLY ) {
        printf("%s: %-14s%-6s%-14s%s\n",
               disassembly.codeAddr,
               disassembly.hex,
               disassembly.inst,
               disassembly.addr,
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
