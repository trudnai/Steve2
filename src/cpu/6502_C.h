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

#ifndef _6502_C_h
#define _6502_C_h

// http://6502.org/tutorials/interrupts.html
//
// 2.3 WAI: FASTER INTERRUPT SERVICE ON THE 65C02
//
// WDC's 65C02 has a "wait" instruction, WAI. This allows a special case of ultra-fast IRQ interrupt service.
//
// In the earlier doorbell comparison, the guests, after ringing the doorbell, had to wait for you to put down what you were doing and get to the door, which introduced a small delay before they would see the door open. The comparison now is that you have already put your work down and gone to the door so you're right there ready to open it immediately upon hearing the bell.
//
// If the main part of the work can be paused until the next interrupt, you can set the interrupt-disable bit I-- yes, set it-- execute WAI, and have the very next instruction to be the beginning of the ISR. There will be no jumping through vectors; and since you know exactly where the program pointer will be when the interrupt hits, you will not necessarily need to save any registers your ISR uses. If you do, you can do it before the interrupt.
//
// The WAI guarantees that the processor will not be in the middle of executing another instruction when the IRQ line is pulled down, so we can eliminate that part of the latency. The other part of the latency, the 7-clock interrupt sequence, gets eliminated by the fact that we have used SEI to disable the normal IRQ operation, so the IRQ will only have the effect of re-starting the processor and making it continue on with the next instruction instead of taking the vector. And since we don't take the vector, we won't use RTI at the end either.
//
// Here's the idea. The LDA and STA instructions were selected only arbitrarily for the example. The xxx just represent the continuation of code execution after the interrupt service is finished.
//
// STA VIA1IER     ; Pre-interrupt code finishes here.
// SEI             ; Disable interrupts if not already done.
// WAI             ; Put processor into pause mode.
// LDA VIA1PB      ; First instruction of ISR goes here.
//       .         ; (Notice the code is straight-lined.)
//       .         ; Service the interrupt.
//       .
// xxx             ; End of ISR moves right into the next thing
// xxx             ; for the computer to do, without using RTI.


// 6502 instructions with additional addressing modes

    // ADC AND CMP EOR LDA ORA SBC STA - (zp) addressing mode
        case 0x12: ORA( src_ind() ); return 5;                          // ORA (zpg)
        case 0x32: AND( src_ind() ); return 5;                          // AND (zpg)
        case 0x52: EOR( src_ind() ); return 5;                          // EOR (zpg)
        case 0x72: ADC( src_ind() ); return 5;                          // ADC (zpg)
        case 0x92: STA( src_ind() ); return 5;                          // STA (zpg)
        case 0xB2: LDA( src_ind() ); return 5;                          // LDA (zpg)
        case 0xD2: CMP( src_ind() ); return 5;                          // CMP (zpg)
        case 0xF2: SBC( src_ind() ); return 5;                          // SBC (zpg)

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
        case 0x80: BRA( rel_addr() ); return 3;                         // BRA reladdr

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
