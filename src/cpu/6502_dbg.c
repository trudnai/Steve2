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
#define DISASSEMBLER

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

#include "6502_dis.h"
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


#include "6502_dbg.h"
#include "6502_instructions.h"
#include "6502_bp.h"

INLINE int m6502_Disass_1_Instr(void) {

    _disNewInstruction();
    
    switch ( _fetch_dis() ) {
#include "6502_std.h"       // Standard 6502 instructions
//#include "6502_und.h"       // Undocumented 6502 instructions
#include "6502_C.h"         // Extended 65C02 instructions

        default:
            dbgPrintf("%04X: Unimplemented Instruction 0x%02X\n", m6502.PC -1, _memread_dis( m6502.PC -1 ));
            return 2;
    } // switch fetch
    
    return 2;
}


void m6502_Debug(void) {
    m6502.clktime += m6502.clkfrm;
    //    m6502.clkfrm = 0;
    m6502.lastIO = 0;
    m6502.interrupt = NO_INT; // TODO: This should be taken care by the interrupt handler

    m6502_dbg_on();

    if( diskAccelerator_count ) {
        if( --diskAccelerator_count <= 0 ) {
            // make sure we only adjust clock once to get back to normal
            diskAccelerator_count = 0;
            clk_6502_per_frm = clk_6502_per_frm_set;
        }
    }

    clk_6502_per_frm_max = clk_6502_per_frm;

    for ( m6502.clkfrm = m6502_Step(); m6502.clkfrm < clk_6502_per_frm_max ; m6502.clkfrm += m6502_Step() ) {
        switch (m6502.interrupt) {
            case HALT:
                if (m6502.debugger.mask.hlt) {
                    cpuState = cpuState_halted;
                    //                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case BREAK:
                if (m6502.debugger.mask.brk) {
                    cpuState = cpuState_halted;
                    //                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case IRQ:
                if (m6502.debugger.mask.irq) {
                    cpuState = cpuState_halted;
                    //                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case NMI:
                if (m6502.debugger.mask.nmi) {
                    cpuState = cpuState_halted;
                    //                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case INV:
                if (m6502.debugger.mask.inv) {
                    cpuState = cpuState_halted;
                    //                    m6502.debugger.wMask = 0;
                    return;
                }
                break;

            case RET:
                // Step_Out & Step_Over: Return to caller
                if (m6502.debugger.mask.out) {
                    if ( m6502.SP >= m6502.debugger.SP ) {
                        cpuState = cpuState_halted;
                        //                        m6502.debugger.wMask = 0;
                        return;
                    }
                }
                break;

            case HARDRESET:
                hardReset();
                break;

            case SOFTRESET:
                softReset();
                break;

            default:
                // Step_Out: If there was a POP (PLA, PLX, PLY, PLP), then we should update the monitoring stack pointer
                // (so we can return to the caller, not stopping at the POP)
                if ( m6502.SP > m6502.debugger.SP ) {
                    m6502.debugger.SP = m6502.SP;
                }
                break;
        }

        m6502.interrupt = NO_INT;

        if ( m6502_dbg_bp_exists(breakpoints, bp_last_idx, m6502.PC) ) {
            cpuState = cpuState_halted;
//            m6502.debugger.wMask = 0;
//            m6502.debugger.on = 0;
            m6502.interrupt = BREAKPOINT;
            return;
        }

    }

    // play the entire sound buffer for this frame
    spkr_update();
    // this will take care of turning off disk motor sound when time is up
    spkr_update_disk_sfx();

}


/// Turn On Debugger
void m6502_dbg_on(void) {
    m6502.debugger.on = 1;
}


/// Turn Off Debugger
void m6502_dbg_off(void) {
    m6502.debugger.on = 0;
}


/// Initialize Breakpoints
void m6502_dbg_init(void) {
    m6502_dbg_on();
    m6502.debugger.wMask = 0;
    m6502.debugger.mask.hlt = 1;
    m6502.debugger.mask.brk = 1;
    m6502.debugger.mask.inv = 1;
    m6502.debugger.SP = 0xFF;
    m6502_dbg_bp_del_all(breakpoints);
}


