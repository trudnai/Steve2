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


    @IBAction func ROM_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
//            rom_loadFile( Bundle.main.resourcePath, menuIdentifier.rawValue + ".rom" )
            ViewController.romFileName = menuIdentifier.rawValue + ".rom"
            m6502_ColdReset( Bundle.main.resourcePath, ViewController.romFileName )
        }
    }
    
    @IBAction func Disk1_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
            woz_loadFile( Bundle.main.resourcePath! + "/" + menuIdentifier.rawValue + ".woz" )
        }
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

    @IBAction func selectAnImageFromFile(sender: AnyObject) {
        let openPanel = NSOpenPanel()
        openPanel.title = "Disk Image"
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.allowedFileTypes = ["dsk","do","po","nib", "woz"]
        
        openPanel.begin { (result) -> Void in
            if result == NSApplication.ModalResponse.OK {
                print("file:", openPanel.url!.path)
                //Do what you will
                //If there's only one URL, surely 'openPanel.URL'
                //but otherwise a for loop works
                woz_loadFile( openPanel.url?.path )
            }
        }
    }

}

