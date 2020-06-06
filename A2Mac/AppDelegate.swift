//
//  AppDelegate.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 7/25/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    var preferencesController : PreferencesWindowController?

    @IBAction func ROM_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
//            rom_loadFile( Bundle.main.resourcePath, menuIdentifier.rawValue + ".rom" )
            ViewController.romFileName = menuIdentifier.rawValue + ".rom"
            m6502_ColdReset( Bundle.main.resourcePath, ViewController.romFileName )
        }
    }
    
    @IBAction func Disk1_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
            let woz_err = woz_loadFile( Bundle.main.resourcePath! + "/" + menuIdentifier.rawValue + ".woz" )
            ViewController.current?.chk_woz_load(err: woz_err)
        }
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }
    
    @IBAction func selectAnImageFromFile(sender: AnyObject) {
        ViewController.current?.openDiskImage()
    }

    
    func application(_ sender: NSApplication, openFile filename: String) -> Bool {
        let woz_err = woz_loadFile( filename )

        return woz_err == 0;
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
    
}

