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

#ifndef __6502_INSTR_SET_CLR_H__
#define __6502_INSTR_SET_CLR_H__

/**
 CLC  Clear Carry Flag
 
 0 -> C                           N Z C I D V
                                  - - 0 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLC           18    1     2
 **/
INLINE void CLC() {
    dbgPrintf("CLC ");
    disPrintf(disassembly.inst, "CLC");
#ifndef DISASSEMBLER
    m6502.C = 0;
#endif
}

/**
 CLD  Clear Decimal Mode
 
 0 -> D                           N Z C I D V
                                  - - - - 0 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLD           D8    1     2
 **/
INLINE void CLD() {
    dbgPrintf("CLD ");
    disPrintf(disassembly.inst, "CLD");
#ifndef DISASSEMBLER
    m6502.D = 0;
#endif
}

/**
 CLI  Clear Interrupt Disable Bit
 
 0 -> I                           N Z C I D V
                                  - - - 0 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLI           58    1     2
 **/
INLINE void CLI() {
    dbgPrintf("CLI ");
    disPrintf(disassembly.inst, "CLI");
#ifndef DISASSEMBLER
    m6502.I = 0;
#endif
}

/**
 CLV  Clear Overflow Flag
 
 0 -> V                           N Z C I D V
                                  - - - - - 0
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       CLV           B8    1     2
 **/
INLINE void CLV() {
    dbgPrintf("CLV ");
    disPrintf(disassembly.inst, "CLV");
#ifndef DISASSEMBLER
    m6502.V = 0;
#endif
}

/**
 SEC  Set Carry Flag
 
 1 -> C                           N Z C I D V
                                  - - 1 - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEC           38    1     2
 **/
INLINE void SEC() {
    dbgPrintf("SEC ");
    disPrintf(disassembly.inst, "SEC");
#ifndef DISASSEMBLER
    m6502.C = 1;
#endif
}

/**
 SED  Set Decimal Flag
 
 1 -> D                           N Z C I D V
                                  - - - - 1 -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SED           F8    1     2
 **/
INLINE void SED() {
    dbgPrintf("SED ");
    disPrintf(disassembly.inst, "SED");
#ifndef DISASSEMBLER
    m6502.D = 1;
#endif
}

/**
 SEI  Set Interrupt Disable Status
 
 1 -> I                           N Z C I D V
                                  - - - 1 - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       SEI           78    1     2
 **/
INLINE void SEI() {
    dbgPrintf("SEI ");
    disPrintf(disassembly.inst, "SEI");
#ifndef DISASSEMBLER
    m6502.I = 1;
#endif
}

/**
 RMB SMB - Reset or Set Memory Bit
 
 RMB and SMB clear (RMB) or set (SMB) the specified bit in the specified zero page location,
 and can be used in conjuction with the BBR and BBS instructions. Again, note that as with BBR and TRB,
 the term reset in RMB is used to mean clear.
 
 The function of RMB and SMB is very similar to the function of TRB and TSB, except that RMB and SMB
 can clear or set only one zero page bit, whereas TRB and TSB can clear or set any number of bits. Also,
 only zero page addressing is available with RMB and SMB, whereas zero page and absolute addressing
 are available for both TRB and TSB. As a result, RMB and SMB do not offer much that isn't already available
 with TRB and TSB (which are available on 65C02s from all manufacturers). The main advantages are that
 RMB and SMB, unlike TRB and TSB, do not use the accumulator, leaving it available, and do not affect any flags.
 However, it is worth noting that it is rarely useful to preserve the value of the Z (zero) flag (the only flag affected by
 TRB and TSB), unlike other flags (such as the carry).
 
 Like BBR and BBS, the bit to test is typically specified as part of the instruction name rather than the operand, i.e.
 
 Flags affected: none
 
 OP LEN CYC MODE FLAGS    SYNTAX
 -- --- --- ---- -----    ------
 07 2   5   zp   ........ RMB0 $12
 17 2   5   zp   ........ RMB1 $12
 27 2   5   zp   ........ RMB2 $12
 37 2   5   zp   ........ RMB3 $12
 47 2   5   zp   ........ RMB4 $12
 57 2   5   zp   ........ RMB5 $12
 67 2   5   zp   ........ RMB6 $12
 77 2   5   zp   ........ RMB7 $12
 87 2   5   zp   ........ SMB0 $12
 97 2   5   zp   ........ SMB1 $12
 A7 2   5   zp   ........ SMB2 $12
 B7 2   5   zp   ........ SMB3 $12
 C7 2   5   zp   ........ SMB4 $12
 D7 2   5   zp   ........ SMB5 $12
 E7 2   5   zp   ........ SMB6 $12
 F7 2   5   zp   ........ SMB7 $12
**/
#ifndef DISASSEMBLER
#define RMB(n) INLINE void RMB##n( uint8_t zpg ) { \
    dbgPrintf("RMB"#n" "); \
    disPrintf(disassembly.inst, "RMB"#n); \
    WRLOMEM[zpg] &= ~(1 << n); \
}
#else
#define RMB(n) INLINE void RMB##n( uint8_t zpg ) { \
    dbgPrintf("RMB"#n" "); \
    disPrintf(disassembly.inst, "RMB"#n); \
}
#endif

    RMB(0)
    RMB(1)
    RMB(2)
    RMB(3)
    RMB(4)
    RMB(5)
    RMB(6)
    RMB(7)


#ifndef DISASSEMBLER
#define SMB(n) INLINE void SMB##n( uint8_t zpg ) { \
    dbgPrintf("SMB"#n" "); \
    disPrintf(disassembly.inst, "SMB"#n); \
    WRLOMEM[zpg] |= (1 << n); \
}
#else
#define SMB(n) INLINE void SMB##n( uint8_t zpg ) { \
    dbgPrintf("SMB"#n" "); \
    disPrintf(disassembly.inst, "SMB"#n); \
}
#endif

    SMB(0)
    SMB(1)
    SMB(2)
    SMB(3)
    SMB(4)
    SMB(5)
    SMB(6)
    SMB(7)


#endif // __6502_INSTR_SET_CLR_H__

