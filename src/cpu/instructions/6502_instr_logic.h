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

#ifndef __6502_INSTR_LOGIC_H__
#define __6502_INSTR_LOGIC_H__


/**
 ORA  OR Memory with Accumulator
 
 A OR M -> A                      N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     ORA #oper     09    2     2
 zeropage      ORA oper      05    2     3
 zeropage,X    ORA oper,X    15    2     4
 absolute      ORA oper      0D    3     4
 absolute,X    ORA oper,X    1D    3     4*
 absolute,Y    ORA oper,Y    19    3     4*
 (indirect,X)  ORA (oper,X)  01    2     6
 (indirect),Y  ORA (oper),Y  11    2     5*
**/
INLINE void _ORA( uint8_t src ) {
    set_flags_NZ( m6502.A |= src );
}
INLINE void ORA( uint8_t src ) {
    dbgPrintf("ORA(%02X) ", src);
    disPrintf(disassembly.inst, "ORA");
    _ORA(src);
}

/**
 AND  AND Memory with Accumulator
 
 A AND M -> A                     N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     AND #oper     29    2     2
 zeropage      AND oper      25    2     3
 zeropage,X    AND oper,X    35    2     4
 absolute      AND oper      2D    3     4
 absolute,X    AND oper,X    3D    3     4*
 absolute,Y    AND oper,Y    39    3     4*
 (indirect,X)  AND (oper,X)  21    2     6
 (indirect),Y  AND (oper),Y  31    2     5*
 **/
INLINE void _AND( uint8_t src ) {
    set_flags_NZ( m6502.A &= src );
}
INLINE void AND( uint8_t src ) {
    dbgPrintf("AND(%02X) ", src);
    disPrintf(disassembly.inst, "AND");
    _AND(src);
}

/**
 EOR  Exclusive-OR Memory with Accumulator
 
 A EOR M -> A                     N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     EOR #oper     49    2     2
 zeropage      EOR oper      45    2     3
 zeropage,X    EOR oper,X    55    2     4
 absolute      EOR oper      4D    3     4
 absolute,X    EOR oper,X    5D    3     4*
 absolute,Y    EOR oper,Y    59    3     4*
 (indirect,X)  EOR (oper,X)  41    2     6
 (indirect),Y  EOR (oper),Y  51    2     5*
**/
INLINE void EOR( uint8_t src ) {
    dbgPrintf("EOR(%02X) ", src);
    disPrintf(disassembly.inst, "EOR");
    set_flags_NZ( m6502.A ^= src );
}

#endif // __6502_INSTR_LOGIC_H__

