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

#ifndef __6502_INSTR_CALL_RET_JUMP_H__
#define __6502_INSTR_CALL_RET_JUMP_H__


/**
 JMP  Jump to New Location
 
 (PC+1) -> PCL                    N Z C I D V
 (PC+2) -> PCH                    - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 absolute      JMP oper      4C    3     3
 indirect      JMP (oper)    6C    3     5
 **/
INLINE void JMP( uint16_t addr ) {
    dbgPrintf("JMP %04X ", addr);
    disPrintf(disassembly.inst, "JMP");
//    disPrintf(disassembly.comment, "to:%04X", addr)
#ifdef DEBUG
    if ( addr == m6502.PC - 3 ) {
        dbgPrintf("Infinite Loop at %04X!\n", m6502.PC);
    }
#endif
    m6502.PC = addr;
}

/**
 JSR  Jump to New Location Saving Return Address
 
 push (PC+2),                     N Z C I D V
 (PC+1) -> PCL                    - - - - - -
 (PC+2) -> PCH
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 absolute      JSR oper      20    3     6
 **/
INLINE void JSR( uint16_t addr ) {
    dbgPrintf("JSR ");
    disPrintf(disassembly.inst, "JSR");
    PUSH_addr(m6502.PC -1);
    m6502.PC = addr;
}

/**
 RTS  Return from Subroutine
 
 pull PC, PC+1 -> PC              N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       RTS           60    1     6
 **/
INLINE void RTS() {
    dbgPrintf("RTS ");
    disPrintf(disassembly.inst, "RTS");
    m6502.PC = POP_addr() +1;

    // disk accelerator would only work for a certain amount of time
    // currently it is 200ms simulated times
//    if ( m6502.clktime - disk.clk_last_access > clk_diskAcceleratorTimeout ) {
//        clk_6502_per_frm = clk_6502_per_frm_set;
//    }
}

/**
 RTI  Return from Interrupt
 
 pull SR, pull PC                 N Z C I D V
 from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       RTI           40    1     6
 **/
INLINE void RTI() {
    dbgPrintf("RTI ");
    disPrintf(disassembly.inst, "RTI");
    setFlags( POP() );
//    m6502.I = 0;
    m6502.PC = POP_addr();
}


#endif // __6502_INSTR_CALL_RET_JUMP_H__
