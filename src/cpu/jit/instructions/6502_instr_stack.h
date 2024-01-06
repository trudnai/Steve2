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


#ifndef DISASSEMBLER
INSTR void PUSH( uint8_t src ) {
    // DO NOT MAKE IT NICER! faster this way!
    WRLOMEM[ stack_base_addr | m6502.SP-- ] = src;
}

INSTR uint8_t POP(void) {
    return Apple2_64K_MEM[ stack_base_addr | ++m6502.SP ];
}


INSTR void PUSH_addr( uint16_t addr ) {
    PUSH( (uint8_t)(addr >> 8) );
    PUSH( (uint8_t)addr );
}

INSTR uint16_t POP_addr(void) {
    return  POP() + ( POP() << 8 );
}
#endif // DISASSEMBLER

/**
 PHA  Push Accumulator on Stack
 
 push A                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHA           48    1     3
 **/
INSTR void PHA(void) {
    dbgPrintf("PHA %02X ", m6502.A);
    disPrintf(disassembly.inst, "PHA");

#ifndef DISASSEMBLER
    PUSH( m6502.A );
#endif
}

/**
 PHX  Push index X on Stack
 
 push X                           N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHX           48    1     3
 **/
INSTR void PHX(void) {
    dbgPrintf("PHX %02X ", m6502.X);
    disPrintf(disassembly.inst, "PHX");

#ifndef DISASSEMBLER
    PUSH( m6502.X );
#endif
}

/**
 PHY  Push index Y on Stack
 
 push Y                           N Z C I D V
 - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHY           48    1     3
 **/
INSTR void PHY(void) {
    dbgPrintf("PHY %02X ", m6502.Y);
    disPrintf(disassembly.inst, "PHY");

#ifndef DISASSEMBLER
    PUSH( m6502.Y );
#endif
}

/**
 PLA  Pull Accumulator from Stack
 
 pull A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLA           68    1     4
 **/
INSTR void PLA(void) {
#ifndef DISASSEMBLER
    m6502.A = POP();
#endif

    dbgPrintf("PLA %02X ", m6502.A);
    disPrintf(disassembly.inst, "PLA");

#ifndef DISASSEMBLER
    set_flags_NZ( m6502.A );
#endif
}

/**
 PLX  Pull index X from Stack
 
 pull X                           N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLX           68    1     4
 **/
INSTR void PLX(void) {
#ifndef DISASSEMBLER
    m6502.X = POP();
#endif

    dbgPrintf("PLX %02X ", m6502.X);
    disPrintf(disassembly.inst, "PLX");

#ifndef DISASSEMBLER
    set_flags_NZ( m6502.X );
#endif
}

/**
 PLY  Pull index Y from Stack
 
 pull Y                           N Z C I D V
 + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLY           68    1     4
 **/
INSTR void PLY(void) {
#ifndef DISASSEMBLER
    m6502.Y = POP();
#endif

    dbgPrintf("PLY %02X ", m6502.Y);
    disPrintf(disassembly.inst, "PLY");

#ifndef DISASSEMBLER
    set_flags_NZ( m6502.Y );
#endif
}

/**
 PHP  Push Processor Status on Stack
 
 push SR                          N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PHP           08    1     3
 **/
INSTR void PHP(void) {
    dbgPrintf("PHP %02X ", m6502.SR);
    disPrintf(disassembly.inst, "PHP");

#ifndef DISASSEMBLER
    PUSH( getFlags().SR ); // res and B flag should be set
#endif
}

/**
 PLP  Pull Processor Status from Stack
 
 pull SR                          N Z C I D V
                                  from stack
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       PLP           28    1     4
 **/
INSTR void PLP(void) {
#ifndef DISASSEMBLER
    setFlags(POP() | 0x30); // res and B flag should be set
#endif

    dbgPrintf("PLP %02X ", m6502.SR);
    disPrintf(disassembly.inst, "PLP");
}

#endif // __6502_INSTR_STACK_H__

