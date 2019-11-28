//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_BRANCH_H__
#define __6502_INSTR_BRANCH_H__

INLINE void BRA( int8_t reladdr ) {
    m6502.PC += reladdr;
#ifdef DEBUG
    if ( reladdr == -2 ) {
        dbgPrintf2("Infinite Loop at %04X!\n", m6502.PC);
    }
#endif
    dbgPrintf("BRA %04X ", m6502.PC);
}

/**
 BCC  Branch on Carry Clear
 
 branch on C = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BCC oper      90    2     2**
 **/
INLINE void BCC( int8_t reladdr ) {
    dbgPrintf("BCC ");
    disPrintf(disassembly.inst, "BCC");
    if ( m6502.C == 0 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BCS  Branch on Carry Set
 
 branch on C = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BCS oper      B0    2     2**
 **/
INLINE void BCS( int8_t reladdr ) {
    dbgPrintf("BCS ");
    disPrintf(disassembly.inst, "BCS");
    if ( m6502.C == 1 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BNE  Branch on Result not Zero
 
 branch on Z = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BNE oper      D0    2     2**
 **/
INLINE void BNE( int8_t reladdr ) {
    dbgPrintf("BNE ");
    disPrintf(disassembly.inst, "BNE");
    if ( m6502.Z == 0 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BEQ  Branch on Result Zero
 
 branch on Z = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BEQ oper      F0    2     2**
 **/
INLINE void BEQ( int8_t reladdr ) {
    dbgPrintf("BEQ ");
    disPrintf(disassembly.inst, "BEQ");
    if ( m6502.Z == 1 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BPL  Branch on Result Plus
 
 branch on N = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BPL oper      10    2     2**
 **/
INLINE void BPL( int8_t reladdr ) {
    dbgPrintf("BPL ");
    disPrintf(disassembly.inst, "BPL");
    if ( m6502.N == 0 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BMI  Branch on Result Minus
 
 branch on N = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BMI oper      30    2     2**
 **/
INLINE void BMI( int8_t reladdr ) {
    dbgPrintf("BMI ");
    disPrintf(disassembly.inst, "BMI");
    if ( m6502.N == 1 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BVC  Branch on Overflow Clear
 
 branch on V = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BVC oper      50    2     2**
 **/
INLINE void BVC( int8_t reladdr ) {
    dbgPrintf("BVC ");
    disPrintf(disassembly.inst, "BVC");
    if ( m6502.V == 0 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

/**
 BVS  Branch on Overflow Set
 
 branch on V = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BVC oper      70    2     2**
 **/
INLINE void BVS( int8_t reladdr ) {
    dbgPrintf("BVS ");
    disPrintf(disassembly.inst, "BVS");
    if ( m6502.V == 1 ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
}

#endif // __6502_INSTR_BRANCH_H__
