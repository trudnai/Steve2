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

#ifndef __6502_INSTR_BRANCH_H__
#define __6502_INSTR_BRANCH_H__

INLINE void BRA( int8_t reladdr ) {
    uint8_t pg = m6502.PC >> 8;
    m6502.PC += reladdr;
    m6502.clkfrm += m6502.PC >> 8 == pg ? 1 : 2;
    
#ifdef DEBUG
    if ( reladdr == -2 ) {
        dbgPrintf2("Infinite Loop at %04X!\n", m6502.PC);
    }
#endif
    dbgPrintf("BRA %04X ", m6502.PC);
}

/**
 BCC  Branch on Carry Clear
 
 branch on C = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BCC oper      90    2     2**
 **/
INLINE void BCC( int8_t reladdr ) {
    dbgPrintf("BCC ");
    disPrintf(disassembly.inst, "BCC");
    
#ifndef DEBUGGER
    if ( ! m6502.C ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BCS  Branch on Carry Set
 
 branch on C = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BCS oper      B0    2     2**
 **/
INLINE void BCS( int8_t reladdr ) {
    dbgPrintf("BCS ");
    disPrintf(disassembly.inst, "BCS");

#ifndef DEBUGGER
    if ( m6502.C ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BNE  Branch on Result not Zero
 
 branch on Z = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BNE oper      D0    2     2**
 **/
INLINE void BNE( int8_t reladdr ) {
    dbgPrintf("BNE ");
    disPrintf(disassembly.inst, "BNE");

#ifndef DEBUGGER
    if ( ! m6502.Z ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BEQ  Branch on Result Zero
 
 branch on Z = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BEQ oper      F0    2     2**
 **/
INLINE void BEQ( int8_t reladdr ) {
    dbgPrintf("BEQ ");
    disPrintf(disassembly.inst, "BEQ");

#ifndef DEBUGGER
    if ( m6502.Z ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BPL  Branch on Result Plus
 
 branch on N = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BPL oper      10    2     2**
 **/
INLINE void BPL( int8_t reladdr ) {
    dbgPrintf("BPL ");
    disPrintf(disassembly.inst, "BPL");

#ifndef DEBUGGER
    if ( ! m6502.N ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BMI  Branch on Result Minus
 
 branch on N = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BMI oper      30    2     2**
 **/
INLINE void BMI( int8_t reladdr ) {
    dbgPrintf("BMI ");
    disPrintf(disassembly.inst, "BMI");

#ifndef DEBUGGER
    if ( m6502.N ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BVC  Branch on Overflow Clear
 
 branch on V = 0                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BVC oper      50    2     2**
 **/
INLINE void BVC( int8_t reladdr ) {
    dbgPrintf("BVC ");
    disPrintf(disassembly.inst, "BVC");

#ifndef DEBUGGER
    if ( ! m6502.V ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BVS  Branch on Overflow Set
 
 branch on V = 1                  N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 relative      BVC oper      70    2     2**
 **/
INLINE void BVS( int8_t reladdr ) {
    dbgPrintf("BVS ");
    disPrintf(disassembly.inst, "BVS");

#ifndef DEBUGGER
    if ( m6502.V ) {
        BRA( reladdr );
    }
    else {
        dbgPrintf("-no-");
    }
#endif
}

/**
 BBR BBS - Branch on Bit Reset or Set
 
 BBR and BBS test the specified zero page location and branch if the specified bit is clear (BBR) or set (BBS).
 Note that as with TRB, the term reset in BBR is used to mean clear.
 
 On the 6502 and 65C02, bit 7 is typically the most convenient bit to use for I/O and software flags because
 it can be tested by several instructions, such as BIT and LDA. BBR and BBS can test any of the 8 bits without
 affecting any flags or using any registers. Unlike other branch instructions, BBR and BBS always take the same
 number of cycles (five) whether the branch is taken or not. It is often useful to test bit 0, for example, to test
 whether a byte is even or odd. However, the usefulness of BBR and BBS is somewhat limited for a couple of reasons.
 First, there is only a single addressing mode for these instructions -- no indexing by X or Y, for instance.
 Second, they are restricted to zero page locations. For software flags this may be just fine, but it may not be very
 convenient (or cost effective) to add any additional address decoding hardware that may be necessary to
 map I/O locations to the zero page.
 
 The addressing mode is a combination of zero page addressing and relative addressing -- really just a juxtaposition of the two.
 The bit to test is typically specified as part of the instruction name rather than the operand, i.e.
 
 Flags affected: none
 
 OP LEN CYC MODE   FLAGS    SYNTAX
 -- --- --- ----   -----    ------
 0F 3   5   zp,rel ........ BBR0 $12,LABEL
 1F 3   5   zp,rel ........ BBR1 $12,LABEL
 2F 3   5   zp,rel ........ BBR2 $12,LABEL
 3F 3   5   zp,rel ........ BBR3 $12,LABEL
 4F 3   5   zp,rel ........ BBR4 $12,LABEL
 5F 3   5   zp,rel ........ BBR5 $12,LABEL
 6F 3   5   zp,rel ........ BBR6 $12,LABEL
 7F 3   5   zp,rel ........ BBR7 $12,LABEL
 8F 3   5   zp,rel ........ BBS0 $12,LABEL
 9F 3   5   zp,rel ........ BBS1 $12,LABEL
 AF 3   5   zp,rel ........ BBS2 $12,LABEL
 BF 3   5   zp,rel ........ BBS3 $12,LABEL
 CF 3   5   zp,rel ........ BBS4 $12,LABEL
 DF 3   5   zp,rel ........ BBS5 $12,LABEL
 EF 3   5   zp,rel ........ BBS6 $12,LABEL
 FF 3   5   zp,rel ........ BBS7 $12,LABEL
 
**/
#ifndef DEBUGGER
#define BBR(n) INLINE void BBR##n( uint8_t src, int8_t reladdr ) { \
dbgPrintf("BBR"#n" "); \
disPrintf(disassembly.inst, "BBR"#n); \
    if ( ! (src & (1 << n) ) ) { \
        BRA( reladdr ); \
    } \
}
#else
#define BBR(n) INLINE void BBR##n( uint8_t src, int8_t reladdr ) { \
    dbgPrintf("BBR"#n" "); \
    disPrintf(disassembly.inst, "BBR"#n); \
}
#endif

    BBR(0)
    BBR(1)
    BBR(2)
    BBR(3)
    BBR(4)
    BBR(5)
    BBR(6)
    BBR(7)


#ifndef DEBUGGER
#define BBS(n) INLINE void BBS##n( uint8_t src, int8_t reladdr ) { \
dbgPrintf("BBS"#n" "); \
disPrintf(disassembly.inst, "BBS"#n); \
    if ( (src & (1 << n) ) ) { \
        BRA( reladdr ); \
    } \
}
#else
#define BBS(n) INLINE void BBS##n( uint8_t src, int8_t reladdr ) { \
    dbgPrintf("BBS"#n" "); \
    disPrintf(disassembly.inst, "BBS"#n); \
}
#endif

    BBS(0)
    BBS(1)
    BBS(2)
    BBS(3)
    BBS(4)
    BBS(5)
    BBS(6)
    BBS(7)


#endif // __6502_INSTR_BRANCH_H__
