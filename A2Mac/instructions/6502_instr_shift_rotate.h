//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_SHIFT_ROTATE_H__
#define __6502_INSTR_SHIFT_ROTATE_H__


/**
 ASL  Shift Left One Bit (Memory or Accumulator)
 
 C <- [76543210] <- 0             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ASL A         0A    1     2
 zeropage      ASL oper      06    2     5
 zeropage,X    ASL oper,X    16    2     6
 absolute      ASL oper      0E    3     6
 absolute,X    ASL oper,X    1E    3     7
**/
static inline void ASL( uint8_t * dst ) {
    dbgPrintf("ASL ");
    m6502.flags.C = (*dst & (1<<7)) >> 7;
    *dst <<= 1;
    set_flags_NZ( *dst );
}

/**
 LSR  Shift One Bit Right (Memory or Accumulator)
 
 0 -> [76543210] -> C             N Z C I D V
                                  0 + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   LSR A         4A    1     2
 zeropage      LSR oper      46    2     5
 zeropage,X    LSR oper,X    56    2     6
 absolute      LSR oper      4E    3     6
 absolute,X    LSR oper,X    5E    3     7
**/
static inline void LSR( uint8_t * dst ) {
    dbgPrintf("LSR ");
    m6502.flags.C = *dst & 1;
    *dst >>= 1;
    set_flags_NZ( *dst );
}

/**
 ROL  Rotate One Bit Left (Memory or Accumulator)
 
 C <- [76543210] <- C             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ROL A         2A    1     2
 zeropage      ROL oper      26    2     5
 zeropage,X    ROL oper,X    36    2     6
 absolute      ROL oper      2E    3     6
 absolute,X    ROL oper,X    3E    3     7
**/
static inline void ROL( uint8_t * dst ) {
    dbgPrintf("ROL ");
    uint8_t C = m6502.flags.C;
    m6502.flags.C = (*dst & (1<<7)) >> 7;
    *dst <<= 1;
    *dst |= C;
    set_flags_NZ( *dst );
}

/**
 ROR  Rotate One Bit Right (Memory or Accumulator)
 
 C -> [76543210] -> C             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ROR A         6A    1     2
 zeropage      ROR oper      66    2     5
 zeropage,X    ROR oper,X    76    2     6
 absolute      ROR oper      6E    3     6
 absolute,X    ROR oper,X    7E    3     7
**/
static inline void ROR( uint8_t * dst ) {
    dbgPrintf("ROR ");
    uint8_t C = m6502.flags.C << 7;
    m6502.flags.C = *dst;
    *dst >>= 1;
    *dst |= C;
    set_flags_NZ( *dst );
}


#endif // __6502_INSTR_SHIFT_ROTATE_H__


