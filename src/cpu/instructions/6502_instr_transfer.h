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

#ifndef __6502_INSTR_TRANSFER_H__
#define __6502_INSTR_TRANSFER_H__


/**
 TAX  Transfer Accumulator to Index X
 
 A -> X                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TAX           AA    1     2
 **/
INSTR void TAX() {
    dbgPrintf("TAX(%02X) ", m6502.A);
    disPrintf(disassembly.inst, "TAX");

#ifndef DISASSEMBLER
    set_flags_NZ(m6502.X = m6502.A);
#endif
}

/**
 TXA  Transfer Index X to Accumulator
 
 X -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXA           8A    1     2
 **/
INSTR void TXA() {
    dbgPrintf("TXA(%02X) ", m6502.X);
    disPrintf(disassembly.inst, "TXA");

#ifndef DISASSEMBLER
    set_flags_NZ(m6502.A = m6502.X);
#endif
}


/**
 TAY  Transfer Accumulator to Index Y
 
 A -> Y                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TAY           A8    1     2
 **/
INSTR void TAY() {
    dbgPrintf("TAY ");
    disPrintf(disassembly.inst, "TAY");

#ifndef DISASSEMBLER
    set_flags_NZ(m6502.Y = m6502.A);
#endif
}

/**
 TYA  Transfer Index Y to Accumulator
 
 Y -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TYA           98    1     2
 **/
INSTR void TYA() {
    dbgPrintf("TYA(%02X) ", m6502.Y);
    disPrintf(disassembly.inst, "TYA");

#ifndef DISASSEMBLER
    set_flags_NZ(m6502.A = m6502.Y);
#endif
}

/**
 TSX  Transfer Stack Pointer to Index X
 
 SP -> X                          N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TSX           BA    1     2
 **/
INSTR void TSX() {
    dbgPrintf("TSX(%02X) ", m6502.SP);
    disPrintf(disassembly.inst, "TSX");

#ifndef DISASSEMBLER
    set_flags_NZ(m6502.X = m6502.SP);
#endif
}

/**
 TXS  Transfer Index X to Stack Register
 
 X -> SP                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXS           9A    1     2
 **/
INSTR void TXS() {
    dbgPrintf("TXS(%02X) ", m6502.X);
    disPrintf(disassembly.inst, "TXS");

#ifndef DISASSEMBLER
    m6502.SP = m6502.X;
#endif
}

#endif // __6502_INSTR_TRANSFER_H__

