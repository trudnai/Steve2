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

#ifndef __6502_INSTR_INC_DEC_H__
#define __6502_INSTR_INC_DEC_H__


/**
 INC  Increment Memory by One
 
 M + 1 -> M                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      INC oper      E6    2     5
 zeropage,X    INC oper,X    F6    2     6
 absolute      INC oper      EE    3     6
 absolute,X    INC oper,X    FE    3     7
**/
INLINE void _INC( uint16_t addr ) {
    set_flags_NZ( ++(WRLOMEM[addr]) );
}
INLINE void INC( uint16_t addr ) {
    disPrintf(disassembly.inst, "INC");
    _INC(addr);
}

/**
 INX  Increment Index X by One
 
 X + 1 -> X                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       INX           E8    1     2
**/
INLINE void INX() {
    dbgPrintf("INX %02X -> ", m6502.X);
    disPrintf(disassembly.inst, "INX");
    set_flags_NZ( ++m6502.X );
    dbgPrintf("%02X ", m6502.X);
}

/**
 INY  Increment Index Y by One
 
 Y + 1 -> Y                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       INY           C8    1     2
**/
INLINE void INY() {
    dbgPrintf("INY %02X -> ", m6502.Y);
    disPrintf(disassembly.inst, "INY");
    set_flags_NZ( ++m6502.Y );
    dbgPrintf("%02X ", m6502.Y);
}

/**
 DEC  Decrement Memory by One
 
 M - 1 -> M                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      DEC oper      C6    2     5
 zeropage,X    DEC oper,X    D6    2     6
 absolute      DEC oper      CE    3     3
 absolute,X    DEC oper,X    DE    3     7
**/
INLINE void _DEC( uint16_t addr ) {
    set_flags_NZ( --(WRLOMEM[addr]) );
}
INLINE void DEC( uint16_t addr ) {
    disPrintf(disassembly.inst, "DEC");
    _DEC(addr);
}

/**
 DEX  Decrement Index X by One
 
 X - 1 -> X                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       DEC           CA    1     2
**/
INLINE void DEX() {
    dbgPrintf("DEX %02X -> ", m6502.X);
    disPrintf(disassembly.inst, "DEX");
    set_flags_NZ( --m6502.X );
    dbgPrintf("%02X ", m6502.X);
}

/**
 DEY  Decrement Index Y by One
 
 Y - 1 -> Y                       N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 implied       DEC           88    1     2
 **/
INLINE void DEY() {
    dbgPrintf("DEY %02X -> ", m6502.Y);
    disPrintf(disassembly.inst, "DEY");
    set_flags_NZ( --m6502.Y );
    dbgPrintf("%02X ", m6502.Y);
}

#endif // __6502_INSTR_INC_DEC_H__

