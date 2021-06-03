//
//  AppDelegate.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 7/25/19.
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

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    var preferencesController : PreferencesWindowController?

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }
    
    @IBAction func ROM_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
//            rom_loadFile( Bundle.main.resourcePath, menuIdentifier.rawValue + ".rom" )
            ViewController.romFileName = menuIdentifier.rawValue + ".rom"
//            print("Resource Path: " + Bundle.main.resourcePath!)
            m6502_ColdReset( Bundle.main.resourcePath! + "/rom/", ViewController.romFileName )
        }
    }
    
    @IBAction func Disk1_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
            let woz_err = woz_loadFile( Bundle.main.resourcePath! + "/dsk/" + menuIdentifier.rawValue + ".woz" )
            ViewController.current?.chk_woz_load(err: woz_err)
            woz_flags.image_file_readonly = 1
        }
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }
    
    @IBAction func selectAnImageFromFile(sender: AnyObject) {
        ViewController.current?.openDiskImageDialog()
    }

    
    func application(_ sender: NSApplication, openFile filename: String) -> Bool {
        ViewController.current?.openDiskImage(url: URL(fileURLWithPath: filename))
        return true;
    }
    
    @IBAction func saveFile(_ sender: NSMenuItem) {
        ViewController.current?.saveFile()
    }
    
    @IBAction func saveFileAs(_ sender: NSMenuItem) {
        ViewController.current?.saveFileAs()
    }
    
    @IBAction func Cheat_Wavy_Navy_Add_3_Ships(_ sender: Any) {
        ViewController.current?.Cheat_Wavy_Navy_Add_3_Ships()
    }
    
    @IBAction func Cheat_Wavy_Navy_Never_Lose(_ menuItem: NSMenuItem) {
        menuItem.state = (ViewController.current?.Cheat_Wavy_Navy_Never_Lose())!
    }
    
    @IBAction func Cheat_Wavy_Navy_Lose_To_Win(_ menuItem: NSMenuItem) {
        menuItem.state = (ViewController.current?.Cheat_Wavy_Navy_Lose_To_Win())!
    }
    
    @IBAction func showPreferences(_ sender: NSMenuItem) {
        
        if ( preferencesController == nil ) {
            let storyboard = NSStoryboard(name: NSStoryboard.Name("Preferences"), bundle: nil)
            preferencesController = storyboard.instantiateInitialController() as? PreferencesWindowController
        }
        
        if ( preferencesController != nil ) {
            preferencesController?.showWindow(sender)
        }
    }
    
    @IBAction func PowerOn(_ sender: Any) {
        ViewController.current?.PowerOn(sender)
    }
    
    @IBAction func Pause(_ sender: Any) {
        ViewController.current?.Pause(sender)
    }
    
    @IBAction func PowerOff(_ sender: Any) {
        ViewController.current?.PowerOff(sender)
    }
    
    @IBAction func ColdReset(_ sender: Any) {
        ViewController.current?.Reset(sender)
    }
    
    @IBAction func WarmReset(_ sender: Any) {
        ViewController.current?.Reset(sender)
    }
    
    
    
}

