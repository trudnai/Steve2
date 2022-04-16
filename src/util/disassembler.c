//
//  disassembler.c
//  A2Mac
//
//  Created by Tamas Rudnai on 3/13/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#include <stdio.h>
#include "disassembler.h"

disassembly_t disassembly;
unsigned long long discnt = 0;

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


#ifdef DISASSEMBLER
void printDisassembly( FILE * f ) {
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
//            ++discnt,
//            disassembly.clk,
//            disassembly.addr,
//            disassembly.opcode,
//            disassembly.inst,
//            disassembly.oper,
//            m6502.A,
//            m6502.X,
//            m6502.Y,
//            getFlags2().SR,
//            m6502.SP
//        );
        
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

        fprintf( f, "%-55s; 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X  ;  %s\n", // Steve ][ style
            line,
            m6502.A,
            m6502.X,
            m6502.Y,
            getFlags2().SR,
            m6502.SP,
            disassembly.comment
        );
        
    }
}
#endif // DISASSEMBLER


