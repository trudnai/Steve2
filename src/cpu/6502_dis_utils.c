//
//  6502_dis_utils.c
//  A2Mac
//
//  Created by Tamas Rudnai on 3/13/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#define DISASSEMBLER

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "6502_dis_utils.h"
#include "6502.h"
#include "6502_bp.h"


disassembly_t disassembly;
unsigned long long discnt = 0;

INLINE flags_t getFlags2(void) {
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


void _disHexB( char ** s, const uint8_t b ) {
//    if ( m6502.dbgLevel.trace ) {
    if ( (*s >= disassembly.opcode) && (*s < disassembly.opcode + sizeof(disassembly.opcode)) ) {
        snprintf(*s, 4, "%02X ", b);
        *s += 3;
    }
    else {
        fprintf(stderr, "_disHexB *s is WRONG! (%p vs %p)\n", *s, disassembly.opcode);
    }

//    }
}

void _disHexW( char ** s, const uint16_t w ) {
//    if ( m6502.dbgLevel.trace ) {
        snprintf(*s, 6, "%04X ", w);
        *s += 5;
//    }
}

void _disPuts( char ** s, const char * from ) {
//    if ( m6502.dbgLevel.trace ) {
        while ( (*(*s)++ = *from++) );
//    }
}


void _disPrintf( char * s, const size_t n, const char * fmt, ... ) {
//    if ( m6502.dbgLevel.trace ) {
        va_list arg_ptr;
        
        va_start(arg_ptr, fmt);
        vsnprintf( s, n, fmt, arg_ptr );
        va_end(arg_ptr);
//    }
}

INLINE uint8_t memread8_low( uint16_t addr );

void _disNewInstruction(void) {
//    if ( m6502.dbgLevel.trace ) {
        memset( &disassembly, 0, sizeof(disassembly) );
        disassembly.clk = m6502.clktime + m6502.clkfrm;
        disassembly.addr = m6502.PC;
        snprintf(disassembly.hexAddr, 5, "%04X ", m6502.PC);
        disassembly.pOpcode = disassembly.opcode;
        disassembly.opcode[0] = '\0';
        disassembly.inst[0] = '\0';
        disassembly.oper[0] = '\0';
        disassembly.comment[0] = '\0';
//    }

    disassembly.op = memread8_low(m6502.PC);
}



#ifdef DISASS_TRACE
void printDisassembly( FILE * f ) {
    if ( m6502.debugger.mask.trace && f ) {
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
             disassembly.hexAddr,
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

#endif // DISASS_TRACE


const char * disassemblyLine(_Bool highlight) {
    static char line[256];

    snprintf( line, sizeof(line), "%s %s: %-11s%-4s%s",
        m6502_dbg_bp_exists(breakpoints, disassembly.addr) ? "*" : " ",
        disassembly.hexAddr,
        disassembly.opcode,
        disassembly.inst,
        disassembly.oper
    );

    if (highlight) {
        for (int i = 0; i < sizeof(line); i++) {
            line[i] &= 0x3F;
        }
    }

    return line;
}

