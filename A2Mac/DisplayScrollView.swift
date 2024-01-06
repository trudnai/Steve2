//
//  DisplayScrollView.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 11/10/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

import Foundation
import Cocoa

class DisplayScrollView : NSScrollView {
    
    override func scrollWheel(with event: NSEvent) {
//        print("DisplayScrollView.scrollWheel")
        DebuggerViewController.shared?.scrollWheel(with: event)
    }
    
//    override func scrollPageUp(_ sender: Any?) {
//        if scrollEnabled {
//            super.scrollPageUp(sender)
//        }
//    }
//    override func scrollLineUp(_ sender: Any?) {
//        if scrollEnabled {
//            super.scrollLineUp(sender)
//        }
//    }
//    override func scrollPageDown(_ sender: Any?) {
//        if scrollEnabled {
//            super.scrollPageDown(sender)
//        }
//    }
//    override func scrollLineDown(_ sender: Any?) {
//        if scrollEnabled {
//            super.scrollLineDown(sender)
//        }
//    }
//    override func scrollToBeginningOfDocument(_ sender: Any?) {
//        if scrollEnabled {
//            super.scrollToBeginningOfDocument(sender)
//        }
//    }
//    override func scrollToEndOfDocument(_ sender: Any?) {
//        if scrollEnabled {
//            super.scrollToEndOfDocument(sender)
//        }
//    }
//    override func scroll(_ point: NSPoint) {
//        if scrollEnabled {
//            super.scroll(point)
//        }
//    }

}
