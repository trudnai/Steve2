//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __6502_INSTR_UNDOC_H__
#define __6502_INSTR_UNDOC_H__


/**
ANC -     "AND" Memory with Accumulator
        THEN Copy Bit 7 of Result into Carry
 
    (M "AND" A) -> A
    THEN msb(A) -> C
**/
INLINE void ANC ( uint8_t src ) {
    disPrintf(disassembly.inst, "ANC");
    
    set_flags_NZ( m6502.A &= src );
    m6502.C = m6502.A >> 7;
}


/**
 ASR -     "AND" Memory with Accumulator
 THEN Shift Accumulator One Bit Right
 
 (M "AND" A) -> A
 THEN LSR A
 **/
INLINE void ASR ( uint8_t src ) {
    disPrintf(disassembly.inst, "ASR");
    
    // AND
    m6502.A &= src;

    // LSR A
    m6502.C = m6502.A & 1;
    set_flags_NZ( m6502.A >>= 1 );
}


/**
 NOP  No Operation
 
 ---                              N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       NOP           EA    ?     ?
 **/
INLINE void NOP2( uint8_t src ) {
    disPrintf(disassembly.inst, "NOP2");
}



/**
 RLA -     Rotate Memory One Bit Left
 THEN "AND" Memory with Accumulator
 
 ROL M
 AND M
**/
INLINE void RLA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "RLA");

    // ROL M
    uint8_t C = m6502.C != 0;
    m6502.C = WRLOMEM[addr] & 0x80;
    WRLOMEM[addr] <<= 1;
    WRLOMEM[addr] |= C;

    // AND M
    set_flags_NZ( m6502.A &= WRLOMEM[addr] );

}


/**
 RRA -     Rotate Memory One Bit Right
 THEN Add Memory to Accumulator with Carry
 
 ROR M
 ADC M
 **/
INLINE void RRA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "RRA");
    _ROR(addr);
    _ADC(WRLOMEM[addr]);
}


/**
 SLO -     Shift Memory One Bit Left
        THEN "OR" Memory with Accumulator
        into Accumulator and Memory
 
 ASL M
 ORA M
  -> A,M
**/

INLINE void SLO ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SLO");
    
    m6502.C = WRLOMEM[addr] & 0x80;
    WRLOMEM[addr] <<= 1;        // ASL M -> M
    m6502.A ^= WRLOMEM[addr];   // EOR M -> A
    set_flags_NZ( WRLOMEM[addr] = m6502.A );    // A -> M
}


/**
 SRE -     Shift Memory One Bit Right
 THEN "OR" Memory with Accumulator
 
 LSR M
 ORA M
 **/

INLINE void SRE ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SRE");

    // LSR
    m6502.C = WRLOMEM[addr] & 1;
    set_flags_NZ( WRLOMEM[addr] >>= 1 );
    
    // EOR M
    set_flags_NZ( m6502.A |= WRLOMEM[addr] );

}



#endif // __6502_INSTR_UNDOC_H__

