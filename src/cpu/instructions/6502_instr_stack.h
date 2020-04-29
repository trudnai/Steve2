//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_STACK_H__
#define __6502_INSTR_STACK_H__

static const uint16_t stack_base_addr = 0x100;


INLINE void PUSH( uint8_t src ) {
//    RAM[ stack_base_addr | m6502.SP-- ] = src;
    // DO NOT MAKE IT NICER! faster this way!
    *( RAM_PG_WR_TBL[ stack_base_addr >> 8 ] + m6502.SP-- ) = src;
}

INLINE uint8_t POP() {
//    return RAM[ stack_base_addr | ++m6502.SP ];
    return *( RAM_PG_WR_TBL[ stack_base_addr >> 8 ] + ++m6502.SP );
}


INLINE void PUSH_addr( uint16_t addr ) {
    PUSH( (uint8_t)(addr >> 8) );
    PUSH( (uint8_t)addr );
}

INLINE uint16_t POP_addr() {
    return  POP() + ( POP() << 8 );
}


/**
 PHA  Push Accumulator on Stack
 
 push A                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHA           48    1     3
 **/
INLINE void PHA() {
    dbgPrintf("PHA %02X ", m6502.A);
    disPrintf(disassembly.inst, "PHA");
    PUSH( m6502.A );
}

/**
 PLA  Pull Accumulator from Stack
 
 pull A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLA           68    1     4
 **/
INLINE void PLA() {
    m6502.A = POP();
    dbgPrintf("PLA %02X ", m6502.A);
    disPrintf(disassembly.inst, "PLA");
    set_flags_NZ( m6502.A );
}

/**
 PHP  Push Processor Status on Stack
 
 push SR                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHP           08    1     3
 **/
INLINE void PHP() {
    dbgPrintf("PHP %02X ", m6502.SR);
    disPrintf(disassembly.inst, "PHP");
    PUSH( m6502.SR ); // res and B flag should be set
}

/**
 PLP  Pull Processor Status from Stack
 
 pull SR                          N Z C I D V
                                  from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLP           28    1     4
 **/
INLINE void PLP() {
    m6502.SR = POP() | 0x30; // res and B flag should be set
    dbgPrintf("PLP %02X ", m6502.SR);
    disPrintf(disassembly.inst, "PLP");
}

#endif // __6502_INSTR_STACK_H__

