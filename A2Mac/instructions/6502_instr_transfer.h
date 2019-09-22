//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
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
INLINE void TAX() {
    dbgPrintf("TAX(%02X) ", m6502.A);
    disPrintf(disassembly.inst, "TAX");
    set_flags_NZ(m6502.X = m6502.A);
}

/**
 TXA  Transfer Index X to Accumulator
 
 X -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXA           8A    1     2
 **/
INLINE void TXA() {
    dbgPrintf("TXA(%02X) ", m6502.X);
    disPrintf(disassembly.inst, "TXA");
    set_flags_NZ(m6502.A = m6502.X);
}


/**
 TAY  Transfer Accumulator to Index Y
 
 A -> Y                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TAY           A8    1     2
 **/
INLINE void TAY() {
    dbgPrintf("TAY ");
    disPrintf(disassembly.inst, "TAY");
    set_flags_NZ(m6502.Y = m6502.A);
}

/**
 TYA  Transfer Index Y to Accumulator
 
 Y -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TYA           98    1     2
 **/
INLINE void TYA() {
    dbgPrintf("TYA(%02X) ", m6502.Y);
    disPrintf(disassembly.inst, "TYA");
    set_flags_NZ(m6502.A = m6502.Y);
}

/**
 TSX  Transfer Stack Pointer to Index X
 
 SP -> X                          N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TSX           BA    1     2
 **/
INLINE void TSX() {
    dbgPrintf("TSX(%02X) ", m6502.SP);
    disPrintf(disassembly.inst, "TSX");
    set_flags_NZ(m6502.X = m6502.SP);
}

/**
 TXS  Transfer Index X to Stack Register
 
 X -> SP                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXS           9A    1     2
 **/
INLINE void TXS() {
    dbgPrintf("TXS(%02X) ", m6502.X);
    disPrintf(disassembly.inst, "TXS");
    m6502.SP = m6502.X;
}

#endif // __6502_INSTR_TRANSFER_H__

