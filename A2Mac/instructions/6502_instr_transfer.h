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
static inline void TAX() {
    dbgPrintf("TAX ");
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
static inline void TXA() {
    dbgPrintf("TXA ");
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
static inline void TAY() {
    dbgPrintf("TAY ");
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
static inline void TYA() {
    dbgPrintf("TYA ");
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
static inline void TSX() {
    dbgPrintf("TSX ");
    set_flags_NZ(m6502.X = m6502.sp);
}

/**
 TXS  Transfer Index X to Stack Register
 
 X -> SP                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       TXS           9A    1     2
 **/
static inline void TXS() {
    dbgPrintf("TXS ");
    m6502.sp = m6502.X;
}

#endif // __6502_INSTR_TRANSFER_H__

