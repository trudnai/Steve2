//
//  6502_dis.c
//  A2Mac
//
//  Created by Tamas Rudnai on 3/13/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#define DISASSEMBLER

#include "6502_dis.h"
#include "6502_instructions.h"


INLINE int m6502_Disass_1_Instr(void) {

    _disNewInstruction();

    switch ( _fetch_dis() ) {
#include "6502_std.h"           // Standard 6502 instructions
//#include "6502_und.h"           // Undocumented 6502 instructions
#include "6502_C.h"             // Extended 65C02 instructions
#include "6502_C_Rockwell.h"    // Extended 65C02 instructions

        default:
            dbgPrintf("%04X: Unimplemented Instruction 0x%02X\n", m6502.PC -1, _memread_dis( m6502.PC -1 ));
            return 2;
    } // switch fetch

    return 2;
}


