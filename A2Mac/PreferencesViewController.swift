//
//  PreferencesViewController.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 6/4/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

import Cocoa

class PreferencesViewController: NSViewController {

    override func viewDidLoad() {
        super.viewDidLoad()

        self.preferredContentSize = NSMakeSize(self.view.frame.size.width, self.view.frame.size.height)
    }

    
    override func viewDidAppear() {
        super.viewDidAppear()
        
//        // Update window title with the active TableView Title
//        self.parent?.view.window?.title = self.title!
    }
}
