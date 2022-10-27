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
// Documentations:
//
// http://nesdev.com/6502_cpu.txt
// http://www.oxyron.de/html/opcodes02.html
// https://macgui.com/kb/article/46
// https://www.masswerk.at/6502/6502_instruction_set.html
//

#define CLK_WAIT

#define DEBUGGER
#define FETCH_ADDR disass_addr

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "6502.h"
#include "speaker.h"


#include "../util/common.h"


#define SOFTRESET_VECTOR    0x3F2

#define NMI_VECTOR          0xFFFA
#define RESET_VECTOR        0xFFFC
#define IRQ_VECTOR          0xFFFE

extern m6502_t m6502;

uint16_t disass_addr = 0xFDED;

#include "../util/disassembler.h"
#include "../dev/mem/mmio.h"



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


#include "6502_instructions.h"

INLINE int m6502_Disass_1_Instr(void) {

    disNewInstruction();
    
    switch ( fetch() ) {
#include "6502_std.h"       // Standard 6502 instructions
//#include "6502_und.h"       // Undocumented 6502 instructions
#include "6502_C.h"         // Extended 65C02 instructions

        default:
            dbgPrintf("%04X: Unimplemented Instruction 0x%02X\n", m6502.PC -1, memread( m6502.PC -1 ));
            return 2;
    } // switch fetch16
    
    return 2;
}


void m6502_Disass(void) {

    // init time
//#ifdef CLK_WAIT
//    unsigned long long elpased = (unsigned long long)-1LL;
//#endif

    m6502.clktime += m6502.clkfrm;
    m6502.clkfrm = 0;
    m6502.lastIO = 0;

    if( diskAccelerator_count ) {
        if( --diskAccelerator_count <= 0 ) {
            // make sure we only adjust clock once to get back to normal
            diskAccelerator_count = 0;
            clk_6502_per_frm = clk_6502_per_frm_set;
        }
    }

#ifdef SPEEDTEST
    for ( inst_cnt = 0; inst_cnt < iterations ; inst_cnt++ )
#elif defined( CLK_WAIT )
        // we clear the clkfrm from ViewController Update()
        // we will also use this to pause the simulation if not finished by the end of the frame
    for ( clk_6502_per_frm_max = clk_6502_per_frm; m6502.clkfrm < clk_6502_per_frm_max ; m6502.clkfrm += m6502_Disass_1_Instr() )
#else
    // this is for max speed only -- WARNING! It works only if simulation runs in a completely different thread from the Update()
    for ( ; ; )
#endif
    {
    }
    // TODO: clkfrm is already increamented!!!
//    printDisassembly(outdev);
        
}



