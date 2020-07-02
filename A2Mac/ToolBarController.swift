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
        ViewController.current?.PowerOn(sender)
    }
    
    @IBAction func PowerOff(_ sender: Any) {
        ViewController.current?.PowerOff(sender)
    }
    
    @IBAction func Pause(_ sender: Any) {
        ViewController.current?.Pause(sender)
    }
    
    @IBAction func Copy(_ sender: Any) {
    }
    
    @IBAction func Paste(_ sender: Any) {
    }
    
    @IBAction func Mono(_ sender: Any) {
        ViewController.current?.ColorMonitorSelector(color: false)
    }
    
    @IBAction func Color(_ sender: Any) {
        ViewController.current?.ColorMonitorSelector(color: true)
    }
    
    @IBAction func FGcolor(_ sender: Any) {
    }
    
    @IBAction func BGcolor(_ sender: Any) {
    }
    
    @IBAction func buttonPressed(_ sender: NSButton) {
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
    
    @IBAction func CPUMode(_ sender: NSPopUpButton) {
        ViewController.current?.setSimulationMode( mode: sender.selectedItem?.title ?? "Normal" )
    }
    
}
