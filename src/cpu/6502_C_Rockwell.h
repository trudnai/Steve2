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

#ifndef _6502_C_Rockwell_h
#define _6502_C_Rockwell_h

// Additional Rockwell 65C02 and WDC 65C02 instructions
// Four additional instructions and one additional addressing mode are available on 65C02s manufactured by Rockwell and WDC

    // BBR BBS - Branch on Bit Reset or Set
        case 0x0F: BBR0( src_zp(), rel_addr() ); return 5;              // BBR0 zpg, rel
        case 0x1F: BBR1( src_zp(), rel_addr() ); return 5;              // BBR1 zpg, rel
        case 0x2F: BBR2( src_zp(), rel_addr() ); return 5;              // BBR2 zpg, rel
        case 0x3F: BBR3( src_zp(), rel_addr() ); return 5;              // BBR3 zpg, rel
        case 0x4F: BBR4( src_zp(), rel_addr() ); return 5;              // BBR4 zpg, rel
        case 0x5F: BBR5( src_zp(), rel_addr() ); return 5;              // BBR5 zpg, rel
        case 0x6F: BBR6( src_zp(), rel_addr() ); return 5;              // BBR6 zpg, rel
        case 0x7F: BBR7( src_zp(), rel_addr() ); return 5;              // BBR7 zpg, rel

        case 0x8F: BBS0( src_zp(), rel_addr() ); return 5;              // BBS0 zpg, rel
        case 0x9F: BBS1( src_zp(), rel_addr() ); return 5;              // BBS1 zpg, rel
        case 0xAF: BBS2( src_zp(), rel_addr() ); return 5;              // BBS2 zpg, rel
        case 0xBF: BBS3( src_zp(), rel_addr() ); return 5;              // BBS3 zpg, rel
        case 0xCF: BBS4( src_zp(), rel_addr() ); return 5;              // BBS4 zpg, rel
        case 0xDF: BBS5( src_zp(), rel_addr() ); return 5;              // BBS5 zpg, rel
        case 0xEF: BBS6( src_zp(), rel_addr() ); return 5;              // BBS6 zpg, rel
        case 0xFF: BBS7( src_zp(), rel_addr() ); return 5;              // BBS7 zpg, rel


    // RMB SMB - Reset or Set Memory Bit
        case 0x07: RMB0( addr_zp() ); return 5;                          // RMB0 zpg, rel
        case 0x17: RMB1( addr_zp() ); return 5;                          // RMB1 zpg, rel
        case 0x27: RMB2( addr_zp() ); return 5;                          // RMB2 zpg, rel
        case 0x37: RMB3( addr_zp() ); return 5;                          // RMB3 zpg, rel
        case 0x47: RMB4( addr_zp() ); return 5;                          // RMB4 zpg, rel
        case 0x57: RMB5( addr_zp() ); return 5;                          // RMB5 zpg, rel
        case 0x67: RMB6( addr_zp() ); return 5;                          // RMB6 zpg, rel
        case 0x77: RMB7( addr_zp() ); return 5;                          // RMB7 zpg, rel

        case 0x87: SMB0( addr_zp() ); return 5;                          // SMB0 zpg, rel
        case 0x97: SMB1( addr_zp() ); return 5;                          // SMB1 zpg, rel
        case 0xA7: SMB2( addr_zp() ); return 5;                          // SMB2 zpg, rel
        case 0xB7: SMB3( addr_zp() ); return 5;                          // SMB3 zpg, rel
        case 0xC7: SMB4( addr_zp() ); return 5;                          // SMB4 zpg, rel
        case 0xD7: SMB5( addr_zp() ); return 5;                          // SMB5 zpg, rel
        case 0xE7: SMB6( addr_zp() ); return 5;                          // SMB6 zpg, rel
        case 0xF7: SMB7( addr_zp() ); return 5;                          // SMB7 zpg, rel



#endif /* _6502_C_Rockwell_h */
