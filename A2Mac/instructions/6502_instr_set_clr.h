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
INLINE void CLC() {
    dbgPrintf("CLC ");
    disPrintf(disassembly.inst, "CLC");
    m6502.C = 0;
}

/**
 CLD  Clear Decimal Mode
 
 0 -> D                           N Z C I D V
                                  - - - - 0 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLD           D8    1     2
 **/
INLINE void CLD() {
    dbgPrintf("CLD ");
    disPrintf(disassembly.inst, "CLD");
    m6502.D = 0;
}

/**
 CLI  Clear Interrupt Disable Bit
 
 0 -> I                           N Z C I D V
                                  - - - 0 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLI           58    1     2
 **/
INLINE void CLI() {
    dbgPrintf("CLI ");
    disPrintf(disassembly.inst, "CLI");
    m6502.I = 0;
}

/**
 CLV  Clear Overflow Flag
 
 0 -> V                           N Z C I D V
                                  - - - - - 0
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLV           B8    1     2
 **/
INLINE void CLV() {
    dbgPrintf("CLV ");
    disPrintf(disassembly.inst, "CLV");
    m6502.V = 0;
}

/**
 SEC  Set Carry Flag
 
 1 -> C                           N Z C I D V
                                  - - 1 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEC           38    1     2
 **/
INLINE void SEC() {
    dbgPrintf("SEC ");
    disPrintf(disassembly.inst, "SEC");
    m6502.C = 1;
}

/**
 SED  Set Decimal Flag
 
 1 -> D                           N Z C I D V
                                  - - - - 1 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SED           F8    1     2
 **/
INLINE void SED() {
    dbgPrintf("SED ");
    disPrintf(disassembly.inst, "SED");
    m6502.D = 1;
}

/**
 SEI  Set Interrupt Disable Status
 
 1 -> I                           N Z C I D V
                                  - - - 1 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEI           78    1     2
 **/
INLINE void SEI() {
    dbgPrintf("SEI ");
    disPrintf(disassembly.inst, "SEI");
    m6502.I = 1;
}

#endif // __6502_INSTR_SET_CLR_H__

