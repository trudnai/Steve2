//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_COMPARE_TEST_H__
#define __6502_INSTR_COMPARE_TEST_H__

/**
 BIT  Test Bits in Memory with Accumulator
 
 bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
 the zeroflag is set to the result of operand AND accumulator.
 
 A AND M, M7 -> N, M6 -> V        N Z C I D V
                                 M7 + - - - M6
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      BIT oper      24    2     3
 absolute      BIT oper      2C    3     4
 
 **/
INLINE void BIT( uint8_t src ) {
    dbgPrintf("BIT(%02X) ", src);
    disPrintf(disassembly.inst, "BIT");
    set_flags_NV(src);
    set_flags_Z(m6502.A & src);
}

/**
 CMP  Compare Memory with Accumulator
 
 A - M                            N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     CMP #oper     C9    2     2
 zeropage      CMP oper      C5    2     3
 zeropage,X    CMP oper,X    D5    2     4
 absolute      CMP oper      CD    3     4
 absolute,X    CMP oper,X    DD    3     4*
 absolute,Y    CMP oper,Y    D9    3     4*
 (indirect,X)  CMP (oper,X)  C1    2     6
 (indirect),Y  CMP (oper),Y  D1    2     5*
 **/
INLINE void CMP( uint8_t src ) {
    dbgPrintf("CMP(%02X) ", src);
    disPrintf(disassembly.inst, "CMP");
    set_flags_NZC( (int16_t)m6502.A - src );
}

/**
 CPX  Compare Memory and Index X
 
 X - M                            N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     CPX #oper     E0    2     2
 zeropage      CPX oper      E4    2     3
 absolute      CPX oper      EC    3     4
 **/
INLINE void CPX( uint8_t src ) {
    dbgPrintf("CPX(%02X) ", src);
    disPrintf(disassembly.inst, "CPX");
    set_flags_NZC( (int16_t)m6502.X - src );
}

/**
 CPY  Compare Memory and Index Y
 
 Y - M                            N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     CPY #oper     C0    2     2
 zeropage      CPY oper      C4    2     3
 absolute      CPY oper      CC    3     4
 **/
INLINE void CPY( uint8_t src ) {
    dbgPrintf("CPY(%02X) ", src);
    disPrintf(disassembly.inst, "CPY");
    set_flags_NZC( (int16_t)m6502.Y - src );
}

#endif // __6502_INSTR_COMPARE_TEST_H__

