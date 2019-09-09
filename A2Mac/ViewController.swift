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
    
    static let charConvStr : String =
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"
    
    static let charConvTbl = Array( charConvStr )

    let textLineOfs : [Int] = [
        0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x028, 0x0A8, 0x128, 0x1A8,
        0x228, 0x2A8, 0x328, 0x3A8, 0x050, 0x0D0, 0x150, 0x1D0, 0x250, 0x2D0, 0x350, 0x3D0
    ]
    
    func update() {
        
        while true {
            usleep(33333) // 1/30 sec

            let textBaseAddr = 0x400
            let textLines = 24
            let textCols = 40
            
            var txt : String = "|"

            for y in 0...textLines-1 {
                let textAddr = textBaseAddr + textLineOfs[y]
                let textBufferPointer = UnsafeRawBufferPointer(start: &RAM + textAddr, count: textCols)

                for (index, byte) in textBufferPointer.enumerated() {
                    let idx = Int(byte);
                    let chr = ViewController.charConvTbl[idx]
        //            print("byte \(index): \(chr)")
                    txt = txt + [chr]
                }
                
                txt = txt + "|\n|"
            }
            
            
            DispatchQueue.main.async {
                self.display.stringValue = txt;
            }
        }
        
//        DispatchQueue.main.asyncAfter(deadline: .now() + 1/30, execute: {
//            self.update()
//        })

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


//        DispatchQueue.main.asyncAfter(deadline: .now() + 1/30, execute: {
//            self.update()
//        })
        
        DispatchQueue.global(qos: .background).async {
            self.update()
        }

    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }

}

