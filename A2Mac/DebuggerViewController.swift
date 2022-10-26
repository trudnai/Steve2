//
//  PreferencesViewController.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 6/4/20.
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

import Cocoa

class DebuggerViewController: NSViewController {

    @IBOutlet var CPU_Display: DisplayView!

    override func viewDidLoad() {
        super.viewDidLoad()

        self.preferredContentSize = NSMakeSize(self.view.frame.size.width, self.view.frame.size.height)
    }

    
    override func viewDidAppear() {
        super.viewDidAppear()
        
//        // Update window title with the active TableView Title
//        self.parent?.view.window?.title = self.title!

        DisplayRegisters()
    }


    func bin(n : UInt8, sp : String = "", mid : String = " ") -> String {
        var str = ""
        var n = n

        for i in 1...8 {
            if n & 0x80 == 0 {
                str += "0"
            }
            else {
                str += "1"
            }

            str += sp
            if i == 4 {
                str += mid
            }

            n <<= 1
        }

        return str
    }

    func DisplayRegisters() {

        let registers = String(format:"""
A: %02X    %3u    %@
X: %02X    %3u    %@
Y: %02X    %3u    %@

SP: %02X
PC: %04X

N V - B D I Z C
%d %d %d %d %d %d %d %d
""",

            m6502.A, m6502.A, bin(n: m6502.A),
            m6502.X, m6502.X, bin(n: m6502.X),
            m6502.Y, m6502.Y, bin(n: m6502.Y),
            m6502.SP,
            m6502.PC,
            m6502.N > 0, m6502.V > 0, m6502.res > 0, m6502.B > 0, m6502.D > 0, m6502.I > 0, m6502.Z > 0, m6502.C > 0
        )

        CPU_Display.string = registers

    }



}
