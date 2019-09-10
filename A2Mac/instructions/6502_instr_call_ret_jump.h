//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_CALL_RET_JUMP_H__
#define __6502_INSTR_CALL_RET_JUMP_H__


/**
 JMP  Jump to New Location
 
 (PC+1) -> PCL                    N Z C I D V
 (PC+2) -> PCH                    - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 absolute      JMP oper      4C    3     3
 indirect      JMP (oper)    6C    3     5
 **/
static inline void JMP( uint16_t addr ) {
    dbgPrintf("JMP ");
    m6502.pc = addr;
}

/**
 JSR  Jump to New Location Saving Return Address
 
 push (PC+2),                     N Z C I D V
 (PC+1) -> PCL                    - - - - - -
 (PC+2) -> PCH
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 absolute      JSR oper      20    3     6
 **/
static inline void JSR( uint16_t addr ) {
    dbgPrintf("JSR ");
    PUSH_addr(m6502.pc);
    JMP( addr );
}

/**
 RTS  Return from Subroutine
 
 pull PC, PC+1 -> PC              N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       RTS           60    1     6
 **/
static inline void RTS() {
    dbgPrintf("RTS ");
    JMP( POP_addr() );
}

/**
 RTI  Return from Interrupt
 
 pull SR, pull PC                 N Z C I D V
 from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       RTI           40    1     6
 **/
static inline void RTI() {
    dbgPrintf("RTI ");
    m6502.sr = POP();
    RTS();
}


#endif // __6502_INSTR_CALL_RET_JUMP_H__
