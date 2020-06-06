//
//  PreferencesWindowController.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 6/4/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

import Cocoa

class PreferencesWindowController: NSWindowController, NSWindowDelegate {

    override func windowDidLoad() {
        super.windowDidLoad()

        // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    }
    
    func windowShouldClose(_ sender: NSWindow) -> Bool {
        // Hide window instead of closing
        self.window?.orderOut(sender)
        return false
    }
    
}
