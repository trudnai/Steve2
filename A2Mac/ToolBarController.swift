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
import AVFoundation


class ToolBarController: NSWindowController, NSWindowDelegate {
    
    func window(_ window: NSWindow, willUseFullScreenPresentationOptions proposedOptions: NSApplication.PresentationOptions = []) -> NSApplication.PresentationOptions {
        return [.autoHideToolbar, .autoHideMenuBar, .fullScreen]
    }
    
    var sideBarWidth = CGFloat()

    
    func setDisplaySize(
        leading: CGFloat,
        trailing: CGFloat,
        top: CGFloat,
        bottom: CGFloat,
        sidePanelSize: CGFloat,
        sidePanelHidden: Bool
    ) {
        if let view = window?.contentView {
            for constraint in view.constraints {
                switch constraint.identifier {
                case "Display Background Trailing":
                    constraint.constant = sidePanelSize
                case "Display Margin Leading":
                    constraint.constant = leading
                case "Display Margin Trailing":
                    constraint.constant = trailing
                case "Display Margin Top":
                    constraint.constant = top
                case "Display Margin Bottom":
                    constraint.constant = bottom
                default:
                    continue
                }
            }
            
            for subview in view.subviews {
                if subview.identifier?.rawValue == "Side Panel" {
                    subview.isHidden = sidePanelHidden
                }
            }
        }
    }
    
    func setDisplayFullscreen() {
        print("setDisplayFullscreen")
        setDisplaySize(
            leading: 0,
            trailing: 0,
            top: 0,
            bottom: 0,
            sidePanelSize: 0,
            sidePanelHidden: true
        )
    }
    
    func setDisplayWindowed() {
        print("setDisplayWindowed")
        setDisplaySize(
            leading: 11,
            trailing: -11,
            top: 16,
            bottom: -16,
            sidePanelSize: 136,
            sidePanelHidden: false
        )
    }
    
    func windowWillEnterFullScreen(_ notification: Notification) {
        setDisplayFullscreen()
    }
    
    func windowWillExitFullScreen(_ notification: Notification) {
        setDisplayWindowed()
    }
    
    override func windowDidLoad() {
        let isWindowFullscreen = window?.styleMask.contains(.fullSizeContentView) ?? false
        if isWindowFullscreen {
            window?.toggleFullScreen(self)
        }
        
        PauseToolbarItem.isEnabled = false;
        PauseButton.state = .off
        PauseToolbarItem.label = "Off"
    }
        
    @IBAction func PowerOn(_ sender: Any) {
        switch cpuState {
        case cpuState_inited, cpuState_unknown:
            PauseToolbarItem.isEnabled = true;
            PauseButton.state = .on
            PauseToolbarItem.label = "Pause"
            
            ViewController.current?.PowerOn(sender)

        default:
            PauseToolbarItem.isEnabled = false;
            PauseButton.state = .off
            PauseToolbarItem.label = "Off"
            
            ViewController.current?.PowerOff(sender)
        }
    }
    
    @IBOutlet weak var PauseToolbarItem: NSToolbarItem!
    @IBOutlet weak var PauseButton: NSButton!
    @IBAction func Pause(_ sender: Any) {
        switch cpuState {
        case cpuState_halted:
            PauseButton.state = .on
            PauseToolbarItem.label = "Pause"
            ViewController.current?.Pause(sender)

        case cpuState_running:
            PauseButton.state = .off
            PauseToolbarItem.label = "Resume"
            ViewController.current?.Pause(sender)
            
        default:
            PauseToolbarItem.isEnabled = false;
            PauseButton.state = .off
            PauseToolbarItem.label = "Off"
            break
        }
        
    }
    
    @IBAction func Reset(_ sender: Any) {
        ViewController.current?.Reset(sender)
    }
    
    @IBAction func SelectAll(_ sender: Any) {
//        ViewController.current?.displayField.currentEditor()?.selectAll(nil)
//        ViewController.current?.displayField.selectText(nil)
        ViewController.current?.textDisplay.setSelectedRange(NSRange())
    }
    
    @IBAction func Copy(_ sender: Any) {
        ViewController.current?.Copy()
    }
    
    @IBAction func Paste(_ sender: Any) {
        ViewController.current?.Paste()
    }
    
    @IBAction func Green(_ sender: Any) {
//        print("NSToolbarItem:", sender.tag)
        ViewController.current?.MonoMonitorChange(color: "Green")
    }
    
    @IBAction func Amber(_ sender: Any) {
//        print("NSToolbarItem:", sender.tag)
        ViewController.current?.MonoMonitorChange(color: "Amber")
    }
    
    @IBAction func White(_ sender: Any) {
//        print("NSToolbarItem:", sender.tag)
        ViewController.current?.MonoMonitorChange(color: "White")
    }
    
    @IBAction func Color(_ sender: Any) {
        ViewController.current?.ColorMonitorSelector(color: true)
    }
    
    @IBOutlet weak var SpeedSelector: NSToolbarItem!
    
    @IBAction func SpeedSelected(_ slider: NSSliderCell) {
        switch slider.intValue {
        case 1:
            ViewController.current?.setCPUClockSpeed(freq: 0.25)
            SpeedSelector.label = "0.25 MHz"
            
        case 2:
            ViewController.current?.setCPUClockSpeed(freq: 0.5)
            SpeedSelector.label = "0.5 MHz"
            
        case 4:
            ViewController.current?.setCPUClockSpeed(freq: 1.5)
            SpeedSelector.label = "1.5 MHz"
            
        case 5:
            ViewController.current?.setCPUClockSpeed(freq: 2)
            SpeedSelector.label = "2 MHz"
            
        case 6:
            ViewController.current?.setCPUClockSpeed(freq: 2.8)
            SpeedSelector.label = "2.8 MHz"
            
        case 7:
            ViewController.current?.setCPUClockSpeed(freq: 4)
            SpeedSelector.label = "4 MHz"
            
        case 8:
            ViewController.current?.setCPUClockSpeed(freq: 10)
            SpeedSelector.label = "10 MHz"
            
        case 9:
            ViewController.current?.setCPUClockSpeed(freq: 100)
            SpeedSelector.label = "100 MHz"
            
        case 10:
            ViewController.current?.setCPUClockSpeed(freq: 2000)
            SpeedSelector.label = "MAX Speed"
            
        default:
            ViewController.current?.setCPUClockSpeed(freq: 14.31818 / 14.0)
            SpeedSelector.label = "1.023 MHz"
        }
    }
    
    @IBAction func Mute(_ sender: Any) {
        spkr_mute()
    }
    
    @IBAction func VolDn(_ sender: Any) {
        spkr_vol_dn()
    }
    
    @IBAction func VolUp(_ sender: Any) {
        spkr_vol_up()
    }
    
    
    @IBAction func GamePort(_ sender: NSPopUpButton) {
        switch sender.selectedItem?.tag {
        case 1: // "Keyboard -> Joystick"
            if let state = ViewController.current?.Keyboard2Joystick {
                ViewController.current?.Keyboard2Joystick = !state
                sender.selectedItem?.state = state ? .off : .on
            }
            break
            
        case 2: // "Mouse -> Joystick"
            if let state = ViewController.current?.Mouse2Joystick {
                ViewController.current?.Mouse2Joystick = !state
                sender.selectedItem?.state = state ? .off : .on
            }
            break
            
        case 3: // "Mouse Enabled"
            if let state = ViewController.current?.MouseInterface {
                ViewController.current?.MouseInterface = !state
                sender.selectedItem?.state = state ? .off : .on
            }
            break
            
        default:
            break
        }
    }
    
    @IBAction func DiskOptions(_ sender: NSPopUpButton) {
        switch sender.selectedItem?.tag {
        case 1: // "Quick Disk"
            if diskAccelerator_enabled == 0 {
                diskAccelerator_enabled = 1;
            }
            else {
                diskAccelerator_enabled = 0;
            }

            sender.selectedItem?.state = diskAccelerator_enabled == 1 ? .on : .off
            break
            
        case 2: // "Disk Sound"
            if disk_sfx_enabled == 0 {
                disk_sfx_enabled = 1;
            }
            else {
                disk_sfx_enabled = 0;
            }
            
            sender.selectedItem?.state = disk_sfx_enabled == 1 ? .on : .off
            break
            
        case 3: // "Write Enabled"
            break
            
        default:
            break
        }
    }

    @IBAction func CPUMode(_ sender: NSPopUpButton) {
        switch sender.selectedItem?.tag {
        case 1: // "Normal Mode"
            ViewController.current?.setSimulationMode( mode: "Normal" )
            for i in sender.itemArray {
                i.state = .off
            }
            sender.selectedItem?.state = .on
            break
            
        case 2: // "Eco Mode"
            ViewController.current?.setSimulationMode( mode: "Eco" )
            for i in sender.itemArray {
                i.state = .off
            }
            sender.selectedItem?.state = .on
            break

        case 3: // "Write Enabled"
            ViewController.current?.setSimulationMode( mode: "Game" )
            for i in sender.itemArray {
                i.state = .off
            }
            sender.selectedItem?.state = .on
            break

        default:
            break
        }
    }
    
    @IBAction func CRTModeOnOff(_ sender: NSButton) {
        ViewController.current?.CRTMonitorOnOff(sender)
    }
    
}
