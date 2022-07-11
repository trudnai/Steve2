//
//  NSMenu_Hard_Hat_Mack.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 7/5/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

import Cocoa
import Foundation


class MainMenu: NSMenu {
    
    override func submenuAction(_ sender: Any?) {
        super.submenuAction(sender)
        print("MainMenu submenuAction")
    }
    
}
