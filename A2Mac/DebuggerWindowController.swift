//
//  ToolBarController.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 6/30/20.
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


class DebuggerToolBarController: NSWindowController, NSWindowDelegate {
    
    static var current : DebuggerToolBarController? = nil
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        DebuggerToolBarController.current = self
    }


    override func windowDidLoad() {
        let isWindowFullscreen = window?.styleMask.contains(.fullSizeContentView) ?? false
        if isWindowFullscreen {
            window?.toggleFullScreen(self)
        }
    }


    @IBAction func Continue(_ sender: Any) {
        switch cpuState {
        case cpuState_inited, cpuState_unknown:
            ViewController.current?.PowerOn(sender)

        default:
            ViewController.current?.PowerOff(sender)
        }
    }


    @IBAction func Step_Over(_ sender: Any) {
    }


    @IBAction func Step_In(_ sender: Any) {
        m6502_Step()
        // TODO: This should be in Debugger!
        if let debugger = DebuggerViewController.shared {
            debugger.Update()
        }
    }


    @IBAction func Step_Out(_ sender: Any) {
    }


    @IBAction func SetBreakPoint(_ sender: Any) {
    }


    @IBAction func DisableBreakPoint(_ sender: Any) {
    }


}
