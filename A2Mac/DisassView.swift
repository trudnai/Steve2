//
//  DisassView.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 5/11/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

import Cocoa

class DisassView: NSTextView {

    override var acceptsFirstResponder: Bool {
        get {
            return true
        }
    }

    var trackingArea: NSTrackingArea?

    func debugDisplayContraints() {
        print( "debugDisplayContraints ----------------------------------" )
        if let view = window?.contentView {
            for constraint in view.constraints {
                print( "debugDisplayContraints:", constraint.identifier ?? "{Constaint has no identifier}", constraint.constant )
            }
        }
    }

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
        //        print(#function + "DisassView")
        DebuggerViewController.shared?.mouseDown(with: event)

        // debugDisplayContraints()
    }

    override func mouseUp(with event: NSEvent) {
        //        print(#function + "DisassView")
        DebuggerViewController.shared?.mouseUp(with: event)
    }

    override func rightMouseDown(with event: NSEvent) {
        //        print(#function + "DisassView")
        DebuggerViewController.shared?.rightMouseDown(with: event)
    }

    override func rightMouseUp(with event: NSEvent) {
        //        print(#function + "DisassView")
        DebuggerViewController.shared?.rightMouseUp(with: event)
    }

    override func otherMouseDown(with event: NSEvent) {
        //        print(#function + "DisassView")
        DebuggerViewController.shared?.otherMouseDown(with: event)
    }

    override func otherMouseUp(with event: NSEvent) {
        //        print(#function + "DisassView")
        DebuggerViewController.shared?.otherMouseUp(with: event)
    }

}

