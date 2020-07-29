//
//  6502_std.h
//  A2Mac
//
//  Created by Tamas Rudnai on 7/27/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#ifndef _6502_std_h
#define _6502_std_h

        case 0x00: BRK(); return 7;                                    // BRK
        case 0x01: ORA( src_X_ind() ); return 6;                       // ORA X,ind
        
        case 0x05: ORA( src_zp() ); return 3;                          // ORA zpg
        case 0x06: ASL( addr_zp() ); return 5;                         // ASL zpg
        
        case 0x08: PHP(); return 3;                                    // PHP
        case 0x09: ORA( imm() ); return 2;                             // ORA imm
        case 0x0A: ASLA(); return 2;                                   // ASL A
        
        case 0x0D: ORA( src_abs() ); return 4;                         // ORA abs
        case 0x0E: ASL( addr_abs() ); return 6;                        // ASL abs
        
        case 0x10: BPL( rel_addr() ); return 3;                        // BPL rel
        case 0x11: ORA( src_ind_Y() ); return 5;                       // ORA ind,Y

        case 0x15: ORA( src_zp_X() ); return 4;                        // ORA zpg,X
        case 0x16: ASL( addr_zp_X() ); return 6;                       // ASL zpg,X

        case 0x18: CLC(); return 2;                                    // CLC
        case 0x19: ORA( src_abs_Y() ); return 4+1;                       // ORA abs,Y

        case 0x1D: ORA( src_abs_X() ); return 4+1;                       // ORA abs,X
        case 0x1E: ASL( addr_abs_X() ); return 7;                      // ASL abs,X

        case 0x20: JSR( abs_addr() ); return 6;                        // JSR abs
        case 0x21: AND( src_X_ind() ); return 6;                       // AND X,ind

        case 0x24: BIT( src_zp() ); return 3;                          // BIT zpg
        case 0x25: AND( src_zp() ); return 3;                          // AND zpg
        case 0x26: ROL( addr_zp() ); return 5;                         // ROL zpg

        case 0x28: PLP(); return 4;                                    // PLP
        case 0x29: AND( imm() ); return 2;                             // AND imm
        case 0x2A: ROLA(); return 2;                                   // ROL A

        case 0x2C: BIT( src_abs() ); return 4;                         // BIT abs
        case 0x2D: AND( src_abs() ); return 4;                         // AND abs
        case 0x2E: ROL( addr_abs() ); return 6;                        // ROL abs

        case 0x30: BMI( rel_addr() ); return 3;                        // BMI rel
        case 0x31: AND( src_ind_Y() ); return 5;                       // AND ind,Y

        case 0x35: AND( src_zp_X() ); return 4;                        // AND zpg,X
        case 0x36: ROL( addr_zp_X() ); return 6;                       // ROL zpg,X

        case 0x38: SEC(); return 2;                                    // SEC
        case 0x39: AND( src_abs_Y() ); return 4+1;                     // AND abs,Y

        case 0x3D: AND( src_abs_X() ); return 4+1;                     // AND abs,X
        case 0x3E: ROL( addr_abs_X() ); return 7;                      // ROL abs,X

        case 0x40: RTI(); return 6;                                    // RTI
        case 0x41: EOR( src_X_ind() ); return 6;                       // EOR X,ind

        case 0x45: EOR( src_zp() ); return 3;                          // EOR zpg
        case 0x46: LSR( addr_zp() ); return 5;                         // LSR zpg

        case 0x48: PHA(); return 3;                                    // PHA
        case 0x49: EOR( imm() ); return 2;                             // EOR imm
        case 0x4A: LSRA(); return 2;                                   // LSR A

        case 0x4C: JMP( abs_addr() ); return 3;                        // JMP abs
        case 0x4D: EOR( src_abs() ); return 4;                         // EOR abs
        case 0x4E: LSR( addr_abs() ); return 6;                        // LSR abs

        case 0x50: BVC( rel_addr() ); return 3;                        // BVC rel
        case 0x51: EOR( src_ind_Y() ); return 5;                       // EOR ind,Y

        case 0x55: EOR( src_zp_X() ); return 4;                        // AND zpg,X
        case 0x56: LSR( addr_zp_X() ); return 6;                       // LSR zpg,X

        case 0x58: CLI(); return 2;                                    // CLI
        case 0x59: EOR( src_abs_Y() ); return 4+1;                     // EOR abs,Y

        case 0x5D: EOR( src_abs_X() ); return 4+1;                     // EOR abs,X
        case 0x5E: LSR( addr_abs_X() ); return 7;                      // LSR abs,X

        case 0x60: RTS(); return 6;                                    // RTS
        case 0x61: ADC( src_X_ind() ); return 6;                       // ADC X,ind

        case 0x65: ADC( src_zp() ); return 3;                          // ADC zpg
        case 0x66: ROR( addr_zp() ); return 5;                         // ROR zpg

        case 0x68: PLA(); return 4;                                    // PLA
        case 0x69: ADC( imm() ); return 2;                             // ADC imm
        case 0x6A: RORA(); return 2;                                   // ROR A

        case 0x6C: JMP( ind_addr() ); return    5;                        // JMP ind
        case 0x6D: ADC( src_abs() ); return 4;                         // ADC abs
        case 0x6E: ROR( addr_abs() ); return 6;                        // ROR abs

        case 0x70: BVS( rel_addr() ); return 3;                        // BVS rel
        case 0x71: ADC( src_ind_Y() ); return 5;                       // ADC ind,Y

        case 0x75: ADC( src_zp_X() ); return 4;                        // ADC zpg,X
        case 0x76: ROR( addr_zp_X() ); return 6;                       // ROR zpg,X

        case 0x78: SEI(); return 2;                                    // SEI
        case 0x79: ADC( src_abs_Y() ); return 4+1;                     // ADC abs,Y

        case 0x7D: ADC( src_abs_X() ); return 4+1;                     // ADC abs,X
        case 0x7E: ROR( addr_abs_X() ); return 7;                      // ROR abs,X

        case 0x81: STA( addr_ind_X() ) ; return 6;                     // STA X,ind

        case 0x84: STY( addr_zp() ); return 3;                         // STY zpg
        case 0x85: STA( addr_zp() ); return 3;                         // STA zpg
        case 0x86: STX( addr_zp() ); return 3;                         // STX zpg

        case 0x88: DEY(); return 2;                                    // DEY

        case 0x8A: TXA(); return 2;                                    // TXA

        case 0x8C: STY( addr_abs() ); return 4;                        // STY abs
        case 0x8D: STA( addr_abs() ); return 4;                        // STA abs
        case 0x8E: STX( addr_abs() ); return 4;                            // STX abs

        case 0x90: BCC( rel_addr() ); return 3;                        // BCC rel
        case 0x91: STA( addr_ind_Y() ); return 6;                      // STA ind,Y

        case 0x94: STY( addr_zp_X() ); return 4;                       // STY zpg,X
        case 0x95: STA( addr_zp_X() ); return 4;                       // STA zpg,X
        case 0x96: STX( addr_zp_Y() ); return 4;                       // STX zpg,Y

        case 0x98: TYA(); return 2;                                    // TYA
        case 0x99: STA( addr_abs_Y() ); return 5;                      // STA abs,Y
        case 0x9A: TXS(); return 2;                                    // TXS

        case 0x9D: STA( addr_abs_X() ); return 5;                      // STA abs,X

        case 0xA0: LDY( imm() ); return 2;                             // LDY imm
        case 0xA1: LDA( src_X_ind() ) ; return 6;                      // LDA X,ind
        case 0xA2: LDX( imm() ); return 2;                             // LDX imm

        case 0xA4: LDY( src_zp() ); return 3;                          // LDY zpg
        case 0xA5: LDA( src_zp() ); return 3;                          // LDA zpg
        case 0xA6: LDX( src_zp() ); return 3;                          // LDX zpg

        case 0xA8: TAY(); return 2;                                    // TAY
        case 0xA9: LDA( imm() ); return 2;                             // LDA imm
        case 0xAA: TAX(); return 2;                                    // TAX

        case 0xAC: LDY( src_abs() ); return 4;                         // LDY abs
        case 0xAD: LDA( src_abs() ); return 4;                         // LDA abs
        case 0xAE: LDX( src_abs() ); return 4;                         // LDX abs

        case 0xB0: BCS( rel_addr() ); return 3;                        // BCS rel
        case 0xB1: LDA( src_ind_Y() ); return 5;                       // LDA ind,Y

        case 0xB4: LDY( src_zp_X() ); return 4+1;                        // LDY zpg,X
        case 0xB5: LDA( src_zp_X() ); return 4+1;                        // LDA zpg,X
        case 0xB6: LDX( src_zp_Y() ); return 4+1;                        // LDX zpg,Y

        case 0xB8: CLV(); return 2;                                    // CLV
        case 0xB9: LDA( src_abs_Y() ); return 4;                       // LDA abs,Y
        case 0xBA: TSX(); return 2;                                    // TSX

        case 0xBC: LDY( src_abs_X() ); return 4;                       // LDY abs,X
        case 0xBD: LDA( src_abs_X() ); return 4;                       // LDA abs,X
        case 0xBE: LDX( src_abs_Y() ); return 4;
// LDX abs,Y
        case 0xC0: CPY( imm() ); return 2;                             // CPY imm
        case 0xC1: CMP( src_X_ind() ) ; return 6;                      // LDA X,ind

        case 0xC4: CPY( src_zp() ); return 3;                          // CPY zpg
        case 0xC5: CMP( src_zp() ); return 3;                          // CMP zpg
        case 0xC6: DEC( addr_zp() ); return 5;                         // DEC zpg

        case 0xC8: INY(); return 2;                                    // INY
        case 0xC9: CMP( imm() ); return 2;                             // CMP imm
        case 0xCA: DEX(); return 2;                                    // DEX

        case 0xCC: CPY( src_abs() ); return 4;                         // CPY abs
        case 0xCD: CMP( src_abs() ); return 4;                         // CMP abs
        case 0xCE: DEC( addr_abs() ); return 6;                        // DEC abs

        case 0xD0: BNE( rel_addr() ); return 3;                        // BNE rel
        case 0xD1: CMP( src_ind_Y() ); return 5;                       // CMP ind,Y

        case 0xD5: CMP( src_zp_X() ); return 4;                        // CMP zpg,X
        case 0xD6: DEC( addr_zp_X() ); return 6;                       // DEC zpg,X

        case 0xD8: CLD(); return 2;                                    // CLD
        case 0xD9: CMP( src_abs_Y() ); return 4;                       // CMP abs,Y

        case 0xDD: CMP( src_abs_X() ); return 4;                       // CMP abs,X
        case 0xDE: DEC( addr_abs_X() ); return 7;                      // DEC abs,X

        case 0xE0: CPX( imm() ); return 2;                             // CPX imm
        case 0xE1: SBC( src_X_ind() ) ; return 6;                      // SBC (X,ind)

        case 0xE4: CPX( src_zp() ); return 3;                          // CPX zpg
        case 0xE5: SBC( src_zp() ); return 3;                          // SBC zpg
        case 0xE6: INC( addr_zp() ); return 5;                         // INC zpg

        case 0xE8: INX(); return 2;                                    // INX
        case 0xE9: SBC( imm() ); return 2;                             // SBC imm
        case 0xEA: NOP(); return 2;                                    // NOP

        case 0xEC: CPX( src_abs() ); return 4;                         // CPX abs
        case 0xED: SBC( src_abs() ); return 4;                         // SBC abs
        case 0xEE: INC( addr_abs() ); return 6;                        // INC abs

        case 0xF0: BEQ( rel_addr() ); return 3;                        // BEQ rel
        case 0xF1: SBC( src_ind_Y() ); return 5;                       // SBC ind,Y

        case 0xF5: SBC( src_zp_X() ); return 4;                        // SBC zpg,X
        case 0xF6: INC( addr_zp_X() ); return 6;                       // INC zpg,X

        case 0xF8: SED(); return 2;                                    // SED
        case 0xF9: SBC( src_abs_Y() ); return 4+1;                       // SBC abs,Y

        case 0xFD: SBC( src_abs_X() ); return 4+1;                       // SBC abs,X
        case 0xFE: INC( addr_abs_X() ); return 7;                      // INC abs,X


#endif /* _6502_std_h */
