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


    @IBAction func Disk1_Selected(_ sender: NSMenuItem) {
        if let menuIdentifier = sender.identifier {
            woz_loadFile( Bundle.main.resourcePath, menuIdentifier.rawValue + ".woz" )
        }
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }


}

