//
//  View.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 9/18/19.
//  Copyright © 2019, 2020 Tamas Rudnai. All rights reserved.
//
// This file is part of Steve ][ -- The Apple ][ Emulator.
//
// Steve ][ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Steve ][ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Steve ][.  If not, see <https://www.gnu.org/licenses/>.
//

import Cocoa

class MonitorView: NSView {
    
    @IBOutlet var textDisplay: DisplayView!
    
    var textDisplay_boundsSize = NSSize()
    var textDisplay_width_diff : CGFloat?
    var textDisplay_height_diff : CGFloat?

    @objc func frameDidChange(notification: NSNotification) {
        var textFrameSize = frame.size
        
        if textDisplay_width_diff == nil {
            textDisplay_width_diff = textFrameSize.width - textDisplay.frame.width
            textDisplay_height_diff = textFrameSize.height - textDisplay.frame.height
            textDisplay_boundsSize = textDisplay.bounds.size
        }
        
        textFrameSize.width -= textDisplay_width_diff!
        textFrameSize.height -= textDisplay_height_diff!
        textDisplay.setFrameSize(textFrameSize)
        textDisplay.setBoundsSize(textDisplay_boundsSize)
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        
        postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(self, selector: #selector(frameDidChange), name: NSView.frameDidChangeNotification, object: self)
    }
    
    override func viewDidChangeEffectiveAppearance() {
        postsFrameChangedNotifications = true
    }

//    override func performKeyEquivalent(with event: NSEvent) -> Bool {
//        return true
//    }
    
//    override func keyDown(with event: NSEvent) {
//        print("MV KBD Event")
//    //        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
//    //        case [.command] where event.characters == "l",
//    //             [.command, .shift] where event.characters == "l":
//    //            print("command-l or command-shift-l")
//    //        default:
//    //            break
//    //        }
//    //        print( "key = " + (event.charactersIgnoringModifiers ?? ""))
//    //        print( "\ncharacter = " + (event.characters ?? ""))
//        
//        #if FUNCTIONTEST
//        #else
//        let keyCode = UInt32(event.keyCode)
//        switch keyCode {
//        case eLeftArrowKey:
//            kbdInput(0x08)
//        case eRightArrowKey:
//            kbdInput(0x15)
//        case eLeftArrowKey:
//            kbdInput(0x0B)
//        case eRightArrowKey:
//            kbdInput(0x0A)
//        default:
//    //            print("keycode: %d", keyCode)
//            if let chars = event.characters {
//                let char = chars.uppercased()[chars.startIndex]
//                if let ascii = char.asciiValue {
//                    kbdInput(ascii)
//                }
//            }
//        }
//        #endif
//        
//    }
}


