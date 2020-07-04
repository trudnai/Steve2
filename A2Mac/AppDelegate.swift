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

