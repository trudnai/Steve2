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
    
    var trackingArea: NSTrackingArea?
    
    /// Install tracking area if window is set, remove previous one if needed.
    func installTrackingArea() {
        guard let window = window else { return }
        window.acceptsMouseMovedEvents = true
        if trackingArea != nil { removeTrackingArea(trackingArea!) }
        let trackingOptions : NSTrackingArea.Options = [.activeAlways, .mouseEnteredAndExited, .mouseMoved]
        trackingArea = NSTrackingArea(rect: bounds,
                                      options: trackingOptions,
                                      owner: self, userInfo: nil)
        self.addTrackingArea(trackingArea!)
    }
    
    
    // Called when layout is modified
    override func updateTrackingAreas() {
        super.updateTrackingAreas()
        installTrackingArea()
    }

    
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

    
//    static let overscan_h = CGFloat(25)
//    static let textViewBounds = NSSize(width: 280 * 4 + overscan_h, height: 192 * 4)
//    let textDisplay_frameSize = NSSize(width: 1120 + overscan_h, height: 768)
//    
//    var textDisplay_width_diff : CGFloat?
//    var textDisplay_height_diff : CGFloat?
//    
//    var locked = false
//
//    func adjustTextDisplaySize() {
//        var textFrameSize = frame.size
//        
//        if locked
//        || frame.size.width > 8192
//        || frame.size.width < 200
//        || frame.size.height > 8192
//        || frame.size.height < 200 {
//            return
//        }
//        
//        locked = true
//
//        if textDisplay_width_diff == nil {
//            textDisplay_width_diff = textFrameSize.width - textDisplay_frameSize.width
//            textDisplay_height_diff = textFrameSize.height - textDisplay_frameSize.height
//        }
//        
//        textFrameSize.width -= textDisplay_width_diff!
//        textFrameSize.height -= textDisplay_height_diff!
//        
////        print( String(
////            format: "DisplayView fw:%.2f fh:%.2f bw:%.2f bh:%.2f to fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
////            frame.size.width,
////            frame.size.height,
////            textFrameSize.width,
////            textFrameSize.height,
////            bounds.size.width,
////            bounds.size.height,
////            MonitorView.textViewBounds.width,
////            MonitorView.textViewBounds.height
////        ))
//        
//        // BUGFIX: I am not sure why but if I do not adjust the frame and bounds size
//        //         couple of times, Cocoa miscalculates them
////        for _ in 0...15 {
////            self.setFrameSize(textFrameSize)
//            self.setFrameSize(textDisplay_frameSize)
//            self.setBoundsSize(MonitorView.textViewBounds)
////        }
//        
//        locked = false
//    }
    
//    override func viewDidEndLiveResize() {
//        NSLog("DisplayView:viewDidEndLiveResize")
//        print( String(
//            format: "DisplayView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            frame.size.width,
//            frame.size.height,
//            bounds.size.width,
//            bounds.size.height
//        ))
//    }
//
//    @objc func frameDidChange(notification: NSNotification) {
//        NSLog("DisplayView:frameDidChange")
////        adjustTextDisplaySize()
//        print( String(
//            format: "DisplayView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            frame.size.width,
//            frame.size.height,
//            bounds.size.width,
//            bounds.size.height
//        ))
//    }
    
//    @objc func globalFrameDidChange(notification: NSNotification) {
//        NSLog("DisplayView:globalFrameDidChange")
//        print( String(
//            format: "DisplayView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            self.frame.size.width,
//            self.frame.size.height,
//            self.bounds.size.width,
//            self.bounds.size.height
//        ))
//    }
    
//    @objc func boundsDidChange(notification: NSNotification) {
//        NSLog("DisplayView:boundsDidChange")
//        print( String(
//            format: "DisplayView fw:%.2f fh:%.2f bw:%.2f bh:%.2f",
//            self.frame.size.width,
//            self.frame.size.height,
//            self.bounds.size.width,
//            self.bounds.size.height
//        ))
//    }
    
//    required init?(coder: NSCoder) {
//        super.init(coder: coder)
//        
//        postsFrameChangedNotifications = true
//        NotificationCenter.default.addObserver(self, selector: #selector(frameDidChange), name: NSView.frameDidChangeNotification, object: self)
//        NotificationCenter.default.addObserver(self, selector: #selector(globalFrameDidChange), name: NSView.globalFrameDidChangeNotification, object: self)
//        NotificationCenter.default.addObserver(self, selector: #selector(boundsDidChange), name: NSView.boundsDidChangeNotification, object: self)
//    }
}

