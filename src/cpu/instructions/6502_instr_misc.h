//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019,2020 Tamas Rudnai. All rights reserved.
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

#ifndef __6502_INSTR_MISC_H__
#define __6502_INSTR_MISC_H__


/**
 BRK  Force Break
 
 interrupt,                       N Z C I D V
 push PC+2, push SR               - - - 1 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       BRK           00    1     7
 **/
INLINE int BRK() {
    dbgPrintf("BRK ");
    disPrintf(disassembly.inst, "BRK");
    PUSH_addr(m6502.PC +1); // PC +2, however, fetch already incremented it by 1
    // B flag should be set before pushing flags onto the stack
    m6502.B = 1;
    PUSH( getFlags().SR );
    m6502.I = 1;
    m6502.PC = memread16(IRQ_VECTOR);
    
    return 7;
}

/**
 HLT / JAM / KIL  Halts (Hangs / Jams / Kills) the CPU - Well, it hangs it untill the next power cycle
 **/
INLINE void HLT() {
    disPrintf(disassembly.inst, "HLT");
    m6502.interrupt = HALT;
}

/**
 NOP  No Operation
 
 ---                              N Z C I D V
                                 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       NOP           EA    1     2
 **/
INLINE void NOP() {
    dbgPrintf("NOP ");
    disPrintf(disassembly.inst, "NOP");
}

#endif // __6502_INSTR_MISC_H__

