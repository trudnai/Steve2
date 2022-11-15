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
    
    var scrollEnabled = false
    
    override func scrollWheel(with event: NSEvent) {
        if scrollEnabled {
            super.scrollWheel(with: event)
        }
        else {
            let textView = documentView as! NSTextView
            var scrollTo = textView.visibleRect.origin
            let lineSpacing = CGFloat(1.5) // textView.defaultParagraphStyle?.lineSpacing ?? CGFloat(1.5)
            let lineHeight = textView.font!.pointSize * lineSpacing

            scrollTo.y = scrollTo.y + event.scrollingDeltaY * lineHeight

            textView.scroll(scrollTo)
        }
    }
    
    override func scrollPageUp(_ sender: Any?) {
        if scrollEnabled {
            super.scrollPageUp(sender)
        }
    }
    override func scrollLineUp(_ sender: Any?) {
        if scrollEnabled {
            super.scrollLineUp(sender)
        }
    }
    override func scrollPageDown(_ sender: Any?) {
        if scrollEnabled {
            super.scrollPageDown(sender)
        }
    }
    override func scrollLineDown(_ sender: Any?) {
        if scrollEnabled {
            super.scrollLineDown(sender)
        }
    }
    override func scrollToBeginningOfDocument(_ sender: Any?) {
        if scrollEnabled {
            super.scrollToBeginningOfDocument(sender)
        }
    }
    override func scrollToEndOfDocument(_ sender: Any?) {
        if scrollEnabled {
            super.scrollToEndOfDocument(sender)
        }
    }
    override func scroll(_ point: NSPoint) {
        if scrollEnabled {
            super.scroll(point)
        }
    }

//    override func viewDidEndLiveResize() {
//        NSLog("DisplayScrollView:viewDidEndLiveResize")
//        print( String(
//            format: "DisplayScrollView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            self.frame.size.width,
//            self.frame.size.height,
//            self.bounds.size.width,
//            self.bounds.size.height
//        ))
//    }
//    
//    @objc func frameDidChange(notification: NSNotification) {
//        NSLog("DisplayScrollView:frameDidChange")
//        print( String(
//            format: "DisplayScrollView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            self.frame.size.width,
//            self.frame.size.height,
//            self.bounds.size.width,
//            self.bounds.size.height
//        ))
//    }
//    
//    @objc func globalFrameDidChange(notification: NSNotification) {
//        NSLog("DisplayScrollView:globalFrameDidChange")
//        print( String(
//            format: "DisplayScrollView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            self.frame.size.width,
//            self.frame.size.height,
//            self.bounds.size.width,
//            self.bounds.size.height
//        ))
//    }
//    
//    @objc func boundsDidChange(notification: NSNotification) {
//        NSLog("DisplayScrollView:boundsDidChange")
//        print( String(
//            format: "DisplayScrollView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            self.frame.size.width,
//            self.frame.size.height,
//            self.bounds.size.width,
//            self.bounds.size.height
//        ))
//    }
//    
//    required init?(coder: NSCoder) {
//        super.init(coder: coder)
//        
//        postsFrameChangedNotifications = true
//        NotificationCenter.default.addObserver(self, selector: #selector(frameDidChange), name: NSView.frameDidChangeNotification, object: self)
//        NotificationCenter.default.addObserver(self, selector: #selector(globalFrameDidChange), name: NSView.globalFrameDidChangeNotification, object: self)
//        NotificationCenter.default.addObserver(self, selector: #selector(boundsDidChange), name: NSView.boundsDidChangeNotification, object: self)
//    }
}
