//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_SET_CLR_H__
#define __6502_INSTR_SET_CLR_H__

/**
 CLC  Clear Carry Flag
 
 0 -> C                           N Z C I D V
                                  - - 0 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLC           18    1     2
 **/
static inline void CLC() {
    dbgPrintf("CLC ");
    m6502.flags.C = 0;
}

/**
 CLD  Clear Decimal Mode
 
 0 -> D                           N Z C I D V
                                  - - - - 0 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLD           D8    1     2
 **/
static inline void CLD() {
    dbgPrintf("CLD ");
    m6502.flags.D = 0;
}

/**
 CLI  Clear Interrupt Disable Bit
 
 0 -> I                           N Z C I D V
                                  - - - 0 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLI           58    1     2
 **/
static inline void CLI() {
    dbgPrintf("CLI ");
    m6502.flags.I = 0;
}

/**
 CLV  Clear Overflow Flag
 
 0 -> V                           N Z C I D V
                                  - - - - - 0
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLV           B8    1     2
 **/
static inline void CLV() {
    dbgPrintf("CLV ");
    m6502.flags.V = 0;
}

/**
 SEC  Set Carry Flag
 
 1 -> C                           N Z C I D V
                                  - - 1 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEC           38    1     2
 **/
static inline void SEC() {
    dbgPrintf("SEC ");
    m6502.flags.C = 1;
}

/**
 SED  Set Decimal Flag
 
 1 -> D                           N Z C I D V
                                  - - - - 1 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SED           F8    1     2
 **/
static inline void SED() {
    dbgPrintf("SED ");
    m6502.flags.D = 1;
}

/**
 SEI  Set Interrupt Disable Status
 
 1 -> I                           N Z C I D V
                                  - - - 1 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEI           78    1     2
 **/
static inline void SEI() {
    dbgPrintf("SEI ");
    m6502.flags.I = 1;
}

#endif // __6502_INSTR_SET_CLR_H__

