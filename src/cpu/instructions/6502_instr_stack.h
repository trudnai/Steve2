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

#ifndef __6502_INSTR_STACK_H__
#define __6502_INSTR_STACK_H__

static const uint16_t stack_base_addr = 0x100;


INLINE void PUSH( uint8_t src ) {
    // DO NOT MAKE IT NICER! faster this way!
    WRLOMEM[ stack_base_addr | m6502.SP-- ] = src;
}

INLINE uint8_t POP() {
    return Apple2_64K_MEM[ stack_base_addr | ++m6502.SP ];
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
 PHX  Push index X on Stack
 
 push X                           N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHX           48    1     3
 **/
INLINE void PHX() {
    dbgPrintf("PHX %02X ", m6502.X);
    disPrintf(disassembly.inst, "PHX");
    PUSH( m6502.X );
}

/**
 PHY  Push index Y on Stack
 
 push Y                           N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHY           48    1     3
 **/
INLINE void PHY() {
    dbgPrintf("PHY %02X ", m6502.Y);
    disPrintf(disassembly.inst, "PHY");
    PUSH( m6502.Y );
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
 PLX  Pull index X from Stack
 
 pull X                           N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLX           68    1     4
 **/
INLINE void PLX() {
    m6502.X = POP();
    dbgPrintf("PLX %02X ", m6502.X);
    disPrintf(disassembly.inst, "PLX");
    set_flags_NZ( m6502.X );
}

/**
 PLY  Pull index Y from Stack
 
 pull Y                           N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLY           68    1     4
 **/
INLINE void PLY() {
    m6502.Y = POP();
    dbgPrintf("PLY %02X ", m6502.Y);
    disPrintf(disassembly.inst, "PLY");
    set_flags_NZ( m6502.Y );
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
    PUSH( getFlags().SR ); // res and B flag should be set
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
    setFlags(POP() | 0x30); // res and B flag should be set
    dbgPrintf("PLP %02X ", m6502.SR);
    disPrintf(disassembly.inst, "PLP");
}

#endif // __6502_INSTR_STACK_H__

