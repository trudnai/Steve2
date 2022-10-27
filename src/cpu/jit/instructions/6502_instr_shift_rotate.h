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

#ifndef __6502_INSTR_SHIFT_ROTATE_H__
#define __6502_INSTR_SHIFT_ROTATE_H__


/**
 ASL  Shift Left One Bit (Memory or Accumulator)
 
 C <- [76543210] <- 0             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ASL A         0A    1     2
 zeropage      ASL oper      06    2     5
 zeropage,X    ASL oper,X    16    2     6
 absolute      ASL oper      0E    3     6
 absolute,X    ASL oper,X    1E    3     7
**/
#ifndef DEBUGGER
INSTR void _ASL( uint16_t addr ) {
    m6502.C = memread(addr) & 0x80;
    set_flags_NZ( WRLOMEM[addr] <<= 1 );
}
#endif
INSTR void ASL( uint16_t addr ) {
    dbgPrintf("ASL ");
    disPrintf(disassembly.inst, "ASL");

#ifndef DEBUGGER
    _ASL(addr);
#endif
}
INSTR void ASLA(void) {
    dbgPrintf("ASL ");
    disPrintf(disassembly.inst, "ASL");

#ifndef DEBUGGER
    m6502.C = m6502.A & 0x80;
    set_flags_NZ( m6502.A <<= 1 );
#endif
}

/**
 LSR  Shift One Bit Right (Memory or Accumulator)
 
 0 -> [76543210] -> C             N Z C I D V
                                  0 + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   LSR A         4A    1     2
 zeropage      LSR oper      46    2     5
 zeropage,X    LSR oper,X    56    2     6
 absolute      LSR oper      4E    3     6
 absolute,X    LSR oper,X    5E    3     7
**/
INSTR void LSR( uint16_t addr ) {
    dbgPrintf("LSR ");
    disPrintf(disassembly.inst, "LSR");

#ifndef DEBUGGER
    m6502.C = WRLOMEM[addr] & 1;
    set_flags_NZ( WRLOMEM[addr] >>= 1 );
#endif
}
INSTR void LSRA(void) {
    dbgPrintf("LSR ");
    disPrintf(disassembly.inst, "LSR");

#ifndef DEBUGGER
    m6502.C = m6502.A & 1;
    set_flags_NZ( m6502.A >>= 1 );
#endif
}

/**
 ROL  Rotate One Bit Left (Memory or Accumulator)
 
 C <- [76543210] <- C             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ROL A         2A    1     2
 zeropage      ROL oper      26    2     5
 zeropage,X    ROL oper,X    36    2     6
 absolute      ROL oper      2E    3     6
 absolute,X    ROL oper,X    3E    3     7
**/
#ifndef DEBUGGER
INSTR void _ROL( uint16_t addr ) {
    uint8_t C = m6502.C != 0;
    m6502.C = WRLOMEM[addr] & 0x80;
    WRLOMEM[addr] <<= 1;
    set_flags_NZ( WRLOMEM[addr] |= C );
}
#endif
INSTR void ROL( uint16_t addr ) {
    dbgPrintf("ROL ");
    disPrintf(disassembly.inst, "ROL");

#ifndef DEBUGGER
    _ROL(addr);
#endif
}
INSTR void ROLA(void) {
    dbgPrintf("ROL ");
    disPrintf(disassembly.inst, "ROL");

#ifndef DEBUGGER
    uint8_t C = m6502.C != 0;
    m6502.C = m6502.A & 0x80;
    m6502.A <<= 1;
    set_flags_NZ( m6502.A |= C );
#endif
}

/**
 ROR  Rotate One Bit Right (Memory or Accumulator)
 
 C -> [76543210] -> C             N Z C I D V
                                  + + + - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 accumulator   ROR A         6A    1     2
 zeropage      ROR oper      66    2     5
 zeropage,X    ROR oper,X    76    2     6
 absolute      ROR oper      6E    3     6
 absolute,X    ROR oper,X    7E    3     7
**/
#ifndef DEBUGGER
INSTR void _ROR( uint16_t addr ) {
    uint8_t C = m6502.C != 0;
    m6502.C = WRLOMEM[addr] & 1;
    WRLOMEM[addr] >>= 1;
    set_flags_NZ( WRLOMEM[addr] |= C  << 7 );
}
#endif
INSTR void ROR( uint16_t addr ) {
    dbgPrintf("ROR ");
    disPrintf(disassembly.inst, "ROR");
    
#ifndef DEBUGGER
    _ROR(addr);
#endif
}
INSTR void RORA(void) {
    dbgPrintf("ROR ");
    disPrintf(disassembly.inst, "ROR");

#ifndef DEBUGGER
    uint8_t C = m6502.C != 0;
    m6502.C = m6502.A & 1;
    m6502.A >>= 1;
    set_flags_NZ( m6502.A |= C << 7);
#endif
}


#endif // __6502_INSTR_SHIFT_ROTATE_H__


