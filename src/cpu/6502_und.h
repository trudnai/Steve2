//
//  6502_und.h
//  A2Mac
//
//  Created by Tamas Rudnai on 7/27/20.
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
// Documentations:
//
// http://nesdev.com/6502_cpu.txt
// http://www.oxyron.de/html/opcodes02.html
// https://macgui.com/kb/article/46
// https://www.masswerk.at/6502/6502_instruction_set.html
//

#ifndef _6502_und_h
#define _6502_und_h

        case 0x02: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x03: SLO( addr_zp_X() ); return 8;                       // SLO* zpg,X (undocumented)
        case 0x04: NOP(); src_zp(); return 3;                          // NOP* zpg (undocumented)

        case 0x07: SLO( addr_zp() ); return 5;                         // SLO* zpg (undocumented)

        case 0x0B: ANC( imm() ); return 2;                             // ANC** imm (undocumented)
        case 0x0C: NOP(); src_abs(); return 4;                         // NOP* (undocumented)

        case 0x0F: SLO( addr_abs() ); return 6;                        // SLO* (undocumented)

        case 0x12: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x13: SLO( addr_zp_Y() ); return 8;                       // SLO* zpg,Y (undocumented)
        case 0x14: NOP(); addr_zp_X(); return 4;                       // NOP* zpg,X (undocumented)

        case 0x17: SLO( addr_zp_X() ); return 6;                       // SLO* zpg,X (undocumented)

        case 0x1A: NOP(); return 2;                                    // NOP* (undocumented)
        case 0x1B: SLO( addr_abs_Y() ); return 7;                      // SLO* abs,Y (undocumented)
        case 0x1C: NOP(); src_abs_X(); return 4;                       // NOP* (undocumented)

        case 0x1F: SLO( addr_abs_X() ); return 7;                      // SLO* abs,X (undocumented)

        case 0x22: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x23: RLA( addr_ind_X() ); return 8;                      // RLA* ind,X 8 (undocumented)

        case 0x27: RLA( addr_zp() ); return 5;                         // RLA* zpg 5 (undocumented)

        case 0x2B: ANC( imm() ); return 2;                             // ANC* imm 2 (undocumented)

        case 0x2F: RLA( addr_abs() ); return 6;                        // RLA* abs 6 (undocumented)

        case 0x32: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x33: RLA( addr_ind_Y() ); return 8;                      // RLA* izy 8 (undocumented)
        case 0x34: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)

        case 0x37: RLA( addr_zp_X() ); return 6;                       // RLA* zpx 6 (undocumented)

        case 0x3A: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0x3B: RLA( addr_abs_Y() ); return 7;                      // RLA* aby 7 (undocumented)
        case 0x3C: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)

        case 0x3F: RLA( addr_abs_X() ); return 7;                      // RLA* abx 7 (undocumented)

        case 0x42: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x43: SRE( addr_ind_X() ); return 8;                      // SRE* izx 8 (undocumented)
        case 0x44: NOP(); return 3;                                    // NOP* zp 3 (undocumented)

        case 0x47: SRE( addr_zp() ); return 5;                         // SRE* zp 5 (undocumented)

        case 0x4B: ASR( imm() ); return 2;                             // ASR* imm 2 (undocumented)

        case 0x4F: SRE( addr_abs() ); return 6;                        // SRE* abs 6 (undocumented)

        case 0x52: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x53: SRE( addr_ind_Y() ); return 8;                      // SRE* izy 8 (undocumented)
        case 0x54: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)

        case 0x57: SRE( addr_ind_X() ); return 6;                      // SRE* zpx 6 (undocumented)

        case 0x5A: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0x5B: SRE( addr_abs_Y() ); return 7;                      // SRE* aby 7 (undocumented)
        case 0x5C: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)

        case 0x5F: SRE( addr_abs_X() ); return 7;                      // SRE* abx 7 (undocumented)

        case 0x62: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x63: RRA( addr_ind_X() ); return 8;                      // RRA* izx 8 (undocumented)
        case 0x64: NOP(); return 3;                                    // NOP* zp 3 (undocumented)

        case 0x67: RRA( addr_zp() ); return 5;                         // RRA* zp 5 (undocumented)

        case 0x6B: ARC( imm() ); return 2;                             // ARR/ARC* imm 2 (undocumented)

        case 0x6F: RRA( addr_abs() ); return 6;                        // RRA* abs 6 (undocumented)

        case 0x72: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x73: RRA( addr_ind_Y() ); return 8;                      // RRA* izy 8 (undocumented)
        case 0x74: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)

        case 0x77: RRA( addr_zp_X() ); return 6;                       // RRA* zpx 6 (undocumented)

        case 0x7A: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0x7B: RRA( addr_abs_Y() ); return 7;                      // RRA* aby 7 (undocumented)
        case 0x7C: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)

        case 0x7F: RRA( addr_abs_X() ); return 7;                      // RRA* abx 7 (undocumented)
        case 0x80: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)

        case 0x82: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0x83: SAX( addr_ind_X() ); return 6;                      // SAX* izx 6 (undocumented)

        case 0x87: SAX( addr_zp() ); return 3;                         // SAX* izx 6 (undocumented)

        case 0x89: NOP(); imm(); return 2;                             // NOP* imm (undocumented)

        case 0x8B: XAA( imm() ); return 2;                             // XAA* imm 2 (undocumented, highly unstable!)

        case 0x8F: SAX( addr_abs() ); return 4;                        // SAX* abs 4 (undocumented)

        case 0x92: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0x93: SHA( addr_ind_Y() ); return 6;                      // SHA* izy 6 (undocumented, unstable)

        case 0x97: SAX( addr_zp_Y() ); return 4;                       // SAX* izy 4 (undocumented)

        case 0x9B: SAS( addr_abs_Y() ); return 5;                      // SAS* aby 5 (undocumented, unstable)
        case 0x9C: SHY( addr_abs_X() ); return 5;                      // SHY* abx 5 (undocumented, unstable)

        case 0x9E: SHX( addr_abs_Y() ); return 5;                      // SHX* aby 5 (undocumented, unstable)
        case 0x9F: SAX( addr_abs_Y() ); return 5;                      // SAX* aby 5 (undocumented, unstable)

        case 0xA3: LAX( src_X_ind() ); return 6;                       // LAX* izx 6 (undocumented)

        case 0xA7: LAX( src_zp() ); return 3;                          // LAX* zpg 3 (undocumented)

        case 0xAB: LAX( imm() ); return 2;                             // LAX* imm 2 (undocumented, highly unstable)

        case 0xAF: LAX( src_abs() ); return 4;                         // LAX* abs 4 (undocumented)

        case 0xB2: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0xB3: LAX( src_ind_Y() ); return 5;                       // LAX* izy 5 (undocumented)

        case 0xB7: LAX( src_zp_Y() ); return 4+1;                        // LAX* zpy 4 (undocumented)

        case 0xBB: LAS( src_abs_Y() ); return 4;                       // LAX* aby 4 (undocumented)

        case 0xBF: LAX( src_abs_Y() ); return 4;                       // LAX* aby 4 (undocumented)

        case 0xC2: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0xC3: DCP( addr_ind_X() ); return 8;                      // DCP* izx 8 (undocumented)
    
        case 0xC7: DCP( addr_zp() ); return 5;                         // DCP* zpg 5 (undocumented)

        case 0xCB: SBX( imm() ); return 2;                             // SBX* imm 2 (undocumented)

        case 0xCF: DCP( addr_abs() ); return 6;                        // DCP* abs 6 (undocumented)

        case 0xD2: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0xD3: DCP( addr_ind_Y() ); return 8;                      // DCP* izy 8 (undocumented)
        case 0xD4: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)

        case 0xD7: DCP( addr_zp_X() ); return 6;                       // DCP* zpx 6 (undocumented)

        case 0xDA: NOP(); return 2;                                    // NOP* 2 (undocumented)
        case 0xDB: DCP( addr_abs_Y() ); return 7;                      // DCP* aby 7 (undocumented)
        case 0xDC: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)
        
        case 0xDF: DCP( addr_abs_X() ); return 7;                      // DCP* abx 7 (undocumented)

        case 0xE2: NOP(); imm(); return 2;                             // NOP* imm 2 (undocumented)
        case 0xE3: ISB( addr_ind_X() ); return 8;                      // ISB* izx 8 (undocumented)

        case 0xE7: ISB( addr_zp() ); return 5;                         // ISB* zpg 5 (undocumented)

        case 0xEB: SBC( imm() ); return 2;                             // SBC* imm 2 (undocumented)

        case 0xEF: ISB( addr_abs() ); return 6;                        // ISB* abs 6 (undocumented)

        case 0xF2: HLT(); return 0;                                     // HLT* - Halts / Hangs / Jams / Kills the CPU (undocumented)
        case 0xF3: ISB( addr_ind_Y() ); return 8;                      // ISB* izy 8 (undocumented)
        case 0xF4: NOP(); src_zp_X(); return 4;                        // NOP* zpx 4 (undocumented)

        case 0xF7: ISB( addr_zp_X() ); return 6;                       // ISB* zpx 6 (undocumented)

        case 0xFA: NOP(); return 2;                                    // NOP (undocumented)
        case 0xFB: ISB( addr_abs_Y() ); return 7;                      // ISB* aby 7 (undocumented)
        case 0xFC: NOP(); src_abs_X(); return 4;                       // NOP* abx 4 (undocumented)

        case 0xFF: ISB( addr_abs_X() ); return 7;                      // ISB* abx 7 (undocumented)


#endif /* _6502_und_h */
