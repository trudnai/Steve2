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
    
    @IBAction func Power(_ sender: Any) {
        DispatchQueue.global(qos: .userInitiated).async {
            tst6502()
        }
    }
    
    @IBAction func Reset(_ sender: Any) {
//        let resetPointer = UnsafeRawBufferPointer(start: &RAM + 0x3F2, count: 2)
//        let ral = UInt16(resetPointer[0])
//        let rah = UInt16(resetPointer[1])
//        let resetAddr = rah << 8 + ral
//
//        let hex = String(resetAddr, radix: 16, uppercase: true)
//        print("reset to:\(hex)\n")
//        m6502.pc = resetAddr
        
        m6502.interrupt = SOFTRESET;
    }
    
    
    override func keyDown(with event: NSEvent) {
        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
        case [.command] where event.characters == "l",
             [.command, .shift] where event.characters == "l":
            print("command-l or command-shift-l")
        default:
            break
        }
        print( "key = " + (event.charactersIgnoringModifiers ?? ""))
        print( "\ncharacter = " + (event.characters ?? ""))
        
        if let chars = event.characters {
            let char = chars[chars.startIndex]
            if let code = char.asciiValue {
                var A2code = code + 0x60
                
                if ( code == 13 ) {
                    A2code = 141
                }
                print("keycode: \(code) --> \(A2code)")
                
                let resetPointer = UnsafeMutableRawBufferPointer(start: &RAM + 0xC000, count: 1)
                resetPointer[0] = A2code
            }
        }
        
    }
    
    override func flagsChanged(with event: NSEvent) {
        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
        case [.shift]:
            print("shift key is pressed")
        case [.control]:
            print("control key is pressed")
        case [.option] :
            print("option key is pressed")
        case [.command]:
            print("Command key is pressed")
        case [.control, .shift]:
            print("control-shift keys are pressed")
        case [.option, .shift]:
            print("option-shift keys are pressed")
        case [.command, .shift]:
            print("command-shift keys are pressed")
        case [.control, .option]:
            print("control-option keys are pressed")
        case [.control, .command]:
            print("control-command keys are pressed")
        case [.option, .command]:
            print("option-command keys are pressed")
        case [.shift, .control, .option]:
            print("shift-control-option keys are pressed")
        case [.shift, .control, .command]:
            print("shift-control-command keys are pressed")
        case [.control, .option, .command]:
            print("control-option-command keys are pressed")
        case [.shift, .command, .option]:
            print("shift-command-option keys are pressed")
        case [.shift, .control, .option, .command]:
            print("shift-control-option-command keys are pressed")
        default:
            print("no modifier keys are pressed")
        }
    }

    
    func update() {
        
        while true {
            usleep(33333) // 1/30 sec

            let textBaseAddr = 0x400
            let textLines = 24
            let textCols = 40
            
            var txt : String = ""

            for y in 0...textLines-1 {
                let textAddr = textBaseAddr + textLineOfs[y]
                let textBufferPointer = UnsafeRawBufferPointer(start: &RAM + textAddr, count: textCols)

                for (index, byte) in textBufferPointer.enumerated() {
                    let idx = Int(byte);
                    let chr = ViewController.charConvTbl[idx]
        //            print("byte \(index): \(chr)")
                    txt = txt + " \(chr)"
                }
                
                txt = txt + " |\n"
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

