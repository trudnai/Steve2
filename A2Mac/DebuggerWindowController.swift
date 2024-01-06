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
    
    static var shared : DebuggerWindowController? = nil
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        DebuggerWindowController.shared = self
        m6502_dbg_init()
    }


    override func windowDidLoad() {
        let isWindowFullscreen = window?.styleMask.contains(.fullSizeContentView) ?? false
        if isWindowFullscreen {
            window?.toggleFullScreen(self)
        }

        NSEvent.addLocalMonitorForEvents(matching: .keyDown) {
            return self.keyHandler(with: $0)
            // return nil // $0
        }
    }


    var isKey : Bool = false

    func windowDidBecomeKey(_ notification: Notification) {
//        NSLog("windowDidBecomeKey")
        isKey = true
        ViewController.shared?.keyEventsOff()
    }

    func windowDidResignKey(_ notification: Notification) {
//        NSLog("windowDidResignKey")
        isKey = false
        ViewController.shared?.keyEventsOn()
    }

    func windowWillClose(_ notification: Notification) {
        m6502_dbg_off()
        DebuggerWindowController.shared = nil
    }


    // TODO: Probably there is a better way to achieve this
    // fill non-break spaces to provide uniform button width
    let offLabel = "Off \u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}"
    let pauseLabel = "Pause\u{A0}\u{A0}\u{A0}\u{A0}\u{A0}"
    let resumeLabel = "Continue"

    @IBOutlet weak var PauseToolbarItem: NSToolbarItem!
    @IBOutlet weak var PauseButton: NSButton!


    func PauseButtonUpdate(needUpdateMainToolbar: Bool = true) {
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

        if needUpdateMainToolbar {
            ToolBarController.current?.PauseButtonUpdate(needUpdateDebugToolbar: false)
        }
    }


    func Continue() {
        PauseButtonUpdate()

        m6502.debugger.SP = 0xFF

        m6502.debugger.wMask = 0
        m6502.debugger.mask.hlt = 1;
        m6502.debugger.mask.brk = 1;
        m6502.debugger.mask.inv = 1;
        
        m6502.debugger.on = true

        m6502.interrupt = NO_INT

        // because of memory debugger first we need to step over one instruction
        // otherwise it would not step over that instruction
        m6502_Step_dbg()
        // now we cn resume to run
        ViewController.shared?.Resume()
    }


    func Pause() {
        PauseButtonUpdate()

        ViewController.shared?.Pause(0)

        m6502.debugger.on = false
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

    // AppleScript Keycodes
    let leftArrowKey    = 123
    let rightArrowKey   = 124
    let upArrowKey      = 126
    let downArrowKey    = 125

    let F4FunctionKey   = 118
    let F5FunctionKey   =  96
    let F6FunctionKey   =  97
    let F7FunctionKey   =  98
    let F8FunctionKey   = 100


    func keyHandler(with event: NSEvent) -> NSEvent? {
        let keyCode = Int(event.keyCode)

        print("keyDown DWC:", keyCode)

        switch keyCode {
        case leftArrowKey:
            break

        case rightArrowKey:
            break

        case downArrowKey:
            break

        case upArrowKey:
            if isKey {
                print("upArrowKeyDn in Debugger")
                break
            }

            print("upArrowKeyDn outside of Debugger")


        case F4FunctionKey:
            Continue()

        case F5FunctionKey:
            Pause()

        case F6FunctionKey:
            Step_Over(event)

        case F7FunctionKey:
            Step_In(event)

        case F8FunctionKey:
            Step_Out(event)

        default:
//            print("keycode: %d", keyCode)
            return event
        }

        return nil
    }


    let JSR_instr_code : UInt8 = 0x20

    @IBAction func Step_Over(_ sender: Any) {
        if MEM[Int(m6502.PC)] == JSR_instr_code {
            m6502.debugger.SP = m6502.SP > 1 ? m6502.SP : 0

            m6502.debugger.wMask = 0
            m6502.debugger.mask.out = 1
            m6502.debugger.mask.hlt = 1
            m6502.debugger.mask.brk = 1
            m6502.debugger.mask.inv = 1

            m6502.debugger.on = true

            m6502.interrupt = NO_INT

            // because of memory debugger first we need to step over one instruction
            // otherwise it would not step over that instruction
            m6502_Step_dbg()
            // now we cn resume to run
            ViewController.shared?.Resume()
        }
        else {
            // not a JSR call, only do a single step
            Step_In(sender)
        }
    }


    @IBAction func Step_In(_ sender: Any) {
        m6502_Step_dbg()
        
        // TODO: This should be in Debugger!
        if let debuggerViewController = DebuggerViewController.shared {
            debuggerViewController.remove_highlight(view: debuggerViewController.Disass_Display, line: debuggerViewController.highlighted_line_number)
            debuggerViewController.Update()
        }

        // TODO: Update Screen and speaker etc
        ViewController.shared?.Update()
    }


    @IBAction func Step_Out(_ sender: Any) {
        PauseButtonUpdate()

        m6502.debugger.SP = m6502.SP < 0xFE ? m6502.SP + 1 : 0xFF

        m6502.debugger.wMask = 0
        m6502.debugger.mask.out = 1
        m6502.debugger.mask.hlt = 1;
        m6502.debugger.mask.brk = 1;
        m6502.debugger.mask.inv = 1;

        m6502.debugger.on = true

        m6502.interrupt = NO_INT

        // because of memory debugger first we need to step over one instruction
        // otherwise it would not step over that instruction
        m6502_Step_dbg()
        // now we cn resume to run
        ViewController.shared?.Resume()
    }


    @IBAction func SetBreakPoint(_ sender: Any) {
    }


    @IBAction func DisableBreakPoint(_ sender: Any) {
    }


}
