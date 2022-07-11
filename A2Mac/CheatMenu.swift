//
//  NSMenu_Hard_Hat_Mack.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 7/5/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

import Cocoa
import Foundation


class CheatMenu: NSMenu {
    override func update() {
        super.update()
        print("CheatMenu update")
        
//        AppDelegate.current?.Cheat_Menu()
        
        let ships = ViewController.current?.Get_Hard_Hat_Mack()
//        AppDelegate.current?.MenuItem_Hard_Hat_Mack_Add_3_Macks.title =  String(format: "Add 3 Macks (%d)", ships!)

    }
    
    override func itemChanged(_ item: NSMenuItem) {
        super.itemChanged(item)
        print("CheatMenu itemChanged")
    }
    
}
