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


class DebuggerWindowController: NSWindowController, NSWindowDelegate {
    
    static var current : DebuggerWindowController? = nil
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        DebuggerWindowController.current = self
    }


    override func windowDidLoad() {
        let isWindowFullscreen = window?.styleMask.contains(.fullSizeContentView) ?? false
        if isWindowFullscreen {
            window?.toggleFullScreen(self)
        }
    }


    var isKey : Bool = false

    func windowDidBecomeKey(_ notification: Notification) {
//        NSLog("windowDidBecomeKey")
        isKey = true
    }

    func windowDidResignKey(_ notification: Notification) {
//        NSLog("windowDidResignKey")
        isKey = false
    }


    // TODO: Probably there is a better way to achieve this
    // fill non-break spaces to provide uniform button width
    let offLabel = "Off \u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}"
    let pauseLabel = "Pause\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}"
    let resumeLabel = "Continue"

    @IBOutlet weak var PauseToolbarItem: NSToolbarItem!
    @IBOutlet weak var PauseButton: NSButton!


    func ContinuePauseButtonState() {
        DispatchQueue.main.async {
            switch cpuState {
            case cpuState_halted:
                if let PauseButton = self.PauseButton {
                    PauseButton.state = .off
                }
                if let PauseToolbarItem = self.PauseToolbarItem {
                    PauseToolbarItem.isEnabled = true;
                    PauseToolbarItem.label = self.resumeLabel
                }

            case cpuState_running:
                if let PauseButton = self.PauseButton {
                    PauseButton.state = .on
                }
                if let PauseToolbarItem = self.PauseToolbarItem {
                    PauseToolbarItem.isEnabled = true;
                    PauseToolbarItem.label = self.pauseLabel
                }

            default:
                if let PauseToolbarItem = self.PauseToolbarItem {
                    PauseToolbarItem.isEnabled = false;
                    PauseToolbarItem.label = self.offLabel
                }
                if let PauseButton = self.PauseButton {
                    PauseButton.state = .off
                }
                break
            }
        }
    }


    func Continue() {
        ContinuePauseButtonState()
        ToolBarController.current?.PauseButtonUpdate()

        ViewController.current?.Pause(0)
    }


    func Pause() {
        ContinuePauseButtonState()
        ToolBarController.current?.PauseButtonUpdate()

        ViewController.current?.Pause(0)
    }


    @IBAction func ContinuePauseButton(_ sender: Any) {
        switch cpuState {
        case cpuState_halted:
            Continue()

        case cpuState_running:
            Pause()

        default:
            break
        }
    }


    @IBAction func Step_Over(_ sender: Any) {
        let sp = m6502.SP

        repeat {
            m6502_Step()
        } while m6502.SP < 0xFF && m6502.SP < sp

        // TODO: This should be in Debugger!
        if let debugger = DebuggerViewController.shared {
            debugger.Update()
        }

        // TODO: Update Screen and speaker etc
    }


    @IBAction func Step_In(_ sender: Any) {
        m6502_Step()
        // TODO: This should be in Debugger!
        if let debugger = DebuggerViewController.shared {
            debugger.Update()
        }

        // TODO: Update Screen and speaker etc
    }


    @IBAction func Step_Out(_ sender: Any) {
        var sp = m6502.SP

        repeat {
            let opcode = MEM[Int(m6502.PC)]

            m6502_Step()

            // If it was NOT and RTI or RTS and stack pointer is above the saved one...
            if opcode != 0x40 && opcode != 0x60 && m6502.SP > sp {
                // ... then we need to update what we are looking at to get to the true frame pointer
                sp = m6502.SP
            }

        } while m6502.SP < 0xFF && m6502.SP <= sp

        // TODO: This should be in Debugger!
        if let debugger = DebuggerViewController.shared {
            debugger.Update()
        }

        // TODO: Update Screen and speaker etc
    }


    @IBAction func SetBreakPoint(_ sender: Any) {
    }


    @IBAction func DisableBreakPoint(_ sender: Any) {
    }


}
