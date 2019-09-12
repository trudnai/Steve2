//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_INC_DEC_H__
#define __6502_INSTR_INC_DEC_H__


/**
 INC  Increment Memory by One
 
 M + 1 -> M                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      INC oper      E6    2     5
 zeropage,X    INC oper,X    F6    2     6
 absolute      INC oper      EE    3     6
 absolute,X    INC oper,X    FE    3     7
**/
static inline void INC( uint8_t * dst ) {
    dbgPrintf("INC %02X -> ", *dst);
    set_flags_NZ( ++(*dst) );
    dbgPrintf("%02X ", *dst);
}

/**
 INX  Increment Index X by One
 
 X + 1 -> X                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       INX           E8    1     2
**/
static inline void INX() {
    dbgPrintf("INX %02X -> ", m6502.X);
    set_flags_NZ( ++m6502.X );
    dbgPrintf("%02X ", m6502.X);
}

/**
 INY  Increment Index Y by One
 
 Y + 1 -> Y                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       INY           C8    1     2
**/
static inline void INY() {
    dbgPrintf("INY %02X -> ", m6502.Y);
    set_flags_NZ( ++m6502.Y );
    dbgPrintf("%02X ", m6502.Y);
}

/**
 DEC  Decrement Memory by One
 
 M - 1 -> M                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      DEC oper      C6    2     5
 zeropage,X    DEC oper,X    D6    2     6
 absolute      DEC oper      CE    3     3
 absolute,X    DEC oper,X    DE    3     7
**/
static inline void DEC( uint8_t * dst ) {
    dbgPrintf("DEC %02X -> ", *dst);
    set_flags_NZ( --(*dst) );
    dbgPrintf("%02X ", *dst);
}

/**
 DEX  Decrement Index X by One
 
 X - 1 -> X                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       DEC           CA    1     2
**/
static inline void DEX() {
    dbgPrintf("DEX %02X -> ", m6502.X);
    set_flags_NZ( --m6502.X );
    dbgPrintf("%02X ", m6502.X);
}

/**
 DEY  Decrement Index Y by One
 
 Y - 1 -> Y                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       DEC           88    1     2
 **/
static inline void DEY() {
    dbgPrintf("DEY %02X -> ", m6502.Y);
    set_flags_NZ( --m6502.Y );
    dbgPrintf("%02X ", m6502.Y);
}

#endif // __6502_INSTR_INC_DEC_H__

