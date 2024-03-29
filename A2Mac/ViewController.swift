//
//  ViewController.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 7/25/19.
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
//import AVFoundation

//import Metal
//
//var device : MTLDevice!
//var metalLayer: CAMetalLayer!
//var vertexBuffer: MTLBuffer!
//var pipelineState: MTLRenderPipelineState!
//var commandQueue: MTLCommandQueue!
////var timer: CADisplayLink!



let K : Double = 1000.0
let M : Double = (K * K)
let G : Double = (M * K)
let T : Double = (G * K)

let KB : Double = 1024.0
let MB : Double = (KB * KB)
let GB : Double = (MB * KB)
let TB : Double = (GB * KB)


let colorWhite  =  NSColor.init( red:0.9296875, green:0.9296875, blue:0.9296875, alpha: 1 )
let colorGreen  =  NSColor.init( red:0.16796875, green:0.84375, blue:0.2890625, alpha: 1 )
let colorOrange =  NSColor.init( red:1, green:0.38671875, blue:0.0078125, alpha: 1 )

var monoColor = colorGreen;

//var spk_up: AVAudioPlayer?
//var spk_dn: AVAudioPlayer?
//
//@_cdecl("ViewController_spk_up_play")
//func spk_up_play() {
//    spk_up?.stop()
//    spk_dn?.stop()
//    spk_up?.play()
//}
//
//@_cdecl("ViewController_spk_dn_play")
//func spk_dn_play() {
//    spk_up?.stop()
//    spk_dn?.stop()
//    spk_dn?.play()
//}


#if METAL_YES
import Metal
#endif

class ViewController: NSViewController  {

    static var shared : ViewController? = nil
    
    var displayLink: CVDisplayLink?
    
    @IBOutlet var monitorView: MonitorView!
    @IBOutlet weak var textDisplayScroller: NSScrollView!
    @IBOutlet var textDisplay: NSTextView!
    @IBOutlet weak var speedometer: NSTextFieldCell!
    @IBOutlet weak var lores: LoRes!
    @IBOutlet weak var hires: HiRes!
    @IBOutlet weak var splashScreen: NSView!
    @IBOutlet weak var scanLines: NSImageView!
    
    var CRTMonitor = false
    var ColorMonitor = true
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
    
    // TODO: On 80n col mode no flash + small caps are inversed on the Flash map
    
    static let charConvStrFlashOff40 : String =
        // INVERSE
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
            
        // FLASH
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" + // FL

        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
            
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u{E27F}"

    static let charConvStrFlashOn40 : String =
        // INVERSE
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +

        // FLASH
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
           
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
            
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u{E27F}"

    
    static let charConvStrCol80 : String =
        // INVERSE
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E120}\u{E121}\u{E122}\u{E123}\u{E124}\u{E125}\u{E126}\u{E127}\u{E128}\u{E129}\u{E12A}\u{E12B}\u{E12C}\u{E12D}\u{E12E}\u{E12F}\u{E130}\u{E131}\u{E132}\u{E133}\u{E134}\u{E135}\u{E136}\u{E137}\u{E138}\u{E139}\u{E13A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +
        
        // INVERSE 2 with small caps
        "\u{E140}\u{E141}\u{E142}\u{E143}\u{E144}\u{E145}\u{E146}\u{E147}\u{E148}\u{E149}\u{E14A}\u{E14B}\u{E14C}\u{E14D}\u{E14E}\u{E14F}\u{E150}\u{E151}\u{E152}\u{E153}\u{E154}\u{E155}\u{E156}\u{E157}\u{E158}\u{E159}\u{E15A}\u{E15B}\u{E15C}\u{E15D}\u{E15E}\u{E15F}\u{E160}\u{E161}\u{E162}\u{E163}\u{E164}\u{E165}\u{E166}\u{E167}\u{E168}\u{E169}\u{E16A}\u{E16B}\u{E16C}\u{E16D}\u{E16E}\u{E16F}\u{E170}\u{E171}\u{E172}\u{E173}\u{E174}\u{E175}\u{E176}\u{E177}\u{E178}\u{E179}\u{E17A}\u{E13B}\u{E13C}\u{E13D}\u{E13E}\u{E13F}" +

        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?" +
        
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u{E27F}"
    
    
    static let charConvTblFlashOn40  = Array( charConvStrFlashOn40  )
    static let charConvTblFlashOff40 = Array( charConvStrFlashOff40 )
    static let charConvTblCol80  = Array( charConvStrCol80  )
    
    static var charConvTblFlashOn  = charConvTblFlashOn40
    static var charConvTblFlashOff = charConvTblFlashOff40
    
    static var charConvTbl = charConvTblFlashOn
    
//    static var romFileName = "Apple2e_Enhanced.rom"
    static var romFileName = "Apple2e_32k.rom"
//    static var romFileName = "077-0019 Apple IIe Diagnostic Card - English.rom"

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
    @IBAction func PowerOn(_ sender: Any) {
        
        #if SCHEDULER_CVDISPLAYLINK
        CVDisplayLinkStop(displayLink!)
        #else
        upd.suspend()
        #endif

        cpuState = cpuState_inited;
        spkr_stopAll()

        //------------------------------------------------------------
        // Animated Splash Screen fade out and (Text) Monitor fade in
        
        hires.isHidden = true
        lores.isHidden = true
//        displayField.alphaValue = 0
//        displayField.isHidden = false
        textDisplayScroller.alphaValue = 0
        textDisplayScroller.isHidden = false
        splashScreen.alphaValue = 1
        splashScreen.isHidden = false

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            NSAnimationContext.runAnimationGroup({ (context) in
                context.duration = 1.0
                // Use the value you want to animate to (NOT the starting value)
                self.textDisplayScroller.animator().alphaValue = 1
                self.hires.animator().alphaValue = 1
                self.lores.animator().alphaValue = 1
                self.splashScreen.animator().alphaValue = 0
            },
            completionHandler:{ () -> Void in
                self.textDisplayScroller.alphaValue = 1
                self.hires.alphaValue = 1
                self.lores.alphaValue = 1
                self.splashScreen.isHidden = true
            })
            
            m6502_ColdReset( Bundle.main.resourcePath! + "/rom/", ViewController.romFileName )
            
            cpuState = cpuState_running;
            #if SCHEDULER_CVDISPLAYLINK
            CVDisplayLinkStart(self.displayLink!)
            #else
            self.upd.resume()
            #endif

            if let debugger = DebuggerWindowController.shared {
                debugger.PauseButtonUpdate(needUpdateMainToolbar: false)
            }
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

    
    @IBAction func PowerOff(_ sender: Any) {
        
        #if SCHEDULER_CVDISPLAYLINK
        CVDisplayLinkStop(displayLink!)
        #else
        upd.suspend()
        #endif

        cpuState = cpuState_inited;
        spkr_stopAll()
        
        //------------------------------------------------------------
        // Animated Splash Screen fade out and (Text) Monitor fade in
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.0) {
            self.splashScreen.alphaValue = 0
            self.splashScreen.isHidden = false

            NSAnimationContext.runAnimationGroup({ (context) in
                context.duration = 0.5
                // Use the value you want to animate to (NOT the starting value)
                self.textDisplayScroller.animator().alphaValue = 0
                self.hires.animator().alphaValue = 0
                self.lores.animator().alphaValue = 0
                self.splashScreen.animator().alphaValue = 1
            },
            completionHandler:{ () -> Void in
                self.textDisplayScroller.alphaValue = 0
                self.textDisplayScroller.isHidden = true
                self.splashScreen.isHidden = false
                
                self.hires.alphaValue = 0
                self.lores.alphaValue = 0
                self.hires.isHidden = true
                self.lores.isHidden = true
                
                self.splashScreen.isHidden = false
            })
        }

        if let debugger = DebuggerWindowController.shared {
            debugger.PauseButtonUpdate(needUpdateMainToolbar: false)
        }

        
//        hires.isHidden = true
//        lores.isHidden = true
//        textDisplayScroller.alphaValue = 0
////        textDisplayScroller.isHidden = false
//        splashScreen.alphaValue = 1
//        splashScreen.isHidden = false
        
        //------------------------------------------------------------
        
    }


    func debuggerShowWindow() {
        if let debuggerWindowController = DebuggerWindowController.shared {
            DispatchQueue.main.async {
                debuggerWindowController.showWindow(self)
            }
        }
    }


    func debuggerRemoveHighlight() {
        if let debuggerViewController = DebuggerViewController.shared {
            debuggerViewController.remove_highlight(view: debuggerViewController.Disass_Display, line: debuggerViewController.highlighted_line_number)
        }
    }


    func debuggerPauseUpdate() {
        if let debuggerViewController = DebuggerViewController.shared {
            debuggerViewController.TrunDisassAddressPC(.on)
            debuggerViewController.remove_highlight(view: debuggerViewController.Disass_Display, line: debuggerViewController.highlighted_line_number)
            debuggerViewController.Update()
        }
    }


    func Resume() {
        debuggerRemoveHighlight()

        #if SCHEDULER_CVDISPLAYLINK
        CVDisplayLinkStart(displayLink!)
        #else
        upd.resume()
        #endif

        cpuState = cpuState_running

        DispatchQueue.main.async {
            self.view.window?.windowController?.showWindow(self)
        }

        if let debugger = DebuggerWindowController.shared {
            debugger.PauseButtonUpdate()
        }
    }


    func Pause() {
        #if SCHEDULER_CVDISPLAYLINK
        CVDisplayLinkStop(displayLink!)
        #else
        upd.suspend()
        #endif

        cpuState = cpuState_halted

        if let debugger = DebuggerWindowController.shared {
            debugger.PauseButtonUpdate()
        }

        debuggerPauseUpdate()
    }


    @IBAction func Pause(_ sender: Any) {

        switch ( cpuState ) {
        case cpuState_halted:
            Resume()
            break
            
        case cpuState_running:
            Pause()
            break
            
        default:
//            upd.suspend()
//            cpuState = cpuState_halted
            break
        }
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
    var flashInverted = false
    
//    let spaceChar : Character = "\u{E17F}"
//    let blockChar : Character = "\u{E07F}"
//    static let spaceChar : Character = " "
//    static let blockChar : Character = "░"
//    static var flashingSpace : Character = " "
    
    let ramBufferPointer = UnsafeRawBufferPointer(start: MEM, count: 64 * 1024)
    static let textPage1Pointer = UnsafeRawBufferPointer(start: MEM + textPage1Addr, count: textBufferSize)
    static let textPage2Pointer = UnsafeRawBufferPointer(start: MEM + textPage2Addr, count: textBufferSize)
    static let textIntPage1BufferPointer = UnsafeRawBufferPointer(start: RAM + textPage1Addr, count: textBufferSize)
    static let textIntPage2BufferPointer = UnsafeRawBufferPointer(start: RAM + textPage2Addr, count: textBufferSize)
    static let textAuxBufferPointer = UnsafeRawBufferPointer(start: AUX + textPage1Addr, count: textBufferSize)
    
    static let textPageShadowBuffer = UnsafeMutableRawBufferPointer.allocate(byteCount: textBufferSize, alignment: 1)
    
    // TODO: Render text screen in native C
//    static let textScreen = UnsafeMutableRawPointer(mutating: testText)

    
    var textBufferPointer = textPage1Pointer

    static let textArraySize = textLines * (textCols + lineEndChars) + textCols * 2

    let txtClear = [Character](repeating: " ", count: textArraySize * 2)
    var unicodeTextArray = [Character](repeating: " ", count: textArraySize * 2)

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
            self.textDisplay.string = txt
            self.speedometer.stringValue = String(format: "%0.3lf MHz", mhz);
        }
    }
    
    
    // AppleScript Keycodes
    let leftArrowKey    = 123
    let rightArrowKey   = 124
    let upArrowKey      = 126
    let downArrowKey    = 125

    let F4FunctionKey   = 118
    let F5FunctionKey   =  96
    let F6FunctionKey   =  97
    let F7FunctionKey   =  98
    let F8FunctionKey   = 100


    override var acceptsFirstResponder: Bool {
        get {
            return true
        }
    }

    
    func SelectAll() {
//        textDisplayScroller.currentEditor()?.selectAll(nil)
//        displayField.selectText(nil)
        textDisplay.setSelectedRange(NSRange())
    }
    
    func Copy() {
        let pasteBoard = NSPasteboard.general
        pasteBoard.clearContents()
        // TODO: Find a better way to avoid index out of range error when the entire text area is selected
        let string = textDisplay.string + " "
        let selectedRange = textDisplay.selectedRange()
        if selectedRange != NSRange() {
            let startIndex = string.index(string.startIndex, offsetBy: selectedRange.lowerBound)
            let endIndex = string.index(string.startIndex, offsetBy: selectedRange.upperBound)
            let selectedString = string[startIndex..<endIndex]
            pasteBoard.setString(String(selectedString), forType: .string)
        }
    }
    
    func Paste() {
        let pasteBoard = NSPasteboard.general
        if let str = pasteBoard.string( forType: .string ) {
            print("PASTED:", str)

            DispatchQueue.global(qos: .background).async {
                for char in str.uppercased() {
                    if let ascii = char.asciiValue {
                        // TODO: Write separate Paste Accelerator
//                        disk_accelerator_speedup()
                        kbdPaste(ascii)
                    }
                }
            }
        }
    }
    
    let mouseCursorHidden = true;
    let mouseCursorJoystickEmulation = NSCursor.crosshair
    let mouseCursorHiddenJoystickEmulation = NSCursor.init(image: NSImage.init(size: NSSize(width: 1, height: 1)), hotSpot: NSPoint(x: 0, y: 0))

    
    func mouseCursor(hide : Bool) {
        if hide {
            if mouseCursorHidden {
                // NSCursor.hide() is working weird, better to set a 1px transparent cursor
//                mouseCursorHiddenJoystickEmulation.set()
                mouseCursorJoystickEmulation.set()
            }
            else {
                mouseCursorJoystickEmulation.set()
            }
        }
        else {
//            if mouseCursorHidden {
//                NSCursor.unhide()
//            }
            NSCursor.arrow.set()
        }
    }

    
    func getScreenWithMouse() -> NSScreen? {
        let mouseLocation = NSEvent.mouseLocation
        let screens = NSScreen.screens
        let screenWithMouse = (screens.first { NSMouseInRect(mouseLocation, $0.frame, false) })
        
        return screenWithMouse
    }
    
    
    func convertPoint(toCG point : NSPoint) -> CGPoint {
        /// Cocoa and Core Graphics (a.k.a. Quartz) use different coordinate systems. In Cocoa, the origin is at the lower left of the primary screen and y increases as you go up. In Core Graphics, the origin is at the top left of the primary screen and y increases as you go down.
        /// Need to convert coordinates from Cocoa to Core Graphics
        var cgpoint = view.window!.convertPoint(toScreen: point)
        if let screen = getScreenWithMouse() {
            cgpoint.y = NSHeight(screen.frame) - cgpoint.y;
        }
        return cgpoint
    }
    
    
    override func mouseMoved(with event: NSEvent) {
//        print(#function)
        var location = event.locationInWindow
//        displayOrigin = textDisplayScroller.frame.origin.
//        print("mx:", location.x, " my:", location.y)
                
        var mouseCursorNeedsReplace = false
        
        if location.x < 8 {
            mouseCursorNeedsReplace = true
            location.x = 8
        }
        if location.x > textDisplay.frame.width - 8 {
            mouseCursorNeedsReplace = true
            location.x = textDisplay.frame.width - 8
        }
        if location.y < 8 {
            mouseCursorNeedsReplace = true
            location.y = 8
        }
        if location.y > textDisplay.frame.height - 8 {
            mouseCursorNeedsReplace = true
            location.y = textDisplay.frame.height - 8
        }

        mouseCursor(hide: Mouse2Joystick)
        
        if ( Mouse2Joystick ) {
            if mouseCursorNeedsReplace {
                CGWarpMouseCursorPosition(convertPoint(toCG: location))
            }
            
            pdl_prevarr[0] = pdl_valarr[0]
            pdl_valarr[0] = Double(location.x / (textDisplay.frame.width) )
            pdl_diffarr[0] = pdl_valarr[0] - pdl_prevarr[0]
            
            pdl_prevarr[1] = pdl_valarr[1]
            pdl_valarr[1] = 1 - Double(location.y / (textDisplay.frame.height) )
            pdl_diffarr[1] = pdl_valarr[1] - pdl_prevarr[1]
        }
        
        if ( MouseInterface ) {
            pdl_prevarr[2] = pdl_valarr[2]
            pdl_valarr[2] = Double(location.x / (textDisplay.frame.width) )
            pdl_diffarr[2] = pdl_valarr[2] - pdl_prevarr[2]
            
            pdl_prevarr[3] = pdl_valarr[3]
            pdl_valarr[3] = 1 - Double(location.y / (textDisplay.frame.height) )
            pdl_diffarr[3] = pdl_valarr[3] - pdl_prevarr[3]
        }
    }


    var savedVideoMode = videoMode_t.init()


    override func keyDown(with event: NSEvent) {
        
        m6502.ecoSpindown = ecoSpindown;
        
        if ( cpuMode == cpuMode_eco ) {
            cpuState = cpuState_running;
            #if SCHEDULER_CVDISPLAYLINK
            CVDisplayLinkStart(displayLink!)
            #else
            upd.resume()
            #endif
        }
        
//        print("keyDown")
        
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
                case "a":
//                    print("CMD + A")
                    SelectAll()
                    return // to avoid deselect text
                    
                case "c":
//                    print("CMD + C")
                    Copy()
                    
                case "v":
//                    print("CMD + V")
                    Paste()

                default:
                    super.keyDown(with: event)
                }
            }
        }
        else {
            #if FUNCTIONTEST
            #else
            let keyCode = Int(event.keyCode)
            switch keyCode {
            case leftArrowKey:
//                print("LEFT");
                if ( Keyboard2Joystick ) {
                    // Keyboard 2 JoyStick (Game Controller / Paddle)
                    pdl_valarr[0] = 0
                }
                kbdInput(0x88)
                
            case rightArrowKey:
//                print("RIGHT")
                // Keyboard 2 JoyStick (Game Controller / Paddle)
                if ( Keyboard2Joystick ) {
                    pdl_valarr[0] = 1
                }
                kbdInput(0x95)
                
            case downArrowKey:
//                print("DOWN")
                // Keyboard 2 JoyStick (Game Controller / Paddle)
                if ( Keyboard2Joystick ) {
                    pdl_valarr[1] = 1
                }
                else {
                    kbdInput(0x8A)
                }
                
            case upArrowKey:
//                print("UP")
                // Keyboard 2 JoyStick (Game Controller / Paddle)
                if ( Keyboard2Joystick ) {
                    pdl_valarr[1] = 0
                }
                else {
                    kbdInput(0x8B)
                }

            case F4FunctionKey:
//                if let debugger = DebuggerWindowController.current {
//                    debugger.Continue()
//                }

                Resume()
                
            case F5FunctionKey:
//                if let debugger = DebuggerWindowController.current {
//                    debugger.Pause()
//                }

                Pause()

            case F6FunctionKey:
                if let debugger = DebuggerWindowController.shared {
                    debugger.Step_Over(event)
                }

            case F7FunctionKey:
                if let debugger = DebuggerWindowController.shared {
                    debugger.Step_In(event)
                }

            case F8FunctionKey:
                if let debugger = DebuggerWindowController.shared {
                    debugger.Step_Out(event)
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
        
//        displayField.currentEditor()?.selectedRange = NSMakeRange(0, 0)
        textDisplay.setSelectedRange(NSRange())
    }
    
    
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
        let flags = event.modifierFlags.intersection(.deviceIndependentFlagsMask)
        switch flags {
        case [.shift]:
            setIO(0xC061, 0)
            setIO(0xC062, 0)
            setIO(0xC063, 0) // inverted (bit 7: not pressed)
//            print("shift key is pressed")
//        case [.control]:
//            print("control key is pressed")
        case [.option] :
            setIO(0xC061, 0)
            setIO(0xC062, 1 << 7)
            setIO(0xC063, 1 << 7) // inverted (bit 7: not pressed)
//            print("option key is pressed")
        case [.command]:
//            print("Command key is pressed")
            setIO(0xC061, 1 << 7)
            setIO(0xC062, 0)
            setIO(0xC063, 1 << 7) // inverted (bit 7: not pressed)
//        case [.control, .shift]:
//            print("control-shift keys are pressed")
        case [.option, .shift]:
            setIO(0xC061, 1 << 7)
            setIO(0xC062, 0)
            setIO(0xC063, 0) // inverted (bit 7: not pressed)
//            print("option-shift keys are pressed")
        case [.command, .shift]:
            setIO(0xC061, 1 << 7)
            setIO(0xC062, 0)
            setIO(0xC063, 0) // inverted (bit 7: not pressed)
//            print("command-shift keys are pressed")
//        case [.control, .option]:
//            print("control-option keys are pressed")
//        case [.control, .command]:
//            print("control-command keys are pressed")
        case [.option, .command]:
            setIO(0xC061, 1 << 7)
            setIO(0xC062, 1 << 7)
            setIO(0xC063, 1 << 7) // inverted (bit 7: not pressed)
//            print("option-command keys are pressed")
//        case [.shift, .control, .option]:
//            print("shift-control-option keys are pressed")
//        case [.shift, .control, .command]:
//            print("shift-control-command keys are pressed")
//        case [.control, .option, .command]:
//            print("control-option-command keys are pressed")
        case [.shift, .command, .option]:
            setIO(0xC061, 1 << 7)
            setIO(0xC062, 1 << 7)
            setIO(0xC063, 0); // inverted (bit 7: not pressed)
//            print("shift-command-option keys are pressed")
//        case [.shift, .control, .option, .command]:
//            print("shift-control-option-command keys are pressed")
//        case [.function]:
//            print("function key is pressed")
//        case [.capsLock]:
//            print("capsLock key is pressed")
        
        case [.control, .command, .option]:
            Mouse2Joystick = !Mouse2Joystick
            mouseCursor(hide: Mouse2Joystick)
            ToolBarController.current?.MouseToJoystickMenuItem.state = Mouse2Joystick ? .on : .off

        default:
            setIO(0xC061, 0)
            setIO(0xC062, 0)
            setIO(0xC063, 1 << 7) // inverted (bit 7: not pressed)
//            print("no modifier keys are pressed")
        }
    }

    
    var was = 0;
    
    var currentVideoMode = videoMode
    var lastFrameTime = CACurrentMediaTime() as Double
    var frameCounter : UInt32 = 0
    var clkCounter : Double = 0
    
    var shadowTxt : String = ""
    var unicodeTextString : String = ""

    
    func Flash() {
        self.frameCnt += 1
        
        if ( self.frameCnt == fps / video_fps_divider / 2 ) {
            if !flashInverted {
                ViewController.charConvTbl = ViewController.charConvTblFlashOn
                flashInverted = true
            }
        }
        else if ( self.frameCnt >= fps / video_fps_divider ) {
            self.frameCnt = 0
            
            if flashInverted {
                ViewController.charConvTbl = ViewController.charConvTblFlashOff
                flashInverted = false
            }
        }
    }

    
    var textNeedRender = false
    
    func RenderText() {
        Flash()

        textNeedRender = false
        
        var fromLines = 0
        var toLines = textLines
        
        if videoMode.text == 0 {
            if videoMode.mixed == 1 {
                fromLines = toLines - 4
            }
            else {
                toLines = 0
            }
        }
        
        unicodeTextArray = NSArray(array: txtClear, copyItems: true) as! [Character]
        
        // render an empty space to eiminate displaying text portion of the screen covered by graphics
        let charDisposition = videoMode.col80 == 0 ? 1 : 2
        for y in 0 ..< fromLines {
            unicodeTextArray[ y * (textCols * charDisposition + lineEndChars) + textCols * charDisposition] = "\n"
        }
        
        // 40 col
        if videoMode.col80 == 0 {
            if MEMcfg.txt_page_2 == 0 {
                if (MEMcfg.RD_AUX_MEM == 0) {
                    textBufferPointer = ViewController.textPage1Pointer
                }
                else {
                    textBufferPointer = ViewController.textIntPage1BufferPointer
                }
            }
            else {
                if (MEMcfg.RD_AUX_MEM == 0) {
                    textBufferPointer = ViewController.textPage2Pointer
                }
                else {
                    textBufferPointer = ViewController.textIntPage2BufferPointer
                }
            }
            
            if textBufferPointer.elementsEqual(ViewController.textPageShadowBuffer) {
            }
            else {
                ViewController.textPage1Pointer.copyBytes(to: ViewController.textPageShadowBuffer)
                textNeedRender = true
                
                // render the rest of the text screen
                for y in fromLines ..< toLines {
                    for x in 0 ..< textCols {
                        let byte = textBufferPointer[ ViewController.textLineOfs[y] + x ]
                        let idx = Int(byte);
                        let chr = ViewController.charConvTbl[idx]
                        
                        unicodeTextArray[ y * (textCols + lineEndChars) + x ] = chr
                    }
                    
                    unicodeTextArray[ y * (textCols + lineEndChars) + textCols ] = "\n"
                }
                
                unicodeTextString = String(unicodeTextArray)
            }
        }
        // 80 col
        else {
            let auxPage = ( MEMcfg.is_80STORE == 1 ) && ( MEMcfg.txt_page_2 == 1 )
            
            let textIntBuffer = auxPage ?  ViewController.textIntPage1BufferPointer : ViewController.textPage1Pointer
            let textAuxBuffer = auxPage ?  ViewController.textPage1Pointer : ViewController.textAuxBufferPointer
            
            // render the rest of the text screen
            for y in fromLines ..< toLines {
                for x in 0 ..< textCols {
                    let byte = textIntBuffer[ ViewController.textLineOfs[y] + x ]
                    let idx = Int(byte);
                    let chr = ViewController.charConvTbl[idx]
                    
                    unicodeTextArray[ y * (textCols * 2 + lineEndChars) + x * 2 + 1] = chr
                    
                    let byte2 = textAuxBuffer[ ViewController.textLineOfs[y] + x ]
                    let idx2 = Int(byte2);
                    let chr2 = ViewController.charConvTbl[idx2]
                    
                    unicodeTextArray[ y * (textCols * 2 + lineEndChars) + x * 2] = chr2
                }
                
                unicodeTextArray[ y * (textCols * 2 + lineEndChars) + textCols * 2] = "\n"
            }
            
            unicodeTextString = String(unicodeTextArray)
        }
    }

    func SetSplashScreenFont() {
        for view in splashScreen.subviews {
            if view is NSTextField {
                let textField = view as! NSTextField
                if let fontSize = textField.font?.pointSize {
                    textField.font = NSFont(name: "PrintChar21", size: fontSize)
                }
            }
        }

        // Set Apple ][ font
        if let fontSize = textDisplay.font?.pointSize {
            textDisplay.font = NSFont(name: "PrintChar21", size: fontSize)
            ViewController.charConvTblFlashOn = ViewController.charConvTblFlashOn40
            ViewController.charConvTblFlashOff = ViewController.charConvTblFlashOff40
        }
    }


    func SetCol40() {
        // Set Apple ][ font
        if let fontSize = textDisplay.font?.pointSize {
            textDisplay.font = NSFont(name: "PrintChar21", size: fontSize)
            ViewController.charConvTblFlashOn = ViewController.charConvTblFlashOn40
            ViewController.charConvTblFlashOff = ViewController.charConvTblFlashOff40
        }
    }


    func SetCol80() {
        // Set Apple ][ font
        if let fontSize = textDisplay.font?.pointSize {
            textDisplay.font = NSFont(name: "PRNumber3", size: fontSize)
            ViewController.charConvTblFlashOn = ViewController.charConvTblCol80
            ViewController.charConvTblFlashOff = ViewController.charConvTblCol80
        }
    }


    func UpdateText() {

// TODO: Render text Screen in native C
//            txt = String(bytesNoCopy: ViewController.textScreen!, length: 10, encoding: .ascii, freeWhenDone: false) ?? "HMM"
        
        if videoMode.col80 != currentVideoMode.col80 {
            currentVideoMode.col80 = videoMode.col80
            
            if videoMode.col80 == 1 {
                SetCol80()
            }
            else {
                SetCol40()
            }
        }
        
        if textNeedRender || shadowTxt != unicodeTextString {
            shadowTxt = unicodeTextString

            let selectedRange = textDisplay.selectedRange()

//            DispatchQueue.main.async { [self] in
                textDisplay.string = unicodeTextString
                textDisplay.setSelectedRange(selectedRange)
//            }
            
//                let bold14 = NSFont.boldSystemFont(ofSize: 14.0)
//                let textColor = NSColor.red
//                let attribs = [NSAttributedString.Key.font:bold14,NSAttributedString.Key.foregroundColor:textColor,NSAttributedString.Key.paragraphStyle:textParagraph]

//                let textParagraph = NSMutableParagraphStyle()
//                textParagraph.lineSpacing = 0
//                textParagraph.minimumLineHeight = 32.0
//                textParagraph.maximumLineHeight = 32.0
//
//                let attribs = [NSAttributedString.Key.paragraphStyle: textParagraph]
//                let attrString:NSAttributedString = NSAttributedString.init(string: unicodeTextString, attributes: attribs)
//                display.attributedStringValue = attrString
        }
//            display.stringValue = "testing\nit\nout"
    }


    func UpdateCPUspeed() {
//        DispatchQueue.main.async { [self] in
        // under ~1.5 MHz -- 3 decimals to be able to display 1.023 MHz
        if ( (mhz < 1.4) && (mhz != floor(mhz)) ) {
            speedometer.stringValue = String(format: "%0.3lf MHz", mhz);
        }
        // under ~100 MHz -- 1 decimal
        else if (mhz < 95) {
            speedometer.stringValue = String(format: "%0.1lf MHz", mhz);
        }
        // over ~1000 MHz -- 1 decimal GHz
        else if (mhz > 950) {
            speedometer.stringValue = String(format: "%0.1lf GHz", mhz / 1000);
        }
        // hundreds -- no decimals
        else {
            speedometer.stringValue = String(format: "%0.0lf MHz", mhz);
        }
//        }
    }
    
    
    func RenderGraphics() {
        // only refresh graphics view when needed (aka not in text mode)
        if ( videoMode.text == 0 ) {
            if ( videoMode.hires == 0 ) {
                // when we change video mode, buffer needs to be cleared to avoid artifacts
                if ( savedVideoMode.text == 1 )
                    || ( savedVideoMode.mixed != videoMode.mixed )
                    || ( savedVideoMode.hires != videoMode.hires )
                {
                    lores.clearScreen()
                    lores.isHidden = false
                    hires.isHidden = true
                    unicodeTextString = String(unicodeTextArray)
                }
                
                lores.Render()
            }
            else {
                // when we change video mode, buffer needs to be cleared to avoid artifacts
                if ( savedVideoMode.text == 1 )
                    || ( savedVideoMode.mixed != videoMode.mixed )
                    || ( savedVideoMode.hires != videoMode.hires )
                {
                    hires.clearScreen()
                    hires.isHidden = false
                    lores.isHidden = true
                    unicodeTextString = String(unicodeTextArray)
                }
                
                hires.Render()
            }
        }
        else if ( savedVideoMode.text == 0 ) {
            // we just switched from grahics to text
            lores.isHidden = true
            hires.isHidden = true
            unicodeTextString = String(unicodeTextArray)
        }
        
        savedVideoMode = videoMode
    }
    
    
    func Render() {
        self.RenderText()

        // Rendering is happening in the main thread, which has two implications:
        //   1. We can update UI elements
        //   2. it is independent of the simulation, de that is running in the background thread while we are busy with rendering...
//        DispatchQueue.global(qos: .userInitiated).async {
        DispatchQueue.main.async {
            self.UpdateText()
            self.UpdateCPUspeed()
                
            #if HIRES
            self.RenderGraphics()
            #endif // HIRES
        }
    }
    
    
    override func mouseDown(with event: NSEvent) {
//        print(#function)
        
        switch event.modifierFlags.intersection(.deviceIndependentFlagsMask) {
        case [.control, .command, .option]:
            Mouse2Joystick = !Mouse2Joystick
            mouseCursor(hide: Mouse2Joystick)
            ToolBarController.current?.MouseToJoystickMenuItem.state = Mouse2Joystick ? .on : .off

        default:
            break
        }

        if ( Mouse2Joystick ) {
            setIO(0xC061, 1 << 7)
        }
        
    }

    override func mouseUp(with event: NSEvent) {
//        print(#function)
        if ( Mouse2Joystick ) {
            setIO(0xC061, 0)
        }
    }
    
    override func rightMouseDown(with event: NSEvent) {
//        print(#function)
        if ( Mouse2Joystick ) {
            setIO(0xC062, 1 << 7)
        }
    }
    
    override func rightMouseUp(with event: NSEvent) {
//        print(#function)
        if ( Mouse2Joystick ) {
            setIO(0xC062, 0)
        }
    }
    
    override func otherMouseDown(with event: NSEvent) {
//        print(#function)
        if ( Mouse2Joystick ) {
            setIO(0xC063, 0) // inverted (bit 7: 0 = pressed)
        }
    }
    
    override func otherMouseUp(with event: NSEvent) {
//        print(#function)
        if ( Mouse2Joystick ) {
            setIO(0xC063, 1 << 7) // inverted (bit 7: 1 = not pressed)
        }
    }


    func diskButtonUpdate() {
        DispatchQueue.main.async {

            if ( self.frameCounter % DEF_DRV_LED_DIV == 0 ) {

                // Disk Motor LED
                if spkr_is_disk_motor_playing() {
                    if self.disk1_led.isHidden {
                        self.disk1_led.isHidden = false
                    }
                }
                else {
                    if !self.disk1_led.isHidden {
                        self.disk1_led.isHidden = true
                    }
                }

                // Disk Loaded
                if woz_is_loaded() > 0 {
                    if self.disk1_closed.isHidden {
                        self.disk1_closed.isHidden = false
                    }
                }
                else {
                    if !self.disk1_closed.isHidden {
                        self.disk1_closed.isHidden = true
                    }
                }

            }
        }
    }


    func debugBreak() {
        Pause()
        spkr_play_disk_motor_time = 0
        spkr_stopAll()

        // TODO: This should be in Debugger!
        debuggerPauseUpdate()
        debuggerShowWindow()
    }


    let UpdateSemaphore = DispatchSemaphore(value: 1)
    func Update() {
//        clk_6502_per_frm_max = 0

        if UpdateSemaphore.wait(timeout: .now() + 0.001) == .timedOut {
            // get back here next time...
            return
        }

        diskButtonUpdate()

        switch cpuState {
            case cpuState_running:
                clkCounter += Double(m6502.clkfrm)
                // we start a new frame from here, so CPU is running even while rendering
//                m6502.clkfrm = 0

                frameCounter += 1

                if ( frameCounter % fps == 0 ) {
                    let currentTime = CACurrentMediaTime() as Double
                    let elpasedTime = currentTime - lastFrameTime
                    lastFrameTime = currentTime
                    mhz = Double( clkCounter ) / (elpasedTime * M);
                    clkCounter = 0
                }
                
                #if SPEEDTEST
                #else
                
                // poll input devices like mouse and joystick
//                Input()
                
                // run some code
                // cpuState = cpuState_executing
//                DispatchQueue.global(qos: .userInitiated).async {
                if m6502.debugger.on {
                    m6502_Debug()

                    switch m6502.interrupt {
                    case HALT:
                        debugBreak()

                    case BREAK: // BRK instruction
                        debugBreak()

                    case BREAKPOINT: // CPU halted because of a breakpoint
                        debugBreak()

                    case BREAKRDMEM: // CPU halted because of a breakpoint
                        debugBreak()

                    case BREAKWRMEM: // CPU halted because of a breakpoint
                        debugBreak()

                    case RET:
                        if m6502.debugger.mask.ret == 1 {
                            // Step_Out / Step_Over
                            if m6502.PC >= m6502.debugger.SP {
                                debugBreak()
                            }
                        }

                    case INV: // invalid instruction
                        debugBreak()

                    default:
                        break
                    }

                    // clear iterrupt
                    m6502.interrupt = NO_INT
                }
                else {
//                    DispatchQueue.global(qos: .userInitiated).async {
                        m6502_Run()
//                    }
                    // cpuState = cpuState_running
                }

                // video rendering
                if ( frameCounter % video_fps_divider == 0 ) {
                    Render()
                }

                // TODO: This should be in Debugger!
                if let debugger = DebuggerViewController.shared {
                    debugger.Update()
                }

                #endif
                
                break
                
            case cpuState_executing:
                // prevent running more instances per session
//                setCPUClockSpeed(freq: MHz_6502 - 1)
                break

            case cpuState_halting:
                cpuState = cpuState_halted
                // last video rendering before halt
                Render()

                break
            
            case cpuState_halted:
                #if SCHEDULER_CVDISPLAYLINK
                CVDisplayLinkStop(displayLink!)
                #else
                upd.suspend()
                #endif
                
                break
                
            default:
                break
        }

        // ok, from now you can update again
        UpdateSemaphore.signal()
    }

    
//    func FromBuf(ptr: UnsafeMutablePointer<UInt8>, length len: Int) -> String? {
//        // convert the bytes using the UTF8 encoding
//        if let theString = NSString(bytes: ptr, length: len, encoding: NSUTF8StringEncoding) {
//            return theString as String
//        } else {
//            return nil // the bytes aren't valid UTF8
//        }
//    }
    
    
    #if SCHEDULER_CVDISPLAYLINK
    #else
    var upd = RepeatingTimer(timeInterval: 1)
    
    func newUpdateTimer( timeInterval : Double ) {
        upd.kill()
        upd = RepeatingTimer(timeInterval: timeInterval)
        upd.eventHandler = {
            self.Update()
        }
        upd.resume()
    }

    #endif

    
    // Kelvin Sherlock's fix to avoid uninstalled font problems
    override func awakeFromNib() {
//        self.display.font = NSFont(name: "PrintChar21", size: 32)
    }
    
    required init?(coder: NSCoder) {
//        print(#function)
        super.init(coder: coder)
        
        ViewController.shared = self
    }
    
    
//    func render() {
//
//        var x = vertexData[2 * 3 + 0]
//        x -= 0.01
//        if x < -1 {
//            x = 1
//        }
//        vertexData[2 * 3 + 0] = x
//
//        guard let drawable = metalLayer?.nextDrawable() else { return }
//        let renderPassDescriptor = MTLRenderPassDescriptor()
//        renderPassDescriptor.colorAttachments[0].texture = drawable.texture
//        renderPassDescriptor.colorAttachments[0].loadAction = .clear
//        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColor(
//            red: 0.0,
//            green: 104.0/255.0,
//            blue: 55.0/255.0,
//            alpha: 0.3)
//
//        if let commandBuffer = commandQueue.makeCommandBuffer() {
//            if let renderEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor) {
//                renderEncoder.setRenderPipelineState(pipelineState)
//                let dataSize = vertexData.count * MemoryLayout.size(ofValue: vertexData[0]) // 1
//                vertexBuffer = device.makeBuffer(bytes: vertexData, length: dataSize, options: []) // 2
//                renderEncoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
//                renderEncoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3, instanceCount: 1)
//                renderEncoder.endEncoding()
//                commandBuffer.present(drawable)
//                commandBuffer.commit()
//            }
//        }
//    }
//
//
//    var vertexData: [Float] = [
//        0.0,  1.0, 0.0,
//        -1.0, -1.0, 0.0,
//        1.0, -1.0, 0.0
//    ]
    
#if SCHEDULER_CVDISPLAYLINK

    //  The callback function is called everytime CVDisplayLink says its time to get a new frame.
    let displayLinkOutputCallback : CVDisplayLinkOutputCallback = { (displayLink: CVDisplayLink, _ inNow: UnsafePointer<CVTimeStamp>, _ inOutputTime: UnsafePointer<CVTimeStamp>, _ flagsIn: CVOptionFlags, _ flagsOut: UnsafeMutablePointer<CVOptionFlags>, _ vController: UnsafeMutableRawPointer?) -> CVReturn in
        
        /*  The displayLinkContext is CVDisplayLink's parameter definition of the view in which we are working.
         In order to access the methods of a given view we need to specify what kind of view it is as right
         now the UnsafeMutablePointer<Void> just means we have a pointer to "something".  To cast the pointer
         such that the compiler at runtime can access the methods associated with our SwiftOpenGLView, we use
         an unsafeBitCast.  The definition of which states, "Returns the the bits of x, interpreted as having
         type U."  We may then call any of that view's methods.  Here we call drawView() which we draw a
         frame for rendering.  */
        //            unsafeBitCast(displayLinkContext, SwiftOpenGLView.self).renderFrame()

/// This can be a precise FPS updater so MHz would be dead perfect pitch -- however, it makes things worse
//        let now : CVTimeStamp = inNow.pointee
////            print( "RateScaler:", now.rateScalar )
//        if (mySelf.last_frame_time != 0) {
//            let videoTimeDiff = now.videoTime - mySelf.last_frame_time
//            let currentFPS = Double(now.videoTimeScale) / Double(videoTimeDiff)
//
//            fps = currentFPS
//            //                mySelf.setCPUClockSpeed(freq: MHz_6502)
//            clk_6502_per_frm = UInt64( MHz_6502 * M / currentFPS )
//            clk_6502_per_frm_set = clk_6502_per_frm
//
//        }
//        mySelf.last_frame_time = now.videoTime;

        let mySelf = Unmanaged<ViewController>.fromOpaque(vController!).takeUnretainedValue()
        mySelf.Update();

        //  We are going to assume that everything went well for this mock up, and pass success as the CVReturn
        return kCVReturnSuccess
    }
    
    // sets a callback at every screen refresh (normally 60Hz)
    func setupDisplayLink() {
        //  Grab the a link to the active displays, set the callback defined above, and start the link.
        /*  An alternative to a nested function is a global function or a closure passed as the argument--a local function
         (i.e. a function defined within the class) is NOT allowed. */
        //  The UnsafeMutablePointer<Void>(unsafeAddressOf(self)) passes a pointer to the instance of our class.
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)
        
        let unsafeSelf: UnsafeMutableRawPointer = Unmanaged.passUnretained(self).toOpaque()
        CVDisplayLinkSetOutputCallback(displayLink!, displayLinkOutputCallback, unsafeSelf)
//        CVDisplayLinkStart(displayLink!)
    }
#endif

    @IBOutlet weak var disk1_led: NSImageView!
    @IBOutlet weak var disk2_led: NSImageView!
    @IBOutlet weak var disk1_closed: NSImageView!
    @IBOutlet weak var disk2_closed: NSImageView!


    var keyDownMonitor : Any?
    var keyUpMonitor : Any?

    func keyEventsOn() {
//        NSEvent.removeMonitor(NSEvent.EventType.flagsChanged)
//        NSEvent.addLocalMonitorForEvents(matching: .flagsChanged) {
//            self.flagsChanged(with: $0)
//            return $0
//        }

        if let event = keyDownMonitor {
            NSEvent.removeMonitor(event)
            keyDownMonitor = nil
        }
        keyDownMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) {
//            print("keyDown event")
            self.keyDown(with: $0)
            return nil
        }
        if let event = keyUpMonitor {
            NSEvent.removeMonitor(event)
            keyUpMonitor = nil
        }
        keyUpMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyUp) {
//            print("keyUp event")
            self.keyUp(with: $0)
            return nil
        }
    }


    func keyEventsOff() {
//        NSEvent.removeMonitor(NSEvent.EventType.flagsChanged)
        if let event = keyDownMonitor {
            NSEvent.removeMonitor(event)
            keyDownMonitor = nil
        }
        if let event = keyUpMonitor {
            NSEvent.removeMonitor(event)
            keyUpMonitor = nil
        }
    }


    override func viewDidLoad() {
        super.viewDidLoad()

//        if let image = Disk1_open_on_img {
//            Disk1_ButtonCell.alternateImage = image
//            NSLog("Disk1_ButtonCell:%@", Disk1_ButtonCell)
//        }

//        let layer = CALayer()
//        hires.layer = layer
//        hires.wantsLayer = true
        
//        device = MTLCreateSystemDefaultDevice()
//        metalLayer = CAMetalLayer()          // 1
//        metalLayer.device = device           // 2
//        metalLayer.pixelFormat = .bgra8Unorm // 3
//        metalLayer.framebufferOnly = true    // 4
//        metalLayer.frame = hires.layer!.frame  // 5
//        hires.layer!.addSublayer(metalLayer)   // 6
//
//        let dataSize = vertexData.count * MemoryLayout.size(ofValue: vertexData[0]) // 1
//        vertexBuffer = device.makeBuffer(bytes: vertexData, length: dataSize, options: []) // 2
//
//        // 1
//        let defaultLibrary = device.makeDefaultLibrary()!
//        let fragmentProgram = defaultLibrary.makeFunction(name: "basic_fragment")
//        let vertexProgram = defaultLibrary.makeFunction(name: "basic_vertex")
//
//        // 2
//        let pipelineStateDescriptor = MTLRenderPipelineDescriptor()
//        pipelineStateDescriptor.vertexFunction = vertexProgram
//        pipelineStateDescriptor.fragmentFunction = fragmentProgram
//        pipelineStateDescriptor.colorAttachments[0].pixelFormat = .bgra8Unorm
//
//        // 3
//        pipelineState = try! device.makeRenderPipelineState(descriptor: pipelineStateDescriptor)
//
//        commandQueue = device.makeCommandQueue()

//        timer = CADisplayLink(target: self, selector: #selector(gameloop))
//        timer.add(to: RunLoop.main, forMode: .default)


        // Set Apple ][ font
        SetCol40()
        SetSplashScreenFont()

        openLog()
        
        hires.clearScreen();
        
        spkr_load_sfx( Bundle.main.resourcePath! + "/sfx" )
        
        let woz_err = woz_loadFile( Bundle.main.resourcePath! + "/dsk/Apple DOS 3.3 January 1983.woz" )
        chk_woz_load(err: woz_err)
        woz_flags.image_file_readonly = 1

        //view.frame = CGRect(origin: CGPoint(), size: NSScreen.main!.visibleFrame.size)
                
//        createHiRes()
        
        self.textDisplayScroller.scaleUnitSquare(to: NSSize(width: 1, height: 1))
        
        keyEventsOn()

//        displayField.maximumNumberOfLines = textLines
//        displayField.preferredMaxLayoutWidth = displayField.frame.width

//        DispatchQueue.main.asyncAfter(deadline: .now() + 1/fps, execute: {
//            self.update()
//        })

//        #if FUNCTIONTEST
//        #else
//        DispatchQueue.global(qos: .background).async {
//            self.update()
//        }
        
//        upd.eventHandler = {
//            self.Update()
//        }
//        upd.resume()
        
        
        
#if SCHEDULER_CVDISPLAYLINK
        //  Grab the a link to the active displays, set the callback defined above, and start the link.
        /*  An alternative to a nested function is a global function or a closure passed as the argument--a local function
         (i.e. a function defined within the class) is NOT allowed. */
        //  The UnsafeMutablePointer<Void>(unsafeAddressOf(self)) passes a pointer to the instance of our class.
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)
        let unsafeSelf: UnsafeMutableRawPointer = Unmanaged.passUnretained(self).toOpaque()
        CVDisplayLinkSetOutputCallback(displayLink!, displayLinkOutputCallback, unsafeSelf)
        CVDisplayLinkStart(displayLink!)
#else
        newUpdateTimer( timeInterval: 1 / Double(fps) )
#endif
        
//        soundGapSlider.integerValue = Int(spkr_extra_buf)
//        ledingInitEdgeLabel.title = "ILE: " + String( SPKR_INITIAL_LEADING_EDGE )
//        initialLeadEdgeSlider.floatValue = SPKR_INITIAL_LEADING_EDGE
//        leadingEdgeLabel.title = "LE: " + String( SPKR_FADE_LEADING_EDGE )
//        leadEdgeSlider.floatValue = SPKR_FADE_LEADING_EDGE
//        trailingInitEdgeLabel.title = "ITE: " + String( SPKR_INITIAL_TRAILING_EDGE )
//        initialTailEdgeSlider.floatValue = SPKR_INITIAL_TRAILING_EDGE
//        trailingEdgeLabel.title = "TE: " + String( SPKR_FADE_TRAILING_EDGE )
//        tailEdgeSlider.floatValue = SPKR_FADE_TRAILING_EDGE

        
//        // BUGFIX: I am not sure why but if I do not adjust the frame and bounds size
//        //         couple of times, Cocoa miscalculates them
//        var size = MonitorView.textViewBounds
//        size.width /= 2
//        size.height /= 2
//        textDisplay.setFrameSize(size)
//        textDisplay.setBoundsSize(size)
//
//        size.width += 136 + 11 * 2
//        size.height += 64 + 11 * 2
//        view.setFrameSize(size)
//        view.setBoundsSize(size)
        
    }


    override func viewDidAppear() {
//        displayField.currentEditor()?.selectedRange = NSMakeRange(0, 0)
//        self.displayField.window?.makeFirstResponder(self)
        textDisplay.setSelectedRange(NSRange())
        textDisplay.window?.makeFirstResponder(self)

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) {
            self.monitorView.adjustTextDisplaySize()
        }
    }

    
    func setSpkrExtrabuf( freq : Double ) {
        // TODO: Probably this is not the best way to deal with the problem: To make sound continous independent of FPS and Freq

//        spkr_extra_buf = Int32( 780 / fps )

        switch freq {
        case 0.25:
            spkr_extra_buf = -65
            break

        case 0.5:
            spkr_extra_buf = -140
            break

        case 1.5:
            spkr_extra_buf = 175
            break

        case 2.0:
//            spkr_extra_buf = Int32( Double(spkr_extra_buf) * 2.961538461538462 ) // normally it should come up as 77, but this way it is calculated with FPS
//            spkr_extra_buf = 20
            spkr_extra_buf = 195 // 88
            break

        case 2.8:
            spkr_extra_buf = 65 // 185
            break

        case 4.0:
//            spkr_extra_buf = Int32( Double(spkr_extra_buf) * 1.346153846153846 ) // normally it should come up as 35, but this way it is calculated with FPS
//            spkr_extra_buf = 45
            spkr_extra_buf = 25 // 90 // 80 // 20
            break

        default:
//            spkr_extra_buf = Int32( 780 / fps ) // normally it should come up as 26, but this way it is calculated with FPS
            spkr_extra_buf = 0 // 26
            break
        }

        soundGapLabel.title = String( spkr_extra_buf )
        soundGapSlider.integerValue = Int(spkr_extra_buf)
    }


    func setCPUClockSpeed( freq : Double ) {
        spkr_stopAll();
        
        MHz_6502 = freq
        clk_6502_per_frm = UInt32( MHz_6502 * M / Double(fps) )
        clk_6502_per_frm_set = clk_6502_per_frm
            
        spkr_extra_buf = 0
//        setSpkrExtrabuf(freq: freq)
    }

    
    @IBOutlet weak var soundGapLabel: NSTextFieldCell!
    @IBAction func extraBuf(_ sender: NSSlider) {
        spkr_extra_buf = sender.intValue
        soundGapLabel.title = String( spkr_extra_buf )
    }
    
    @IBOutlet weak var ledingInitEdgeLabel: NSTextFieldCell!
    @IBAction func leadingInitEdgeSelected(_ sender: NSSlider) {
        SPKR_INITIAL_LEADING_EDGE = sender.floatValue
        ledingInitEdgeLabel.title = "ILE: " + String( SPKR_INITIAL_LEADING_EDGE )
    }
    
    @IBOutlet weak var leadingEdgeLabel: NSTextFieldCell!
    @IBAction func leadingEdgeSelected(_ sender: NSSlider) {
        SPKR_FADE_LEADING_EDGE = sender.floatValue
        leadingEdgeLabel.title = "LE: " + String( SPKR_FADE_LEADING_EDGE )
    }
    
    @IBOutlet weak var trailingInitEdgeLabel: NSTextFieldCell!
    @IBAction func trailingInitEdgeSelected(_ sender: NSSlider) {
        SPKR_INITIAL_TRAILING_EDGE = sender.floatValue
        trailingInitEdgeLabel.title = "ITE: " + String( SPKR_INITIAL_TRAILING_EDGE )
    }
    
    @IBOutlet weak var trailingEdgeLabel: NSTextFieldCell!
    @IBAction func trailingEdgeSelected(_ sender: NSSlider) {
        SPKR_FADE_TRAILING_EDGE = sender.floatValue
        trailingEdgeLabel.title = "TE: " + String( SPKR_FADE_TRAILING_EDGE )
    }
    
    
    @IBOutlet weak var wozExtraLabel: NSTextFieldCell!
    @IBAction func wozExtraSelected(_ sender: NSSlider) {
        extraForward = Int32(sender.floatValue)
        wozExtraLabel.title = "WE: " + String( extraForward )
    }
    
    
    @IBOutlet weak var EMALabel: NSTextFieldCell!
    @IBAction func EMASelected(_ sender: NSSlider) {
        spkr_ema_len = Int32(sender.floatValue)
        EMALabel.title = "EMA: " + String( spkr_ema_len )
    }
    
    
    func setSimulationMode( mode : String ) {
        switch ( mode ) {
        case "Eco":
            cpuMode = cpuMode_eco

            fps = DEFAULT_FPS
            video_fps_divider = ECO_VIDEO_DIV
            spkr_fps_divider = DEF_SPKR_DIV
            break
            
        case "Game":
            cpuMode = cpuMode_game
            cpuState = cpuState_running

            fps = GAME_FPS
            video_fps_divider = GAME_VIDEO_DIV
            spkr_fps_divider = GAME_SPKR_DIV
            break
            
        default:
            cpuMode = cpuMode_normal
            cpuState = cpuState_running
            
            fps = DEFAULT_FPS
            video_fps_divider = DEF_VIDEO_DIV
            spkr_fps_divider = DEF_SPKR_DIV
            break
        }

//        spkr_fps_divider = fps / spkr_fps
//        spkr_fps = fps;
        
        spkr_play_timeout = SPKR_PLAY_TIMEOUT // * spkr_fps_divider

//        pixelTrail = pow(256, 1 / Double(fps / video_fps_divider / 3) )

//        spkr_buf_size = spkr_sample_rate * 2 / spkr_fps
        #if SCHEDULER_CVDISPLAYLINK
        #else
        newUpdateTimer( timeInterval: 1 / Double(fps) )
        #endif
        
        setCPUClockSpeed(freq: MHz_6502)
        
        // TODO: Better way to deal with speaker!!!
        spkr_play_timeout = SPKR_PLAY_TIMEOUT * Int32(video_fps_divider)
    }
    
    
    @IBAction func setCPUMode(_ sender: NSPopUpButton) {
        setSimulationMode(mode: sender.selectedItem?.title ?? "Normal" )
    }
    
    @IBOutlet weak var initialLeadEdgeSlider: NSSlider!
    @IBOutlet weak var leadEdgeSlider: NSSlider!
    @IBOutlet weak var initialTailEdgeSlider: NSSlider!
    @IBOutlet weak var tailEdgeSlider: NSSlider!
    @IBOutlet weak var soundGapSlider: NSSlider!

//    @IBOutlet weak var soundGap: NSTextFieldCell!
//
//    @IBAction func SoundGapChanged(_ sender: NSStepper) {
//        SoundGap.integerValue = sender.integerValue
//        spkr_extra_buf = Int32( sender.integerValue )
//    }
    
    @IBAction func CRTMonitorOnOff(_ sender: NSButton) {
        CRTMonitor = sender.state == .on
        scanLines.isHidden = !CRTMonitor
        
        if ( CRTMonitor ) {
            textDisplay.textColor = .white
            // TODO: Adjust gamma so pixels are brighter
        }
        else {
            textDisplay.textColor = colorWhite
            // TODO: Adjust gamma so pixels are dimmer
        }
        
        hires.RenderFullScreen()
    }
    
    func ColorMonitorSelector( color : Bool ) {
        ColorMonitor = color
        
        if ( ColorMonitor ) {
            textDisplay.textColor = colorWhite // .white
        }
        else {
            textDisplay.textColor = colorGreen // .green
        }
        
        hires.RenderFullScreen()
    }
    
    @IBAction func ColorMonitorOnOff(_ sender: NSButton) {
        ColorMonitorSelector( color: sender.state == .on )
    }

    func MonoMonitorChange( color: String ) {
        switch color {
        case "Green":
            ColorMonitor = false
            monoColor = colorGreen
            hires.monoColor = hires.color_green
            
        case "Amber":
            ColorMonitor = false
            monoColor = colorOrange
            hires.monoColor = hires.color_orange
            
        default:
            ColorMonitor = false
            monoColor = colorWhite
            hires.monoColor = hires.color_white
        }
        
        textDisplay.textColor = monoColor
        hires.RenderFullScreen()
    }
    
    @IBAction func MonitorChange(_ sender: NSButton) {
        switch sender.title {
        case "Green Mono":
            MonoMonitorChange(color: "Green")
            
        case "Amber Mono":
            MonoMonitorChange(color: "Amber")

        default:
            MonoMonitorChange(color: "White")
        }
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
            diskAccelerator_enabled = 1;
        }
        else {
            diskAccelerator_enabled = 0;
        }
    }
    
    @IBAction func DiskSound(_ sender: NSButton) {
        if sender.state == .on {
            disk_sfx_enabled = 1;
        }
        else {
            disk_sfx_enabled = 0;
        }
    }
    
    
    func openDiskImage( url: URL ) {
        woz_eject()
            
        switch url.pathExtension.uppercased() {
            
        case "WOZ":
            let err = woz_loadFile( url.path )
            
            if err == WOZ_ERR_OK {
                NSDocumentController.shared.noteNewRecentDocumentURL(URL(fileURLWithPath: url.path))
            }
            else {
                self.chk_woz_load(err: err)
            }
            
        case "DSK", "DO", "PO" :
            let err = dsk2woz( url.path )
            
            if err == WOZ_ERR_OK {
                let err = woz_parseBuffer()
                
                if err == WOZ_ERR_OK {
                    NSDocumentController.shared.noteNewRecentDocumentURL(URL(fileURLWithPath: url.path))
                }
            }
            else {
                self.chk_woz_load(err: err)
            }
            
        default:
            break
            
        }

    }
    
    @objc func openDiskImageDialog() {
        let openPanel = NSOpenPanel()
        openPanel.title = "Open Disk Image"
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.allowedFileTypes = ["dsk","do","po","nib", "woz"]
        
        openPanel.begin { (result) -> Void in
            if result == NSApplication.ModalResponse.OK {
//                print("file:", openPanel.url!.path)
                //Do what you will
                //If there's only one URL, surely 'openPanel.URL'
                //but otherwise a for loop works
                
                if let url = openPanel.url {
                    self.openDiskImage(url: url)
                }
                else {
                    let a = NSAlert()
                    a.messageText = "File Not Found"
                    a.informativeText = "Could not locate selected file"
                    a.alertStyle = .critical
                    
                    a.beginSheetModal( for: self.view.window! )
                }
            }
        }
    }

    
    @objc func saveDiskImage() {
        let openPanel = NSOpenPanel()
        openPanel.title = "Save Disk Image"
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.allowedFileTypes = ["dsk","do","po","nib", "woz"]
        
        openPanel.begin { (result) -> Void in
            if result == NSApplication.ModalResponse.OK {
//                print("file:", openPanel.url!.path)
                //Do what you will
                //If there's only one URL, surely 'openPanel.URL'
                //but otherwise a for loop works
                
                if let filePath = openPanel.url?.path {
                    let woz_err = woz_saveFile( filePath )
                    
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
    
    @IBOutlet weak var QuickDisk_Disk1: NSMenuItem!
    @IBOutlet weak var QuickDisk_Disk2: NSMenuItem!
    @IBOutlet weak var DiskSound_Disk1: NSMenuItem!
    @IBOutlet weak var DiskSound_Disk2: NSMenuItem!

    @IBAction func Disk1(_ sender: NSPopUpButton) {
        switch sender.selectedItem?.tag {
        case 1: // Open
            openDiskImageDialog()
            
        case 2: // Save
            saveFile()
            
        case 3: // Save As...
            saveFileAs()

        case 4: // Eject
            woz_eject()
            
        case 21: // Quick Disk
            if diskAccelerator_enabled == 0 {
                diskAccelerator_enabled = 1;
            }
            else {
                diskAccelerator_enabled = 0;
            }
            
            let state : NSControl.StateValue = diskAccelerator_enabled == 1 ? .on : .off
            QuickDisk_Disk1?.state = state
            QuickDisk_Disk2?.state = state
            break

        case 22: // Disk Sound
            if disk_sfx_enabled == 0 {
                disk_sfx_enabled = 1;
            }
            else {
                disk_sfx_enabled = 0;
            }
            
            let state : NSControl.StateValue = disk_sfx_enabled == 1 ? .on : .off
            DiskSound_Disk1?.state = state
            DiskSound_Disk2?.state = state
            break

        case 1000: // Open Default Disk Image
            if let menuIdentifier = sender.selectedItem?.title {
                let woz_err = woz_loadFile( Bundle.main.resourcePath! + "/dsk/" + menuIdentifier + ".woz" )
                ViewController.shared?.chk_woz_load(err: woz_err)
                woz_flags.image_file_readonly = 1
            }
            
        default:
            break
        }
    }
    
    @IBAction func traceEnable(_ sender: NSButton) {
        switch sender.state {
        case .on:
            m6502.debugger.mask.trace = 1
            openLog()
            
        default:
            m6502.debugger.mask.trace = 0
            closeLog()
        }
    }
    
    
    func saveFile() {
        if ( woz_flags.image_file_readonly != 0 ) {
            // it is readonly, save it to a different file...
            saveFileAs()
        }
        else {
            // save WOZ image file overwriting the original image
            woz_saveFile(nil)
        }
    }
    
    func saveFileAs() {
        let savePanel = NSSavePanel()
        savePanel.title = "Save WOZ Disk Image As..."
        savePanel.begin { (result) in
            if result.rawValue == NSApplication.ModalResponse.OK.rawValue {
                woz_saveFile( savePanel.url?.path );
            }
            else {
                let a = NSAlert()
                a.messageText = "Are you sure?"
                a.informativeText = "Are you sure you would like to cancel and lose all modification you have made to the Disk Image?\nALERT: You will lose all new files and modified files from this Disk Image since you loaded. Your decision if permanent and irreversible!"
                a.addButton(withTitle: "Save")
                a.addButton(withTitle: "Cancel")
                a.alertStyle = .warning
                
                a.beginSheetModal(for: self.view.window!, completionHandler: { (modalResponse) -> Void in
                    if modalResponse == .alertFirstButtonReturn {
                        self.saveFileAs()
                    }
                })
            }
        }
    }
    
    
    func Cheat_Wavy_Navy_Victory() {
#if !DEBUGGER
        JUMP( 0x1528 ) // called when player clears the level
#endif
    }
    
    func Cheat_Wavy_Navy_Add_3_Ships() {
#if !DEBUGGER
        let ships = min( getMEM( 0x4746 ) + 3, 9 )
        setMEM( 0x4746, ships )
        m6502.A = ships
//        m6502.X = 0x10
        setMEM16(0x4728, 0x16F0) // cursor pos: 0x4728:x, 0x4729:y
        CALL( 0x1FDA ) // position and number needed? A:Number of ships X:0x10
#endif
    }

    func Cheat_Wavy_Navy_OtherCheats() {
#if !DEBUGGER
        // Replace STC / SBC $0x1 to NOPs...
        setMEM( 0x1E63, 0xEA )
        setMEM( 0x1E64, 0xEA )
        setMEM( 0x1E65, 0xEA )

//        // call to decease
//        setMEM( 0x1556, 0xEA );
//        setMEM( 0x1557, 0xEA );
//        setMEM( 0x1558, 0xEA );

//        setMEM( 0x15F3, 0xEA );
//        setMEM( 0x15F4, 0xEA );
//        setMEM( 0x15F5, 0xEA );

//        setMEM( 0x15EA, 0xEA );
//        setMEM( 0x15EB, 0xEA );

//        // no end
//        setMEM( 0x1515, 0xEA );
//        setMEM( 0x1516, 0xEA );

//        // no end
//        setMEM( 0x1537, 0xEA );
//        setMEM( 0x1538, 0xEA );
        
        // lose to win
        setMEM( 0x1545, 0xEA )
        setMEM( 0x1546, 0xEA )

//        var i : UInt16 = 0x15EA;
//        while i < 0x1608 {
//            setMEM( i, 0xEA )
//            i += 1
//        }
#endif
    }

    func Cheat_Wavy_Navy_Never_Lose()  -> NSControl.StateValue {
#if !DEBUGGER
        // Replace STC / SBC #$01 to NOPs...
//        setMEM( 0x1E63, 0xEA )
//        setMEM( 0x1E64, 0xEA )
//        setMEM( 0x1E65, 0xEA )

        if ( getMEM16(0x1E64) == 0x01E9 ) { // SBC #$01
            // Replace SBC #$01 to SBC #$00...
            setMEM( 0x1E65, 0 )
            return .on
        }
        else if ( getMEM16(0x1E64) == 0x00E9 ) { // SBC #$00
            // Replace SBC #$00 to SBC #$01...
            setMEM( 0x1E65, 1 )
            return .off
        }
        else {
            print("Not Wavy Navy!")
            return .off
        }
#else
        return .off
#endif
    }
    
    func Cheat_Wavy_Navy_Lose_To_Win() -> NSControl.StateValue {
#if !DEBUGGER
        if ( getMEM16(0x1545) == 0x09F0 ) { // BEQ $1550
            // lose to win
            setMEM16( 0x1545, 0xEAEA ) // NOP NOP
            return .on
        }
        else if ( getMEM16(0x1545) == 0xEAEA ) { // NOP NOP
            // lose to win
            setMEM16( 0x1545, 0x09F0 ) // BEQ $1550
            return .off
        }
        else {
            print("Not Wavy Navy!")
            return .off
        }
#else
        return .off
#endif
    }
    
    
    func Get_Hard_Hat_Mack() -> UInt8 {
#if !DEBUGGER
        return getMEM( 0x4EDF )
#else
        return 0
#endif
    }
    
    func Cheat_Hard_Hat_Mack(add : UInt8) -> UInt8 {
#if !DEBUGGER
        let ships = min( getMEM( 0x4EDF ) + add, 9 )
        setMEM( 0x4EDF, ships )
//        CALL( 0x1219 ) // starts from the beginning
        CALL( 0x1A2B ) // refresh Mack counter on screen
        
        return ships
#else
        return 0
#endif
    }

    
    func Cheat_Hard_Hat_Mack_Never_Lose() -> NSControl.StateValue {
#if !DEBUGGER
        setMEM( 0x0503, 0x18 )
        setMEM( 0x0504, 0x60 )
        
        setMEM( 0x50A5, 0xEA )
        setMEM( 0x50A6, 0xEA )
        setMEM( 0x50A7, 0xEA )
#endif
        return .on
    }
}


@_cdecl("woz_ask_to_save")
func woz_ask_to_save() {
    ViewController.shared?.saveFile()
}

