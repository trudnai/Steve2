//
//  DisplayView.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 5/11/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

import Cocoa

class DisplayView: NSTextView {
    
//    override var acceptsFirstResponder: Bool {
//        get {
//            return true
//        }
//    }
    
    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }
    
    override func mouseDown(with event: NSEvent) {
//        print(#function + "DisplayView")
        ViewController.current?.mouseDown(with: event)
    }
    
    override func mouseUp(with event: NSEvent) {
//        print(#function + "DisplayView")
        ViewController.current?.mouseUp(with: event)
    }
    
    override func rightMouseDown(with event: NSEvent) {
//        print(#function + "DisplayView")
        ViewController.current?.rightMouseDown(with: event)
    }
    
    override func rightMouseUp(with event: NSEvent) {
//        print(#function + "DisplayView")
        ViewController.current?.rightMouseUp(with: event)
    }
    
    override func otherMouseDown(with event: NSEvent) {
//        print(#function + "DisplayView")
        ViewController.current?.otherMouseDown(with: event)
    }
    
    override func otherMouseUp(with event: NSEvent) {
//        print(#function + "DisplayView")
        ViewController.current?.otherMouseUp(with: event)
    }

}

