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


static inline void PUSH( uint8_t imm ) {
    RAM[ stack_base_addr + m6502.sp-- ] = imm;
}

static inline uint8_t POP() {
    return RAM[ ++m6502.sp + stack_base_addr ];
}

static inline void PUSH_addr( uint16_t addr ) {
    PUSH( (uint8_t)(addr >> 8) );
    PUSH( (uint8_t)addr );
}

static inline uint16_t POP_addr() {
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
static inline void PHA() {
    dbgPrintf("PHA ");
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
static inline void PLA() {
    dbgPrintf("PLA ");
    m6502.A = POP();
    set_flags_NZ(m6502.A);
}

/**
 PHP  Push Processor Status on Stack
 
 push SR                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHP           08    1     3
 **/
static inline void PHP() {
    dbgPrintf("PHP ");
    PUSH( m6502.sr );
}

/**
 PLP  Pull Processor Status from Stack
 
 pull SR                          N Z C I D V
                                  from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLP           28    1     4
 **/
static inline void PLP() {
    dbgPrintf("PLP ");
    m6502.sr = POP();
}

#endif // __6502_INSTR_STACK_H__

