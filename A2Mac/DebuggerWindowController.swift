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


    @IBAction func Continue(_ sender: Any) {
        ContinuePauseButtonState()

        switch cpuState {
        case cpuState_halted:
            ViewController.current?.Pause(sender)

        case cpuState_running:
            ViewController.current?.Pause(sender)

        default:
            break
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
