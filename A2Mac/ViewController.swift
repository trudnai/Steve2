//
//  ViewController.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 7/25/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

import Cocoa

class ViewController: NSViewController {

    @IBOutlet weak var display: NSTextFieldCell!
    
    func check() {
        let textBufferPointer = UnsafeRawBufferPointer(start: &RAM+0x400, count: 0x400)
//        let string = String(bytes: ram, encoding: .utf8)
//        print(string)
        
        for (index, byte) in textBufferPointer.enumerated() {
            print("byte \(index): \(byte)")
        }
    }
    
    
//    func FromBuf(ptr: UnsafeMutablePointer<UInt8>, length len: Int) -> String? {
//        // convert the bytes using the UTF8 encoding
//        if let theString = NSString(bytes: ptr, length: len, encoding: NSUTF8StringEncoding) {
//            return theString as String
//        } else {
//            return nil // the bytes aren't valid UTF8
//        }
//    }

    override func viewDidLoad() {
        super.viewDidLoad()
        display.stringValue = "lll"

        // emulate in he background
//        DispatchQueue.global(qos: .background).async {
        DispatchQueue.global(qos: .userInitiated).async {
            tst6502()
        }
        
//        let r = RAM.1

//        let r = UnsafeMutablePointer<UInt8>(RAM)

//        let r = withUnsafeBytes(of: &RAM) { (rawPtr) -> Array<uint8> in
//            let ptr = rawPtr.baseAddress!.assumingMemoryBound(to: uint8.self)
//            return Array(ptr)
//        }

        // This seem to work:
//        var ram: [UInt8] = []
//        withUnsafeBytes(of: &RAM) {
//            ram.append(contentsOf: $0)
//        }
//        let string = String(bytes: ram, encoding: .utf8)
        
        // ...but this does not:
//        let text = ""
//        while(true) {
//            for i in 0x400...0x7FF {
//                text  += screenBuffer[i]
//            }
//        }


//        DispatchQueue.main.asyncAfter(deadline: .now() + 5.0, execute: {
//            self.check()
//        })
        
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }

}

