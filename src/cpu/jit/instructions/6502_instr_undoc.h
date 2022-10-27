//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019, 2020 Tamas Rudnai. All rights reserved.
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
INSTR void ANC ( uint8_t src ) {
    disPrintf(disassembly.inst, "ANC");
    
#ifndef DEBUGGER
    set_flags_NZ( m6502.A &= src );
    m6502.C = m6502.A >> 7;
#endif
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
INSTR void ARC ( uint8_t src ) {
    disPrintf(disassembly.inst, "ARC");

#ifndef DEBUGGER
    _AND(src);
    m6502.C = m6502.A >> 7;

    // RORA -- Carry not affected
    uint8_t C = m6502.C != 0;
    m6502.A >>= 1;
    set_flags_NZ( m6502.A |= C << 7);
#endif
}


/**
 ASR -     "AND" Memory with Accumulator
 THEN Shift Accumulator One Bit Right
 
 (M "AND" A) -> A
 THEN LSR A
 **/
INSTR void ASR ( uint8_t src ) {
    disPrintf(disassembly.inst, "ASR");
    
#ifndef DEBUGGER
    // AND
    m6502.A &= src;

    // LSR A
    m6502.C = m6502.A & 1;
    set_flags_NZ( m6502.A >>= 1 );
#endif
}


/**
 DCP -     Decrement Memory by One
 THEN Compare Memory with Accumulator
 
 (M - 1) -> M
 THEN CMP M
 **/
INSTR void DCP ( uint16_t addr ) {
    disPrintf(disassembly.inst, "DCP");
#ifndef DEBUGGER
    _DEC(addr);
    _CMP(WRLOMEM[addr]);
#endif
}


/**
 LAS - Stores {adr} & S into A, X and S
 **/
INSTR void LAS ( uint8_t src ) {
    disPrintf(disassembly.inst, "LAS");
#ifndef DEBUGGER
    set_flags_NZ( m6502.A = m6502.X = m6502.SP = m6502.SP & src );
#endif
}


/**
 ISB / ISC / INS -     Increment Memory by One
      THEN Subtract Memory from Accumulator with Borrow
 
    (M + 1) -> M
    THEN (A - M - ~C) -> A
 **/
INSTR void ISB ( uint16_t addr ) {
    disPrintf(disassembly.inst, "ISB");
#ifndef DEBUGGER
    _INC(addr);
    _SBC(WRLOMEM[addr]);
#endif
}


/**
 LAX -     Load Index X and Accumulator with Memory
 
    M -> X,A
 **/
INSTR void LAX ( uint8_t src ) {
    disPrintf(disassembly.inst, "LAX");
#ifndef DEBUGGER
    set_flags_NZ(m6502.A = m6502.X = src);
#endif
}


/**
 RLA -     Rotate Memory One Bit Left
 THEN "AND" Memory with Accumulator
 
 ROL M
 AND M
**/
INSTR void RLA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "RLA");

#ifndef DEBUGGER
    _ROL(addr);
    _AND(WRLOMEM[addr]);
#endif
}


/**
 RRA -     Rotate Memory One Bit Right
 THEN Add Memory to Accumulator with Carry
 
 ROR M
 ADC M
 **/
INSTR void RRA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "RRA");
#ifndef DEBUGGER
    _ROR(addr);
    _ADC(WRLOMEM[addr]);
#endif
}


/**
 SAS -     "AND" Accumulator with Index X into Stack Pointer
 THEN Store Result "AND" (MSB(Address)+1) in Memory
 
 (A "AND" X) -> SP
  THEN (SP "AND" (MSB(adr)+1)) -> M
 **/

INSTR void SAS ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SAS");
#ifndef DEBUGGER
    m6502.SP = m6502.A & m6502.X;
    set_flags_NZ( WRLOMEM[addr] = m6502.SP & ((addr >> 8) + 1) );
#endif
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
INSTR void SBX ( uint8_t src ) {
    disPrintf(disassembly.inst, "SBX");

#ifndef DEBUGGER
    uint16_t tmp;

    // Decimal flag is ignored
    tmp = (m6502.A & m6502.X) - src;
    
    m6502.C = tmp < 0x100;
    m6502.V = ( (m6502.A ^ tmp) & 0x80 ) && ( (m6502.A ^ src) & 0x80 );
    set_flags_NZ( m6502.X = tmp );
#endif
}


/**
 SHA -     Store Index X "AND" Accumulator in Memory
 
 (X "AND" A "AND" addr.H) -> M
 **/

INSTR void SHA ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SHA");
#ifndef DEBUGGER
    set_flags_NZ( WRLOMEM[addr] = m6502.X & m6502.A & ((addr >> 8) + 1) );
#endif
}


/**
 SHY -     Store (MSB(Address)+1) "AND" Index Y in Memory
 
 ((MSB(adr)+1) "AND" Y) -> M
 **/

INSTR void SHY ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SHY");
#ifndef DEBUGGER
    set_flags_NZ( WRLOMEM[addr] = m6502.Y &((addr >> 8) + 1) );
#endif
}


/**
 SHX -     Store (MSB(Address)+1) "AND" Index Y in Memory
 
 ((MSB(adr)+1) "AND" X) -> M
 **/

INSTR void SHX ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SHX");
#ifndef DEBUGGER
    set_flags_NZ( WRLOMEM[addr] = m6502.X &((addr >> 8) + 1) );
#endif
}


/**
 SLO -     Shift Memory One Bit Left
        THEN "OR" Memory with Accumulator
        into Accumulator and Memory
 
 ASL M
 ORA M
  -> A,M
**/

INSTR void SLO ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SLO");
    
#ifndef DEBUGGER
    _ASL(addr);
    _ORA( WRLOMEM[addr] );
    set_flags_NZ( WRLOMEM[addr] = m6502.A );    // A -> M
#endif
}


/**
 SAX -     Store Accumulator "AND" Index X in Memory
 
   (A "AND" X) -> M
 **/

INSTR void SAX ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SAX");
#ifndef DEBUGGER
    set_flags_NZ( WRLOMEM[addr] = m6502.A & m6502.X );
#endif
}


/**
 SRE -     Shift Memory One Bit Right
 THEN "OR" Memory with Accumulator
 
 LSR M
 ORA M
 **/

INSTR void SRE ( uint16_t addr ) {
    disPrintf(disassembly.inst, "SRE");

#ifndef DEBUGGER
    // LSR
    m6502.C = WRLOMEM[addr] & 1;
    set_flags_NZ( WRLOMEM[addr] >>= 1 );
    
    // EOR M
    set_flags_NZ( m6502.A |= WRLOMEM[addr] );
#endif
}


/**
XAA -     "AND" Memory with Index X into Accumulator
 
    (M "AND" X) -> A
**/
INSTR void XAA ( uint8_t src ) {
    disPrintf(disassembly.inst, "XAA");
#ifndef DEBUGGER
    set_flags_NZ( m6502.A = m6502.X & src );
#endif
}



#endif // __6502_INSTR_UNDOC_H__

