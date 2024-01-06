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
    
    @IBOutlet weak var textDisplayScroller: DisplayScrollView!
    @IBOutlet var textDisplay: DisplayView!
    @IBOutlet weak var scanlinesView: NSImageView!
    
    static let overscan_h = CGFloat(25)
    static let overscan_v = CGFloat(60)
    
    static let textViewBounds = NSSize(width: 280 * 4 + overscan_h, height: 192 * 4)
    var textDisplay_width_diff : CGFloat?
    var textDisplay_height_diff : CGFloat?
    
    let monitorView_textViewBounds = NSSize(width: 1120, height: 768)
    static let textDisplay_frameSize = NSSize(width: 1120 + overscan_h, height: 768 + overscan_v)

    override func viewDidMoveToWindow() {
//        print("Added to NEW window")
    }
    
    func adjustTextDisplaySize() {
        textDisplayScroller.updateConstraints()

        var textFrameSize = frame.size
        
        if textDisplay_width_diff == nil {
            textDisplay_width_diff = textFrameSize.width - MonitorView.textDisplay_frameSize.width
            textDisplay_height_diff = textFrameSize.height - MonitorView.textDisplay_frameSize.height
        }

//        textFrameSize.width -= textDisplay_width_diff!
//        textFrameSize.height -= textDisplay_height_diff!
        if let scanlinesView = scanlinesView {
            textFrameSize.width = scanlinesView.frame.size.width
            textFrameSize.height = scanlinesView.frame.size.height
        }
//        print( String(
//            format: "MonitorView fw:%.2f fh:%.2f bw:%.2f bh:%.2f to fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            textDisplay.frame.size.width,
//            textDisplay.frame.size.height,
//            textFrameSize.width,
//            textFrameSize.height,
//            textDisplay.bounds.size.width,
//            textDisplay.bounds.size.height,
//            MonitorView.textViewBounds.width,
//            MonitorView.textViewBounds.height
//        ))
//        print( String(
//            format: "scrollView  fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            scrollView.frame.size.width,
//            scrollView.frame.size.height,
//            scrollView.bounds.size.width,
//            scrollView.bounds.size.height
//        ))
//        print( String(
//            format: "hiresView  fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            hiresView.frame.size.width,
//            hiresView.frame.size.height,
//            hiresView.bounds.size.width,
//            hiresView.bounds.size.height
//        ))
//        print( String(
//            format: "scanlinesView  fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            scanlinesView.frame.size.width,
//            scanlinesView.frame.size.height,
//            scanlinesView.bounds.size.width,
//            scanlinesView.bounds.size.height
//        ))

        // BUGFIX: I am not sure why but if I do not adjust the frame and bounds size
        //         couple of times, Cocoa miscalculates them
        if let textDisplay = textDisplay {
            textDisplay.setFrameSize(textFrameSize)
            textDisplay.setBoundsSize(MonitorView.textViewBounds)
            DispatchQueue.main.async() {
//            for _ in 0...15 {
                textDisplay.setFrameSize(textFrameSize)
    //            textDisplay.setBoundsSize(MonitorView.textViewBounds)
    //            textDisplay.setFrameSize(scanlinesView.frame.size)
                textDisplay.setBoundsSize(MonitorView.textViewBounds)
            }
        }
    }
    
    @objc func frameDidChange(notification: NSNotification) {
//        NSLog("MonitorView:frameDidChange")
        adjustTextDisplaySize()
    }
    
//    @objc func globalFrameDidChange(notification: NSNotification) {
//        NSLog("MonitorView:globalFrameDidChange")
//        adjustTextDisplaySize()
//    }
//
//    @objc func boundsDidChange(notification: NSNotification) {
//        NSLog("MonitorView:boundsDidChange")
//        adjustTextDisplaySize()
//    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        
        postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(self, selector: #selector(frameDidChange), name: NSView.frameDidChangeNotification, object: self)
//        NotificationCenter.default.addObserver(self, selector: #selector(globalFrameDidChange), name: NSView.globalFrameDidChangeNotification, object: self)
//        NotificationCenter.default.addObserver(self, selector: #selector(boundsDidChange), name: NSView.boundsDidChangeNotification, object: self)
    }
    
    override func viewDidEndLiveResize() {
//        NSLog("MonitorView:viewDidEndLiveResize")
        adjustTextDisplaySize()
        
//        print( String(
//            format: "MonitorView1 fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            textDisplay.frame.size.width,
//            textDisplay.frame.size.height,
//            textDisplay.bounds.size.width,
//            textDisplay.bounds.size.height
//        ))
//        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
//            print( String(
//                format: "MonitorView2 fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//                self.textDisplay.frame.size.width,
//                self.textDisplay.frame.size.height,
//                self.textDisplay.bounds.size.width,
//                self.textDisplay.bounds.size.height
//            ))
//        }
    }
    
    override func viewDidChangeEffectiveAppearance() {
//        NSLog("StringviewDidChangeEffectiveAppearance")
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


