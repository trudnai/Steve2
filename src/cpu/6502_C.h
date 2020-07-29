//
//  6502_und.h
//  A2Mac
//
//  Created by Tamas Rudnai on 7/27/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#ifndef _6502_C_h
#define _6502_C_h

// 6502 instructions with additional addressing modes

    // ADC AND CMP EOR LDA ORA SBC STA - (zp) addressing mode
        case 0x12: ORA( src_zp_0() ); return 5;                         // ORA (zpg)
        case 0x32: AND( src_zp_0() ); return 5;                         // AND (zpg)
        case 0x52: EOR( src_zp_0() ); return 5;                         // EOR (zpg)
        case 0x72: ADC( src_zp_0() ); return 5;                         // ADC (zpg)
        case 0x92: STA( src_zp_0() ); return 5;                         // STA (zpg)
        case 0xB2: LDA( src_zp_0() ); return 5;                         // LDA (zpg)
        case 0xD2: CMP( src_zp_0() ); return 5;                         // CMP (zpg)
        case 0xF2: SBC( src_zp_0() ); return 5;                         // SBC (zpg)

    // BIT - imm abs,X zp,X addressing modes
        case 0x34: BIT( src_zp_X() ); return 4;                         // BIT zpg,X
        case 0x3C: BIT( src_abs_X() ); return 4;                        // BIT abs,X
        case 0x89: BIT( imm() ); return 2;                              // BIT imm

    // DEC INC - acc addressing mode
        case 0x1A: INA(); return 2;                                     // INA imm (INC A)
        case 0x3A: DEA(); return 2;                                     // DEA imm (DEC A)

    // JMP - (abs,X) addressing mode
        case 0x7C: JMP( addr_abs_X() ); return 6;                       // JMP abs,X

// Additional instructions

    // BRA - BRanch Always
        case 0x80: JMP( rel_addr() ); return 3;                         // BRA reladdr

    // PHX PHY PLX PLY - PusH or PulL X or Y register
        case 0x5A: PHY(); return 3;                                     // PHY
        case 0x7A: PLY(); return 4;                                     // PLY
        case 0xDA: PHX(); return 3;                                     // PHX
        case 0xFA: PLX(); return 4;                                     // PLX

    // STZ - STore Zero
        case 0x64: STZ( addr_zp() ); return 3;                          // STZ zpg
        case 0x74: STZ( addr_zp_X() ); return 4;                        // STZ zpg,X
        case 0x9C: STZ( addr_abs() ); return 4;                         // STZ abs
        case 0x9E: STZ( addr_abs_X() ); return 5;                       // STZ abs,X

    // TRB - Test and Reset Bits
        case 0x04: TSB( addr_zp() ); return 5;                          // TSB zpg
        case 0x0C: TSB( addr_abs() ); return 6;                         // TSB abs
        case 0x14: TRB( addr_zp() ); return 5;                          // TRB zpg
        case 0x1C: TRB( addr_abs() ); return 6;                         // TRB abs



#endif /* _6502_C_h */
