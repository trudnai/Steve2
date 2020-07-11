//
//  ToolBarController.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 6/30/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

import Cocoa
import AVFoundation


class ToolBarController: NSWindowController {
    
    @IBAction func PowerOn(_ sender: Any) {
        switch cpuState {
        case cpuState_inited, cpuState_unknown:
            ViewController.current?.PowerOn(sender)

        default:
            ViewController.current?.PowerOff(sender)
        }
    }
    
    @IBAction func Pause(_ sender: Any) {
        ViewController.current?.Pause(sender)
    }
    
    @IBAction func Reset(_ sender: Any) {
        ViewController.current?.Reset(sender)
    }
    
    @IBAction func Copy(_ sender: Any) {
    }
    
    @IBAction func Paste(_ sender: Any) {
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
    
    @IBAction func SpeedSelect(_ slider: NSSlider) {
        switch slider.intValue {
        case 2:
            ViewController.current?.setCPUClockSpeed(freq: 2)
            SpeedSelector.label = "2 MHz"
            
        case 3:
            ViewController.current?.setCPUClockSpeed(freq: 4)
            SpeedSelector.label = "4 MHz"
            
        case 4:
            ViewController.current?.setCPUClockSpeed(freq: 10)
            SpeedSelector.label = "10 MHz"
            
        case 5:
            ViewController.current?.setCPUClockSpeed(freq: 100)
            SpeedSelector.label = "100 MHz"
            
        case 6:
            ViewController.current?.setCPUClockSpeed(freq: 1600)
            SpeedSelector.label = "MAX Speed"
            
        default:
            ViewController.current?.setCPUClockSpeed(freq: 1.023)
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
