//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019,2020 Tamas Rudnai. All rights reserved.
//
// This file is part of Steve ][ -- The Apple ][ Emulator.
//
// Steve ][ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Steve ][ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Steve ][.  If not, see <https://www.gnu.org/licenses/>.
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
 ARC -     "AND" Memory with Accumulator
 THEN Copy Bit 7 of Result into Carry
 THEN Rotate Accumulator One Bit Right
 (Carry unaffected)
 
 (M "AND" A) -> A
 THEN msb(A) -> C
 THEN ROR A
**/
INLINE void ARC ( uint8_t src ) {
    disPrintf(disassembly.inst, "ARC");

    _AND(src);
    m6502.C = m6502.A >> 7;

    // RORA -- Carry not affected
    uint8_t C = m6502.C != 0;
    m6502.A >>= 1;
    set_flags_NZ( m6502.A |= C << 7);
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
 DCP -     Decrement Memory by One
 THEN Compare Memory with Accumulator
 
 (M - 1) -> M
 THEN CMP M
 **/
INLINE void DCP ( uint16_t addr ) {
    disPrintf(disassembly.inst, "DCP");
    _DEC(addr);
    _CMP(WRLOMEM[addr]);
}


/**
 LAS - Stores {adr} & S into A, X and S
 **/
INLINE void LAS ( uint8_t src ) {
    disPrintf(disassembly.inst, "LAS");
    set_flags_NZ( m6502.A = m6502.X = m6502.SP = m6502.SP & src );
}


/**
 ISB / ISC / INS -     Increment Memory by One
      THEN Subtract Memory from Accumulator with Borrow
 
    (M + 1) -> M
    THEN (A - M - ~C) -> A
 **/
INLINE void ISB ( uint16_t addr ) {
    disPrintf(disassembly.inst, "ISB");
    _INC(addr);
    _SBC(WRLOMEM[addr]);
}


/**
 LAX -     Load Index X and Accumulator with Memory
 
    M -> X,A
 **/
INLINE void LAX ( uint8_t src ) {
    disPrintf(disassembly.inst, "LAX");
    set_flags_NZ(m6502.A = m6502.X = src);
}


/**
 RLA -     Rotate Memory One Bit Left
 THEN "AND" Memory with Accumulator
 
 ROL M
 AND M
**/
INLINE void RLA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "RLA");

    _ROL(addr);
    _AND(WRLOMEM[addr]);
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
 SAS -     "AND" Accumulator with Index X into Stack Pointer
 THEN Store Result "AND" (MSB(Address)+1) in Memory
 
 (A "AND" X) -> SP
  THEN (SP "AND" (MSB(adr)+1)) -> M
 **/

INLINE void SAS ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SAS");
    m6502.SP = m6502.A & m6502.X;
    set_flags_NZ( WRLOMEM[addr] = m6502.SP & ((addr >> 8) + 1) );
}


/**
 SBX -     Store Index X "AND" Accumulator in Memory
 
 Carry and Decimal flags are ignored but the
 Carry flag will be set in substraction. This
 is due to the CMP command, which is executed
 instead of the real SBC.
 
    (A & X) - Immediate -> X
 
 The 'SBX' ($CB) may seem to be very complex operation, even though it
 is a combination of the subtraction of accumulator and parameter, as
 in the 'CMP' instruction, and the command 'DEX'. As a result, both A
 and X are connected to ALU but only the subtraction takes place. Since
 the comparison logic was used, the result of subtraction should be
 normally ignored, but the 'DEX' now happily stores to X the value of
 (A & X) - Immediate.  That is why this instruction does not have any
 decimal mode, and it does not affect the V flag. Also Carry flag will
 be ignored in the subtraction but set according to the result.

 **/
INLINE void SBX ( uint8_t src ) {
    disPrintf(disassembly.inst, "SBX");

    uint16_t tmp;

    // Decimal flag is ignored
    tmp = (m6502.A & m6502.X) - src;
    
    m6502.C = tmp < 0x100;
    m6502.V = ( (m6502.A ^ tmp) & 0x80 ) && ( (m6502.A ^ src) & 0x80 );
    set_flags_NZ( m6502.X = tmp );
}


/**
 SHA -     Store Index X "AND" Accumulator in Memory
 
 (X "AND" A "AND" addr.H) -> M
 **/

INLINE void SHA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SHA");
    set_flags_NZ( WRLOMEM[addr] = m6502.X & m6502.A & ((addr >> 8) + 1) );
}


/**
 SHY -     Store (MSB(Address)+1) "AND" Index Y in Memory
 
 ((MSB(adr)+1) "AND" Y) -> M
 **/

INLINE void SHY ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SHY");
    set_flags_NZ( WRLOMEM[addr] = m6502.Y &((addr >> 8) + 1) );
}


/**
 SHX -     Store (MSB(Address)+1) "AND" Index Y in Memory
 
 ((MSB(adr)+1) "AND" X) -> M
 **/

INLINE void SHX ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SHX");
    set_flags_NZ( WRLOMEM[addr] = m6502.X &((addr >> 8) + 1) );
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
    
    _ASL(addr);
    _ORA( WRLOMEM[addr] );
    set_flags_NZ( WRLOMEM[addr] = m6502.A );    // A -> M
}


/**
 SAX -     Store Accumulator "AND" Index X in Memory
 
   (A "AND" X) -> M
 **/

INLINE void SAX ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SAX");
    set_flags_NZ( WRLOMEM[addr] = m6502.A & m6502.X );
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


/**
XAA -     "AND" Memory with Index X into Accumulator
 
    (M "AND" X) -> A
**/
INLINE void XAA ( uint8_t src ) {
    disPrintf(disassembly.inst, "XAA");
    set_flags_NZ( m6502.A = m6502.X & src );
}



#endif // __6502_INSTR_UNDOC_H__

