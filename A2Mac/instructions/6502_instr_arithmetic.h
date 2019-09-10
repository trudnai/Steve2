//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//


#ifndef __6502_INSTR_ARITHMETIC_H__
#define __6502_INSTR_ARITHMETIC_H__

#include "common.h"

/**
 ADC  Add Memory to Accumulator with Carry
 
 A + M + C -> A, C                N Z C I D V
                                  + + + - - +
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     ADC #oper     69    2     2
 zeropage      ADC oper      65    2     3
 zeropage,X    ADC oper,X    75    2     4
 absolute      ADC oper      6D    3     4
 absolute,X    ADC oper,X    7D    3     4*
 absolute,Y    ADC oper,Y    79    3     4*
 (indirect,X)  ADC (oper,X)  61    2     6
 (indirect),Y  ADC (oper),Y  71    2     5*
**/
static inline void ADC( uint8_t imm ) {
    dbgPrintf("ADC(%02X) A:%02X + %02X ", imm, m6502.A, imm);
    m6502.A += imm + m6502.flags.C;
    set_flags_NZCV( m6502.A );
    dbgPrintf("-> A:%02X ", m6502.A);
}

/**
 SBC  Subtract Memory from Accumulator with Borrow
 
 A - M - C -> A                   N Z C I D V
                                  + + + - - +
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     SBC #oper     E9    2     2
 zeropage      SBC oper      E5    2     3
 zeropage,X    SBC oper,X    F5    2     4
 absolute      SBC oper      ED    3     4
 absolute,X    SBC oper,X    FD    3     4*
 absolute,Y    SBC oper,Y    F9    3     4*
 (indirect,X)  SBC (oper,X)  E1    2     6
 (indirect),Y  SBC (oper),Y  F1    2     5*
**/
static inline void SBC( uint8_t imm ) {
    dbgPrintf("SBC(%02X) ", imm);
    ADC( ~imm );
}

#endif // __6502_INSTR_ARITHMETIC_H__

