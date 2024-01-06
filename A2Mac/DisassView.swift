//
//  DisassView.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 5/11/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

import Cocoa

class DisassView: DisplayView {

    override func mouseDown(with event: NSEvent) {
//        print(#function + "DisassView")
        if let dbgVC = DebuggerViewController.shared {
            dbgVC.mouseDown(with: event)
            dbgVC.DisassBackground.backgroundColor = dbgVC.selectedBackground
        // debugDisplayContraints()
        }
    }

}

