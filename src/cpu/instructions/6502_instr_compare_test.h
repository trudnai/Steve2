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
INSTR void BIT( uint8_t src ) {
    dbgPrintf("BIT(%02X) ", src);
    disPrintf(disassembly.inst, "BIT");
    
#ifndef DISASSEMBLER
    set_flags_NV(src);
    set_flags_Z(m6502.A & src);
#endif
}

/**
 TRB - Test and Reset Bits
 
 TRB can be one the more confusing instructions for a couple of reasons.
 
 First, the term reset is used to refer to the clearing of a bit, whereas the term clear had been used consistently before, such as CLC
 which stands for CLear Carry. Second, the effect on the Z flag is determined by a different function than the effect on memory.
 
 TRB has the same effect on the Z flag that a BIT instruction does. Specifically, it is based on whether the result of a bitwise AND of the
 accumulator with the contents of the memory location specified in the operand is zero. Also, like BIT, the accumulator is not affected.
 
 The accumulator determines which bits in the memory location specified in the operand are cleared and which are not affected.
 The bits in the accumulator that are ones are cleared (in memory), and the bits that are zeros (in the accumulator) are not affected (in memory).
 This is the same as saying that the resulting memory contents are the bitwise AND of the memory contents with the complement of the
 accumulator (i.e. the exclusive-or of the accululator with $FF).
 
 OP LEN CYC MODE FLAGS    SYNTAX
 -- --- --- ---- -----    ------
 14 2   5   zp   ......Z. TRB $12
 1C 3   6   abs  ......Z. TRB $3456
 
 **/
INSTR void TRB( uint16_t addr ) {
    dbgPrintf("TRB(%02X) ", src);
    disPrintf(disassembly.inst, "TRB");

#ifndef DISASSEMBLER
    set_flags_Z( WRLOMEM[addr] & m6502.A );
    WRLOMEM[addr] &= ~m6502.A;
#endif
}

/**
 TSB - Test and Set Bits
 
 TSB, like TRB, can be confusing. For one, like TRB, the effect on the Z flag is determined by a different function than the effect on memory.
 
 TSB, like TRB, has the same effect on the Z flag that a BIT instruction does. Specifically, it is based on whether the result of a bitwise AND
 of the accumulator with the contents of the memory location specified in the operand is zero. Also, like BIT (and TRB), the accumulator is not affected.
 
 The accumulator determines which bits in the memory location specified in the operand are set and which are not affected. The bits in the
 accumulator that are ones are set to one (in memory), and the bits that are zeros (in the accumulator) are not affected (in memory).
 This is the same as saying that the resulting memory contents are the bitwise OR of the memory contents with the accumulator.
 
 Flags affected: Z
 
 OP LEN CYC MODE FLAGS    SYNTAX
 -- --- --- ---- -----    ------
 04 2   5   zp   ......Z. TSB $12
 0C 3   6   abs  ......Z. TSB $3456

 **/
INSTR void TSB( uint16_t addr ) {
    dbgPrintf("TSB(%02X) ", src);
    disPrintf(disassembly.inst, "TSB");

#ifndef DISASSEMBLER
    set_flags_Z( WRLOMEM[addr] & m6502.A );
    WRLOMEM[addr] |= m6502.A;
#endif
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
#ifndef DISASSEMBLER
INSTR void _CMP( uint8_t src ) {
    set_flags_NZC( (int16_t)m6502.A - src );
}
#endif
INSTR void CMP( uint8_t src ) {
    dbgPrintf("CMP(%02X) ", src);
    disPrintf(disassembly.inst, "CMP");

#ifndef DISASSEMBLER
    _CMP(src);
#endif
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
INSTR void CPX( uint8_t src ) {
    dbgPrintf("CPX(%02X) ", src);
    disPrintf(disassembly.inst, "CPX");

#ifndef DISASSEMBLER
    set_flags_NZC( (int16_t)m6502.X - src );
#endif
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
INSTR void CPY( uint8_t src ) {
    dbgPrintf("CPY(%02X) ", src);
    disPrintf(disassembly.inst, "CPY");

#ifndef DISASSEMBLER
    set_flags_NZC( (int16_t)m6502.Y - src );
#endif
}

#endif // __6502_INSTR_COMPARE_TEST_H__

