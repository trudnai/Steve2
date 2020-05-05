//
//  ViewController.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 7/25/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//


import Cocoa
import AVFoundation


let K : Double = 1000.0
let M : Double = (K * K)
let G : Double = (M * K)
let T : Double = (G * K)

let KB : Double = 1024.0
let MB : Double = (KB * KB)
let GB : Double = (MB * KB)
let TB : Double = (GB * KB)


var spk_up: AVAudioPlayer?
var spk_dn: AVAudioPlayer?

@_cdecl("ViewController_spk_up_play")
func spk_up_play() {
    spk_up?.stop()
    spk_dn?.stop()
    spk_up?.play()
}

@_cdecl("ViewController_spk_dn_play")
func spk_dn_play() {
    spk_up?.stop()
    spk_dn?.stop()
    spk_dn?.play()
}


#if METAL_YES
import Metal
#endif

class ViewController: NSViewController  {

    @IBOutlet weak var displayField: NSTextField!
    @IBOutlet weak var display: NSTextFieldCell!
    @IBOutlet weak var speedometer: NSTextFieldCell!
    @IBOutlet weak var hires: HiRes!
    
    
//    static let charConvStr : String =
//        "@🄰🄱🄲🄳🄴🄵🄶🄷🄸🄹🄺🄻🄼🄽🄾🄿🅀🅁🅂🅃🅄🅅🅆🅇🅈🅉[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
//        "@🅰🅱🅲🅳🅴🅵🅶🅷🅸🅹🅺🅻🅼🅽🅾🅿🆀🆁🆂🆃🆄🆅🆆🆇🆈🆉[\\]^_⬛︎!\"#$%&'()*+,-./0123456789:;<=>?" + // FL
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"

//    static let charConvStr : String =
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_░!\"#$%&'()*+,-./0123456789:;<=>?" + // FL
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\u{E0A0}!\"#$%&'()*+,-./0123456789:;<=>?" +
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"
    
//    static let charConvStr : String =
//    "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_░!\"#$%&'()*+,-./0123456789:;<=>?" + // FL
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\u{E0A0}!\"#$%&'()*+,-./0123456789:;<=>?" +
//        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"
    
    static let charConvStrFlashOff : String =
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
            
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" + // FL
            
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
            
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u{007F}"

    static let charConvStrFlashOn : String =
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
           
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
            
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u{007F}"

    static let charConvTblFlashOn  = Array( charConvStrFlashOn  )
    static let charConvTblFlashOff = Array( charConvStrFlashOff )
    
    static var charConvTbl = charConvTblFlashOn
    
    static var romFileName = "Apple2e_Enhanced.rom";

    let textLineOfs : [Int] = [
        0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x028, 0x0A8, 0x128, 0x1A8,
        0x228, 0x2A8, 0x328, 0x3A8, 0x050, 0x0D0, 0x150, 0x1D0, 0x250, 0x2D0, 0x350, 0x3D0
    ]
    
    var workItem : DispatchWorkItem? = nil;
    @IBAction func Power(_ sender: Any) {
        
        #if SPEEDTEST
        if ( workItem != nil ) {
            workItem!.cancel();
            workItem = nil;
        }
        else {
            workItem = DispatchWorkItem {
                DispatchQueue.global(qos: .userInteractive).async {
//                DispatchQueue.global(qos: .userInitiated).async {
//                DispatchQueue.global(qos: .background).async {
                    tst6502()
                }
            }
            DispatchQueue.global().async(execute: workItem!);
        }
        #else
        upd.suspend()
        halted = true
        usleep(100000);
        
        m6502_ColdReset( Bundle.main.resourcePath, ViewController.romFileName )
                
        halted = false
        upd.resume()

        #endif
    }
    
    @IBAction func Reset(_ sender: Any) {
//        m6502.interrupt = SOFTRESET;
        
//        let saved_frm_set = clk_6502_per_frm_set;
//        clk_6502_per_frm_set = 0
//        clk_6502_per_frm_max = 0
//        // wait for 1 ms to allow the simulation to halt
//        usleep(10000);

        softReset()
     
//        clk_6502_per_frm_set = saved_frm_set
    }
    
    
    static let textBaseAddr = 0x400
    static let textBufferSize = 0x400

    // static only needed to be able to initialize things
    static let textLines = 24
    static let textCols = 40
    static let lineEndChars = 1

    // these are needed to be able to easy access to these constants from instances
    let textLines = ViewController.textLines
    let textCols = ViewController.textCols
    let lineEndChars = ViewController.lineEndChars

    var frameCnt = 0
//    let spaceChar : Character = "\u{E17F}"
//    let blockChar : Character = "\u{E07F}"
//    static let spaceChar : Character = " "
//    static let blockChar : Character = "░"
//    static var flashingSpace : Character = " "
    
    let ramBufferPointer = UnsafeRawBufferPointer(start: MEM, count: 64 * 1024)
    let textBufferPointer = UnsafeRawBufferPointer(start: MEM + textBaseAddr, count: textBufferSize * 2)
    let textAuxBufferPointer = UnsafeRawBufferPointer(start: AUX + textBaseAddr, count: textBufferSize)

    static let textArraySize = textLines * (textCols + lineEndChars) + textCols * 2

    var txtClear = [Character](repeating: " ", count: textArraySize * 2)
    var txtArr = [Character](repeating: " ", count: textArraySize * 2)

    var s = String()
    
    func HexDump() {
        var txt : String = ""
        
        for y in 0...textLines - 1 {
            txt += String(format: "%04X: ", y * 16)
            for x in 0...15 {
                let byte = ramBufferPointer[ y * 16 + x ]
                let chr = String(format: "%02X ", byte)
                txt += chr
            }
            txt += "\n"
        }
        
        DispatchQueue.main.async {
            self.display.stringValue = txt
            self.speedometer.stringValue = String(format: "%0.3lf MHz", mhz);
        }
    }
    
    
    // AppleScript Keycodes
    let leftArrowKey = 123
    let rightArrowKey = 124
    let upArrowKey = 126
    let downArrowKey = 125
    
    var ddd = 9;

    override var acceptsFirstResponder: Bool {
        return true
    }
    
    override func keyDown(with event: NSEvent) {
        print("KBD Event")
        
//        for i in 0...65536 {
//            ddd = Int(event.keyCode) + i
//        }
//        ddd = ddd * 2
        
        
//        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
//        case [.command] where event.characters == "l",
//             [.command, .shift] where event.characters == "l":
//            print("command-l or command-shift-l")
//        default:
//            break
//        }
//        print( "key = " + (event.charactersIgnoringModifiers ?? ""))
//        print( "\ncharacter = " + (event.characters ?? ""))
        
        
        if event.modifierFlags.contains(.command) { // .shift, .option, .control ...
            if let chars = event.charactersIgnoringModifiers {
                switch chars {
                case "v":
                    print("CMD + V")
                    
                    let pasteBoard = NSPasteboard.general
                    if let str = pasteBoard.string( forType: .string ) {
                        print("PASTED:", str)
                        
                        DispatchQueue.global(qos: .background).async {
                            for char in str.uppercased() {
                                if let ascii = char.asciiValue {
                                    kbdInput(ascii)
                                }
                            }
                        }
                        
                    }
                    
                default:
                    break
                }
            }
        }
        else {
            #if FUNCTIONTEST
            #else
            let keyCode = Int(event.keyCode)
            switch keyCode {
            case leftArrowKey:
                kbdInput(0x08)
                setIO(0xC064, 0);
                print("LEFT", ddd);
            case rightArrowKey:
                kbdInput(0x15)
                setIO(0xC064, 255);
                print("RIGHT")
            case downArrowKey:
                kbdInput(0x0B)
                setIO(0xC065, 255);
                print("DOWN")
            case upArrowKey:
                kbdInput(0x0A)
                setIO(0xC065, 0);
                print("UP")
            default:
    //            print("keycode: %d", keyCode)
                if let chars = event.characters {
                    let char = chars.uppercased()[chars.startIndex]
                    if let ascii = char.asciiValue {
                        kbdInput(ascii)
                    }
                }
            }
            #endif
        }
        
    }
    
    override func keyUp(with event: NSEvent) {
        print("KBD Event")
//        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
//        case [.command] where event.characters == "l",
//             [.command, .shift] where event.characters == "l":
//            print("command-l or command-shift-l")
//        default:
//            break
//        }
//        print( "key = " + (event.charactersIgnoringModifiers ?? ""))
//        print( "\ncharacter = " + (event.characters ?? ""))
        
        #if FUNCTIONTEST
        #else
        let keyCode = Int(event.keyCode)
        switch keyCode {
        case leftArrowKey:
//            kbdInput(0x08)
            setIO(0xC064, 127);
            print("left")
        case rightArrowKey:
//            kbdInput(0x15)
            setIO(0xC064, 128);
            print("right")
        case downArrowKey:
//            kbdInput(0x0B)
            setIO(0xC065, 127);
            print("down")
        case upArrowKey:
//            kbdInput(0x0A)
            setIO(0xC065, 128);
            print("up")
        default:
//            print("keycode: %d", keyCode)
//            if let chars = event.characters {
//                let char = chars.uppercased()[chars.startIndex]
//                if let ascii = char.asciiValue {
//                    kbdInput(ascii)
//                }
//            }
            break
        }
        #endif
        
        kbdUp()
        
    }
//    override func flagsChanged(with event: NSEvent) {
//        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
//        case [.shift]:
//            print("shift key is pressed")
//        case [.control]:
//            print("control key is pressed")
//        case [.option] :
//            print("option key is pressed")
//        case [.command]:
//            print("Command key is pressed")
//        case [.control, .shift]:
//            print("control-shift keys are pressed")
//        case [.option, .shift]:
//            print("option-shift keys are pressed")
//        case [.command, .shift]:
//            print("command-shift keys are pressed")
//        case [.control, .option]:
//            print("control-option keys are pressed")
//        case [.control, .command]:
//            print("control-command keys are pressed")
//        case [.option, .command]:
//            print("option-command keys are pressed")
//        case [.shift, .control, .option]:
//            print("shift-control-option keys are pressed")
//        case [.shift, .control, .command]:
//            print("shift-control-command keys are pressed")
//        case [.control, .option, .command]:
//            print("control-option-command keys are pressed")
//        case [.shift, .command, .option]:
//            print("shift-command-option keys are pressed")
//        case [.shift, .control, .option, .command]:
//            print("shift-control-option-command keys are pressed")
//        default:
//            print("no modifier keys are pressed")
//        }
//    }


    
    
    var was = 0;
    
    var currentVideoMode = videoMode
    var lastFrameTime = CACurrentMediaTime() as Double
    var frameCounter : UInt = 0
    var clkCounter : Double = 0
    
    var halted = true;
    
    func Update() {
//        clk_6502_per_frm_max = 0
        
        clkCounter += Double(clkfrm)
        // we start a new frame from here, so CPU is running even while rendering
        clkfrm = 0

        frameCounter += 1
        
        if ( frameCounter % UInt(fps) == 0 ) {
            let currentTime = CACurrentMediaTime() as Double
            let elpasedTime = currentTime - lastFrameTime
            lastFrameTime = currentTime
            mhz = Double( clkCounter ) / (elpasedTime * M);
            clkCounter = 0
        }
        
//        render()
//        hires.compute()
        
//        HexDump()
//        return
        
        frameCnt += 1
        
        if ( frameCnt == fps / 2 ) {
//            flashingSpace = blockChar
            ViewController.charConvTbl = ViewController.charConvTblFlashOn
        }
        else if ( frameCnt >= fps ) {
//            flashingSpace = spaceChar
            ViewController.charConvTbl = ViewController.charConvTblFlashOff
            frameCnt = 0
        }
        
        // Rendering is happening in the main thread, which has two implications:
        //   1. We can update UI elements
        //   2. it is independent of the simulation, de that is running in the background thread while we are busy with rendering...
        DispatchQueue.main.async {
            
            var txt : String = ""
            
            var fromLines = 0
            var toLines = self.textLines

            if videoMode.text == 0 {
                if videoMode.mixed == 1 {
                    fromLines = toLines - 4
                }
                else {
                    toLines = 0
                }
            }

            self.txtArr = self.txtClear

            // render an empty space to eiminate displaying text portion of the screen covered by graphics
            for y in 0 ..< fromLines {
                if videoMode.col80 == 0 {
                    self.txtArr[ y * (self.textCols + self.lineEndChars) + self.textCols ] = "\n"
                }
                else {
                    self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + self.textCols * 2] = "\n"
                }
            }

            // render the rest of the text screen
            for y in fromLines ..< toLines {
                for x in 0 ..< self.textCols {
                    let byte = self.textBufferPointer[ self.textLineOfs[y] + x ]
                    let idx = Int(byte);
                    let chr = ViewController.charConvTbl[idx]
                    
                    if videoMode.col80 == 0 {
                        self.txtArr[ y * (self.textCols + self.lineEndChars) + x ] = chr
                    }
                    else {
                        self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + x * 2 + 1] = chr

                        let byte = self.textAuxBufferPointer[ self.textLineOfs[y] + x ]
                        let idx = Int(byte);
                        let chr = ViewController.charConvTbl[idx]
                        
                        self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + x * 2] = chr
                    }
                }
                
                if videoMode.col80 == 0 {
                    self.txtArr[ y * (self.textCols + self.lineEndChars) + self.textCols ] = "\n"
                }
                else {
                    self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + self.textCols * 2] = "\n"
                }
            }

            txt = String(self.txtArr)

            if videoMode.col80 != self.currentVideoMode.col80 {
                self.currentVideoMode.col80 = videoMode.col80
                
                if let fontSize = self.display.font?.pointSize {
                    if videoMode.col80 == 1 {
                        self.display.font = NSFont(name: "PRNumber3", size: fontSize)
                    }
                    else {
                        self.display.font = NSFont(name: "PrintChar21", size: fontSize)
                    }
                }
            }
            
            self.display.stringValue = txt
//            self.display.stringValue = "testing\nit\nout"

            if ( (mhz < 1.5) && (mhz != floor(mhz)) ) {
                self.speedometer.stringValue = String(format: "%0.3lf MHz", mhz);
            }
            else {
                self.speedometer.stringValue = String(format: "%0.1lf MHz", mhz);
            }
//            else {
//                self.speedometer.stringValue = String(format: "%.0lf MHz", mhz);
//            }
            
            #if HIRES
//            if ( self.was < 300 ) {
//                self.was += 1
//
//                for x in stride(from: 0, to: 280, by: 7) {
//                    for y in stride(from: 0, to: 192, by: 8) {
//                        self.hires.setNeedsDisplay( CGRect(x: x, y: y, width: 7, height: 8) )
//                    }
//                }
//            }
            
//                self.HiRes.setNeedsDisplay(self.HiRes.frame)
//            self.hires.setNeedsDisplay( CGRect(x: 0, y: 0, width: self.hires.frame.width, height: self.hires.frame.height) )

            
//            self.hires.setNeedsDisplay( CGRect(x:   0, y: 100, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 100, y: 100, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 200, y: 100, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 300, y: 100, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 400, y: 100, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 800, y: 100, width: 50, height: 50) )
//
//            self.hires.setNeedsDisplay( CGRect(x:   0, y: 200, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 100, y: 200, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 200, y: 200, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 300, y: 200, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 400, y: 200, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 500, y: 200, width: 50, height: 50) )
//
//            self.hires.setNeedsDisplay( CGRect(x:   0, y: 300, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 100, y: 300, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 200, y: 300, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 300, y: 300, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 400, y: 300, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 500, y: 300, width: 50, height: 50) )
//
//            self.hires.setNeedsDisplay( CGRect(x:   0, y: 400, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 100, y: 400, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 200, y: 400, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 300, y: 400, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 400, y: 400, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 500, y: 400, width: 50, height: 50) )
//
//            self.hires.setNeedsDisplay( CGRect(x:   0, y: 500, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 100, y: 500, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 200, y: 500, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 300, y: 500, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 400, y: 500, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 500, y: 500, width: 50, height: 50) )
//
//            self.hires.setNeedsDisplay( CGRect(x:   0, y: 600, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 100, y: 600, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 200, y: 600, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 300, y: 600, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 400, y: 600, width: 50, height: 50) )
//            self.hires.setNeedsDisplay( CGRect(x: 500, y: 600, width: 50, height: 50) )

            // only refresh graphics view when needed (aka not in text mode)
            if ( videoMode.text == 0 ) {
//                self.hires.needsDisplay = true
                self.hires.Update()
            }
//            }
            #endif
        }
        
        #if SPEEDTEST
        #else
        if ( !halted ) {
            m6502_Run()
        }
        #endif
        
        
    }

    
//    func FromBuf(ptr: UnsafeMutablePointer<UInt8>, length len: Int) -> String? {
//        // convert the bytes using the UTF8 encoding
//        if let theString = NSString(bytes: ptr, length: len, encoding: NSUTF8StringEncoding) {
//            return theString as String
//        } else {
//            return nil // the bytes aren't valid UTF8
//        }
//    }
    
    
    let upd = RepeatingTimer(timeInterval: 1/Double(fps))

    
    override func viewDidLoad() {
        super.viewDidLoad()
        
//        for y in 0 ... textLines - 1 {
//            txtClear[ y * (textCols + lineEndChars) + textCols * 2 + 1 ] = "\n"
//        }

        hires.needsDisplay = true;
        
        woz_loadFile( Bundle.main.resourcePath, "Apple DOS 3.3 January 1983.woz" )

        
        let spk_up_path = Bundle.main.path(forResource: "spk_up", ofType:"wav")!
        let spk_up_url = URL(fileURLWithPath: spk_up_path)
        do {
            spk_up = try AVAudioPlayer(contentsOf: spk_up_url)
//            spk_up?.play()
        } catch {
            // couldn't load file :(
        }
        
        let spk_dn_path = Bundle.main.path(forResource: "spk_dn", ofType:"wav")!
        let spk_dn_url = URL(fileURLWithPath: spk_dn_path)
        do {
            spk_dn = try AVAudioPlayer(contentsOf: spk_dn_url)
//            spk_up?.play()
        } catch {
            // couldn't load file :(
        }



        
        //view.frame = CGRect(origin: CGPoint(), size: NSScreen.main!.visibleFrame.size)
                
//        createHiRes()
        
        self.displayField.scaleUnitSquare(to: NSSize(width: 1, height: 1))
        
//        NSEvent.removeMonitor(NSEvent.EventType.flagsChanged)
//        NSEvent.addLocalMonitorForEvents(matching: .flagsChanged) {
//            self.flagsChanged(with: $0)
//            return $0
//        }

        //        NSEvent.removeMonitor(NSEvent.EventType.keyDown)
        NSEvent.addLocalMonitorForEvents(matching: .keyDown) {
//            print("keyDown event")
            self.keyDown(with: $0)
            return nil
//            return $0
        }
        NSEvent.addLocalMonitorForEvents(matching: .keyUp) {
//            print("keyUp event")
            self.keyUp(with: $0)
            return nil
//            return $0
        }

        displayField.maximumNumberOfLines = textLines
        displayField.preferredMaxLayoutWidth = displayField.frame.width

//        DispatchQueue.main.asyncAfter(deadline: .now() + 1/fps, execute: {
//            self.update()
//        })

//        #if FUNCTIONTEST
//        #else
//        DispatchQueue.global(qos: .background).async {
//            self.update()
//        }
        
        upd.eventHandler = {
            self.Update()
        }
        upd.resume()
//        #endif
    }


    
    func setCPUClockSpeed( freq : Double ) {
        MHz_6502 = UInt64(freq * M)
        clk_6502_per_frm = MHz_6502 / UInt64(fps)
        clk_6502_per_frm_set = clk_6502_per_frm
    }

    @IBAction func speedSelected(_ sender: NSButton) {
        if ( sender.title == "MAX" ) {
            setCPUClockSpeed(freq: 1100)
        }
        else if let freq = Double( sender.title ) {
            setCPUClockSpeed(freq: freq)
        }
    }
    
    
}

