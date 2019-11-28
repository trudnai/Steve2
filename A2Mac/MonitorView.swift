//
//  View.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 9/18/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

import Cocoa

class MonitorView: NSView {
    override func performKeyEquivalent(with event: NSEvent) -> Bool {
        return true
    }
}
