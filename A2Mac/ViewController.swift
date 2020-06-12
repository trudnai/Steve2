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
    @IBOutlet weak var lores: LoRes!
    @IBOutlet weak var hires: HiRes!
    @IBOutlet weak var splashScreen: NSView!
    
    var Keyboard2Joystick = true
    var Mouse2Joystick = false
    var MouseInterface = true

    
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

    static let textLineOfs : [Int] = [
        0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x028, 0x0A8, 0x128, 0x1A8,
        0x228, 0x2A8, 0x328, 0x3A8, 0x050, 0x0D0, 0x150, 0x1D0, 0x250, 0x2D0, 0x350, 0x3D0
    ]
    
    
    func dialogOK(title: String, text: String) {
        let alert = NSAlert()
        alert.messageText = title
        alert.informativeText = text
        alert.alertStyle = .warning
        alert.addButton(withTitle: "OK")
        alert.runModal()
        // return alert.runModal() == .alertFirstButtonReturn
    }
    
    
    @objc func chk_woz_load(err : Int32) {
        switch (err) {
        case WOZ_ERR_OK:
            break
            
        case WOZ_ERR_FILE_NOT_FOUND:
            dialogOK(title: "Error Loading Disk Image", text: "File Not Found!")
            break
            
        case WOZ_ERR_NOT_WOZ_FILE:
            dialogOK(title: "Error Loading Disk Image", text: "File is not a WOZ image!")
            break
            
        case WOZ_ERR_BAD_CHUNK_HDR, WOZ_ERR_BAD_DATA:
            dialogOK(title: "Error Loading Disk Image", text: "Malformed WOZ image!")
            break
            
        default:
            dialogOK(title: "Error Loading Disk Image", text: "Unknown Error! (\(err))" )
            break
        }
    }
    
    
    var workItem : DispatchWorkItem? = nil;
    @IBAction func Power(_ sender: Any) {
        
        upd.suspend()
        halted = true

        //------------------------------------------------------------
        // Animated Splash Screen fade out and (Text) Monitor fade in
        
        hires.isHidden = true
        displayField.alphaValue = 0
        displayField.isHidden = false
        splashScreen.alphaValue = 1
        splashScreen.isHidden = false

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            NSAnimationContext.runAnimationGroup({ (context) in
                context.duration = 0.5
                // Use the value you want to animate to (NOT the starting value)
                self.displayField.animator().alphaValue = 1
                self.splashScreen.animator().alphaValue = 0
            },
            completionHandler:{ () -> Void in
                self.displayField.alphaValue = 1
                self.splashScreen.isHidden = true
            })
            
            m6502_ColdReset( Bundle.main.resourcePath, ViewController.romFileName )
            
            self.halted = false
            self.upd.resume()
        }
        //------------------------------------------------------------
        
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
    
    
    static let textPage1Addr = 0x400
    static let textPage2Addr = 0x800
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
    static let textPage1Pointer = UnsafeRawBufferPointer(start: MEM + textPage1Addr, count: textBufferSize)
    static let textPage2Pointer = UnsafeRawBufferPointer(start: MEM + textPage2Addr, count: textBufferSize)
    static let textIntBufferPointer = UnsafeRawBufferPointer(start: RAM + textPage1Addr, count: textBufferSize)
    static let textAuxBufferPointer = UnsafeRawBufferPointer(start: AUX + textPage1Addr, count: textBufferSize)

    // TODO: Render text screen in native C
//    static let textScreen = UnsafeMutableRawPointer(mutating: testText)

    
    var textBufferPointer = textPage1Pointer

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
//        print("keyDown")
        
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
                    super.keyDown(with: event)
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
//                print("LEFT", ddd);
                if ( Keyboard2Joystick ) {
                    // Keyboard 2 JoyStick (Game Controller / Paddle)
                    pdl_valarr[0] = 0
                }
                kbdInput(0x08)
                
                
            case rightArrowKey:
//                print("RIGHT")
                // Keyboard 2 JoyStick (Game Controller / Paddle)
                if ( Keyboard2Joystick ) {
                    pdl_valarr[0] = 1
                }
                kbdInput(0x15)

            case downArrowKey:
//                print("DOWN")
                // Keyboard 2 JoyStick (Game Controller / Paddle)
                if ( Keyboard2Joystick ) {
                    pdl_valarr[1] = 1
                }
                else {
                    kbdInput(0x0B)
                }
            case upArrowKey:
//                print("UP")
                // Keyboard 2 JoyStick (Game Controller / Paddle)
                if ( Keyboard2Joystick ) {
                    pdl_valarr[1] = 0
                }
                else {
                    kbdInput(0x0A)
                }

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
    
    
    var savedVideoMode = videoMode_t.init()
    
    
    override func keyUp(with event: NSEvent) {
//        print("keyUp")
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
//            setIO(0xC064, 127);
//            print("left")
            // Keyboard 2 JoyStick (Game Controller / Paddle)
            pdl_valarr[0] = 0.5
            
        case rightArrowKey:
//            kbdInput(0x15)
//            setIO(0xC064, 128);
//            print("right")
            // Keyboard 2 JoyStick (Game Controller / Paddle)
            pdl_valarr[0] = 0.5
            
        case downArrowKey:
//            kbdInput(0x0B)
//            setIO(0xC065, 127);
//            print("down")
            // Keyboard 2 JoyStick (Game Controller / Paddle)
            pdl_valarr[1] = 0.5
            
        case upArrowKey:
//            kbdInput(0x0A)
//            setIO(0xC065, 128);
//            print("up")
            // Keyboard 2 JoyStick (Game Controller / Paddle)
            pdl_valarr[1] = 0.5
            
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
    
    override func flagsChanged(with event: NSEvent) {
        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
        case [.shift]:
            setIO(0xC061, 0);
            setIO(0xC062, 0);
            setIO(0xC063, 0); // inverted (bit 7: not pressed)
//            print("shift key is pressed")
//        case [.control]:
//            print("control key is pressed")
        case [.option] :
            setIO(0xC061, 1 << 7);
            setIO(0xC062, 0);
            setIO(0xC063, 1 << 7); // inverted (bit 7: not pressed)
//            print("option key is pressed")
        case [.command]:
//            print("Command key is pressed")
            setIO(0xC061, 0);
            setIO(0xC062, 1 << 7);
            setIO(0xC063, 1 << 7); // inverted (bit 7: not pressed)
//        case [.control, .shift]:
//            print("control-shift keys are pressed")
        case [.option, .shift]:
            setIO(0xC061, 1 << 7);
            setIO(0xC062, 0);
            setIO(0xC063, 0); // inverted (bit 7: not pressed)
//            print("option-shift keys are pressed")
        case [.command, .shift]:
            setIO(0xC061, 0);
            setIO(0xC062, 1 << 7);
            setIO(0xC063, 0); // inverted (bit 7: not pressed)
//            print("command-shift keys are pressed")
//        case [.control, .option]:
//            print("control-option keys are pressed")
//        case [.control, .command]:
//            print("control-command keys are pressed")
        case [.option, .command]:
            setIO(0xC061, 1 << 7);
            setIO(0xC062, 1 << 7);
            setIO(0xC063, 1 << 7); // inverted (bit 7: not pressed)
//            print("option-command keys are pressed")
//        case [.shift, .control, .option]:
//            print("shift-control-option keys are pressed")
//        case [.shift, .control, .command]:
//            print("shift-control-command keys are pressed")
//        case [.control, .option, .command]:
//            print("control-option-command keys are pressed")
        case [.shift, .command, .option]:
            setIO(0xC061, 1 << 7);
            setIO(0xC062, 1 << 7);
            setIO(0xC063, 0); // inverted (bit 7: not pressed)
//            print("shift-command-option keys are pressed")
//        case [.shift, .control, .option, .command]:
//            print("shift-control-option-command keys are pressed")
//        case [.function]:
//            print("function key is pressed")
//        case [.capsLock]:
//            print("capsLock key is pressed")
        default:
            setIO(0xC061, 0);
            setIO(0xC062, 0);
            setIO(0xC063, 1 << 7); // inverted (bit 7: not pressed)
//            print("no modifier keys are pressed")
        }
    }


    
    
    var was = 0;
    
    var currentVideoMode = videoMode
    var lastFrameTime = CACurrentMediaTime() as Double
    var frameCounter : UInt32 = 0
    var clkCounter : Double = 0
    let fpsHalf = fps / 2
    
    var halted = true;
    
    var mouseLocation = NSPoint.zero
    
    var shadowTxt : String = ""
    
    
    func Render() {
        
        frameCnt += 1
        
        if ( frameCnt == fpsHalf ) {
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
        DispatchQueue.main.sync {
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
            let charDisposition = videoMode.col80 == 0 ? 1 : 2
            for y in 0 ..< fromLines {
                self.txtArr[ y * (self.textCols * charDisposition + self.lineEndChars) + self.textCols * charDisposition] = "\n"
            }
            
            
            // 40 col
            if videoMode.col80 == 0 {
                if MEMcfg.txt_page_2 == 0 {
                    self.textBufferPointer = ViewController.textPage1Pointer
                }
                else {
                    self.textBufferPointer = ViewController.textPage2Pointer
                }
                // render the rest of the text screen
                for y in fromLines ..< toLines {
                    for x in 0 ..< self.textCols {
                        let byte = self.textBufferPointer[ ViewController.textLineOfs[y] + x ]
                        let idx = Int(byte);
                        let chr = ViewController.charConvTbl[idx]
                        
                        self.txtArr[ y * (self.textCols + self.lineEndChars) + x ] = chr
                    }
                    
                    self.txtArr[ y * (self.textCols + self.lineEndChars) + self.textCols ] = "\n"
                }
            }
                // 80 col
            else {
                let auxPage = ( MEMcfg.is_80STORE == 1 ) && ( MEMcfg.txt_page_2 == 1 )
                
                let textIntBuffer = auxPage ?  ViewController.textIntBufferPointer : ViewController.textPage1Pointer
                let textAuxBuffer = auxPage ?  ViewController.textPage1Pointer : ViewController.textAuxBufferPointer
                
                // render the rest of the text screen
                for y in fromLines ..< toLines {
                    for x in 0 ..< self.textCols {
                        let byte = textIntBuffer[ ViewController.textLineOfs[y] + x ]
                        let idx = Int(byte);
                        let chr = ViewController.charConvTbl[idx]
                        
                        self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + x * 2 + 1] = chr
                        
                        let byte2 = textAuxBuffer[ ViewController.textLineOfs[y] + x ]
                        let idx2 = Int(byte2);
                        let chr2 = ViewController.charConvTbl[idx2]
                        
                        self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + x * 2] = chr2
                    }
                    
                    self.txtArr[ y * (self.textCols * 2 + self.lineEndChars) + self.textCols * 2] = "\n"
                }
            }
            
            
            txt = String(self.txtArr)
            
            // TODO: Render text Screen in native C
            //            txt = String(bytesNoCopy: ViewController.textScreen!, length: 10, encoding: .ascii, freeWhenDone: false) ?? "HMM"
            
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
            
            if ( self.shadowTxt != txt ) {
                self.shadowTxt = txt
                self.display.stringValue = txt
            }
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
            
            // only refresh graphics view when needed (aka not in text mode)
            if ( videoMode.text == 0 ) {
                if ( videoMode.hires == 0 ) {
                    // when we change video mode, buffer needs to be cleared to avoid artifacts
                    if ( self.savedVideoMode.text == 1 )
                        || ( self.savedVideoMode.mixed != videoMode.mixed )
                        || ( self.savedVideoMode.hires != videoMode.hires )
                    {
                        self.lores.clearScreen()
                        self.lores.isHidden = false
                        self.hires.isHidden = true
                    }
                    
                    self.lores.Update()
                }
                else {
                    // when we change video mode, buffer needs to be cleared to avoid artifacts
                    if ( self.savedVideoMode.text == 1 )
                        || ( self.savedVideoMode.mixed != videoMode.mixed )
                        || ( self.savedVideoMode.hires != videoMode.hires )
                    {
                        self.hires.clearScreen()
                        self.hires.isHidden = false
                        self.lores.isHidden = true
                    }
                    
                    self.hires.Update()
                }
            }
            else if ( self.savedVideoMode.text == 0 ) {
                // we just switched from grahics to text
                self.lores.isHidden = true
                self.hires.isHidden = true
            }
            
            self.savedVideoMode = videoMode
            
            #endif
            
            // stream speaker from a separate thread from the simulation
            // TODO: Do we need to do this from here?
            //            spkr_update()
            
            // Mouse 2 JoyStick (Game Controller / Paddle)
            mouseLocation = view.window!.mouseLocationOutsideOfEventStream
            
            if ( Mouse2Joystick ) {
                pdl_prevarr[0] = pdl_valarr[0]
                pdl_valarr[0] = Double(mouseLocation.x / (displayField.frame.width) )
                pdl_diffarr[0] = pdl_valarr[0] - pdl_prevarr[0]
                
                pdl_prevarr[1] = pdl_valarr[1]
                pdl_valarr[1] = 1 - Double(mouseLocation.y / (displayField.frame.height) )
                pdl_diffarr[1] = pdl_valarr[1] - pdl_prevarr[1]
            }
            
            if ( MouseInterface ) {
                pdl_prevarr[2] = pdl_valarr[2]
                pdl_valarr[2] = Double(mouseLocation.x / (displayField.frame.width) )
                pdl_diffarr[2] = pdl_valarr[2] - pdl_prevarr[2]
                
                pdl_prevarr[3] = pdl_valarr[3]
                pdl_valarr[3] = 1 - Double(mouseLocation.y / (displayField.frame.height) )
                pdl_diffarr[3] = pdl_valarr[3] - pdl_prevarr[3]
            }
        }
    }

    
    func Update() {
        clkCounter += Double(clkfrm)
        // we start a new frame from here, so CPU is running even while rendering
        clkfrm = 0

        frameCounter += 1
        
        if ( frameCounter % fps == 0 ) {
            let currentTime = CACurrentMediaTime() as Double
            let elpasedTime = currentTime - lastFrameTime
            lastFrameTime = currentTime
            mhz = Double( clkCounter ) / (elpasedTime * M);
            clkCounter = 0
        }
        
        
//        if ( frameCounter % 5 == 0 ) {
            Render()
//        }
        
        
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

    static var current : ViewController? = nil
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        ViewController.current = self
        
        hires.clearScreen();
        
        spkr_load_sfx( Bundle.main.resourcePath! )
        
        let woz_err = woz_loadFile( Bundle.main.resourcePath! + "/Apple DOS 3.3 January 1983.woz" )
        chk_woz_load(err: woz_err)

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
            setCPUClockSpeed(freq: 1600)
        }
        else if let freq = Double( sender.title ) {
            setCPUClockSpeed(freq: freq)
        
            
            // TODO: Probably this is not the best way to deal with the problem: To make sound continous independent of FPS and Freq
            
            spkr_extra_buf = 780 / fps
            
            switch freq {
            case 2.0:
                spkr_extra_buf = UInt32( Double(spkr_extra_buf) * 2.961538461538462 ) // normally it should come up as 77, but this way it is calculated with FPS
                break
                
            case 4.0:
                spkr_extra_buf = UInt32( Double(spkr_extra_buf) * 1.346153846153846 ) // normally it should come up as 35, but this way it is calculated with FPS
                break
                
            default:
                spkr_extra_buf = 780 / fps // normally it should come up as 26, but this way it is calculated with FPS
                break
            }
                        
            SoundGap.integerValue = Int(spkr_extra_buf)
        }
        
    }
    
    
    @IBOutlet weak var SoundGap: NSTextFieldCell!
    
    @IBAction func SoundGapChanged(_ sender: NSStepper) {
        SoundGap.integerValue = sender.integerValue
        spkr_extra_buf = UInt32( sender.integerValue )
    }
    
    @IBAction func Keyboard2JoystickOnOff(_ sender: NSButton) {
        Keyboard2Joystick = sender.state == .on
    }
    
    @IBAction func Mouse2JoystickOnOff(_ sender: NSButton) {
        Mouse2Joystick = sender.state == .on
    }
    
    @IBAction func MouseOnOff(_ sender: NSButton) {
        MouseInterface = sender.state == .on
    }
    
    @IBAction func QuickDisk(_ sender: NSButton) {
        if sender.state == .on {
//            diskAccelerator_speed = Int32( 25 * M / Double(fps) )
            diskAccelerator_enabled = 1;
        }
        else {
//            diskAccelerator_speed = 0
            diskAccelerator_enabled = 0;
        }
    }

    
    @objc func openDiskImage() {
        let openPanel = NSOpenPanel()
        openPanel.title = "Disk Image"
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.allowedFileTypes = ["dsk","do","po","nib", "woz"]
        
        openPanel.begin { (result) -> Void in
            if result == NSApplication.ModalResponse.OK {
                print("file:", openPanel.url!.path)
                //Do what you will
                //If there's only one URL, surely 'openPanel.URL'
                //but otherwise a for loop works
                
                if let filePath = openPanel.url?.path {
                    let woz_err = woz_loadFile( filePath )
                    
                    if woz_err == WOZ_ERR_OK {
                        NSDocumentController.shared.noteNewRecentDocumentURL(URL(fileURLWithPath: filePath))
                    }
                    else {
                        self.chk_woz_load(err: woz_err)
                    }
                }
            }
        }
    }

    
    @IBAction func openDocument(_ sender: Any?) {
        openDiskImage()
    }
    
    
}

