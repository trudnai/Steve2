//
//  PreferencesViewController.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 6/4/20.
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

class DebuggerViewController: NSViewController {
    static var shared : DebuggerViewController? = nil

    @IBOutlet var CPU_Display: DisplayView!
    @IBOutlet var Stack_Display: DisplayView!
    @IBOutlet var Mem1_Display: DisplayView!
    @IBOutlet var Disass_Scroll: DisplayScrollView!
    @IBOutlet var Disass_Display: DisplayView!
    @IBOutlet weak var MemoryAddressField: NSTextField!
    @IBOutlet weak var DisassAddressField: NSTextField!
    @IBOutlet weak var DisassAddressPC: NSButton!


    required init?(coder: NSCoder) {
        super.init(coder: coder)
        DebuggerViewController.shared = self
    }


    override func viewDidLoad() {
        super.viewDidLoad()
        self.preferredContentSize = NSMakeSize(self.view.frame.size.width, self.view.frame.size.height)
    }

    
    override func viewDidAppear() {
        super.viewDidAppear()
        
//        // Update window title with the active TableView Title
//        self.parent?.view.window?.title = self.title!

        UpdateImmediately()

        if let debugger = DebuggerWindowController.shared {
            debugger.PauseButtonUpdate(needUpdateMainToolbar: false)
        }

        MemoryAddressField.formatter = HexDigitFormatter(maxLen: 4)
        DisassAddressField.formatter = HexDigitFormatter(maxLen: 4)
    }


    override func scrollWheel(with event: NSEvent) {
        super.scrollWheel(with: event)

//        if view.window?.firstResponder?.textView?.delegate === Stack_Display {
//        print("scroll deltaY", event.deltaY, event.scrollingDeltaY)
//        Stack_Display.scroll(Stack_Display.enclosingScrollView!.visibleRect, by: NSSize(width: 0, height: event.scrollingDeltaY) )
//        }

        var scrollTo = Disass_Display.visibleRect.origin
        let lineSpacing = CGFloat(1.5)
        let lineHeight = Disass_Display.font!.pointSize * lineSpacing
//        print("lineHeight:", lineHeight, "fontSize:", Stack_Display.font?.pointSize)

        let y1 = round( (scrollTo.y + round(event.scrollingDeltaY) * lineHeight) / lineHeight) * lineHeight
        let y2 = round( scrollTo.y / lineHeight + event.scrollingDeltaY ) * lineHeight

        if y1 != y2 {
            print("NOT EQ", y1, y2)
        }

        scrollTo.y = y1

        Disass_Display.scroll(scrollTo)
    }


    func bin(n : UInt8, sp : String = "", mid : String = " ") -> String {
        var str = ""
        var n = n

        for i in 1...8 {
            if n & 0x80 == 0 {
                str += "0"
            }
            else {
                str += "1"
            }

            str += sp
            if i == 4 {
                str += mid
            }

            n <<= 1
        }

        return str
    }

    func DisplayRegisters() {

        let registers = String(format:"""
A: %02X    %3u      %@
X: %02X    %3u      %@
Y: %02X    %3u      %@

SP: %02X
PC: %04X

N V - B D I Z C
%d %d %d %d %d %d %d %d
""",

            m6502.A, m6502.A, bin(n: m6502.A),
            m6502.X, m6502.X, bin(n: m6502.X),
            m6502.Y, m6502.Y, bin(n: m6502.Y),
            m6502.SP,
            m6502.PC,
            m6502.N != 0, m6502.V != 0, m6502.res != 0, m6502.B != 0, m6502.D != 0, m6502.I != 0, m6502.Z != 0, m6502.C != 0
        )

        DispatchQueue.main.async {
            self.CPU_Display.string = registers
        }
    }


    func DisplayStack() {
        var stack = ""
        for i : UInt16 in (0x100...0x1FF).reversed() {
            stack += String(format:"%03X: %02X\n", i, getMEM(i))
        }

        DispatchQueue.main.async {
            self.Stack_Display.string = stack
        }
    }

    func hexLine16(addr : UInt16) -> String {
        var line = String(format: "%04X: ", addr)

        for i : UInt16 in 0...15 {
            line += String(format: "%02X ", getMEM(addr + i))
        }

        return line
    }


    let txtClear = [Character](repeating: " ", count: 16)
    func textLine16(addr : UInt16) -> String {
        var unicodeTextArray = NSArray(array: txtClear, copyItems: true) as! [Character]

        // render the rest of the text screen
        for i in 0 ... 15 {
            let byte = getMEM(addr + UInt16(i))
            let idx = Int(byte);
            let chr = ViewController.charConvTbl[idx]

            unicodeTextArray[i] = chr
        }

        return String(unicodeTextArray)
    }


    var mem_1_addr : UInt16 = 0x400

    func DisplayMemory() {
        var memory = ""
        for i : UInt16 in stride(from: mem_1_addr, to: mem_1_addr + 0xFF, by: 16) {
            memory += hexLine16(addr: i) + " " + textLine16(addr: i) + "\n"
        }

        DispatchQueue.main.async {
            self.Mem1_Display.string = memory
        }
    }


    func ASCII_to_Apple2(line : String) -> String {
        var converted = ""

        for chr in line {
            // make C character NORMAL Apple ][ character
            let c = Int(chr.asciiValue!) & 0x3F | 0x80

            // breakpoint marker
            if c == 0xAA { // '*'
//                converted.append("\u{E895}") // big dot (8x8)
//                converted.append("\u{ED3C}") // big dot2 (8x8)
                converted.append("\u{E095}") // right arrow
//                converted.append("\u{E09B}") // diamond
//                converted.append("\u{E080}") // closed apple
//                converted.append("\u{E081}") // open apple
//                converted.append("\u{E185}") // checkmark
            }
            // normal character
            else {
                converted.append(ViewController.charConvTbl[c])
            }
        }

        return converted
    }


    func invertLine(line : String) -> String {
        var converted = ""

        for chr in line {
            let c = Int(chr.asciiValue!) & 0x3F
            converted.append(ViewController.charConvTbl[c])
        }

        return converted
    }


    let disass_addr_min : UInt16 = 320
    let disass_addr_max : UInt16 = 512
    var disass_addr : UInt16 = 0 /// Address disassembled in the window
    var disass_addr_pc : UInt16 = 0 /// Address to disassemble
    let disass_addr_pre : UInt16 = 20
    let disass_addr_min_pre : UInt16 = 320 - 20
    var line_number = 0
    var scroll_line_number = 0
    var highlighted_line_number = 0
    var line_number_cursor = 0
    let lines_to_disass = 300


    func get_scroll_line(view: NSTextView) -> Int {
        let scrollPos = view.visibleRect.origin.y
        let lineSpacing = CGFloat(1.5)
        let lineHeight = view.font!.pointSize * lineSpacing

        return Int(scrollPos / lineHeight)
    }


    func scroll_to_old(view: DisplayView, line: Int) {
        var scrollTo = view.visibleRect.origin
        let lineSpacing = CGFloat(1.5)
        let lineHeight = view.font!.pointSize * lineSpacing

        scrollTo.y = CGFloat(line) * lineHeight

        view.scroll(scrollTo)
    }


    func getLine(inView view: NSTextView, forY: CGFloat) -> Int {
//        var scrollTo = view.visibleRect.origin
        let lineSpacing = CGFloat(1.5)
        let lineHeight = view.font!.pointSize * lineSpacing

        let line = round(forY) / lineHeight

        return Int(line) + 1
    }


    func getLineRange(inView view: NSTextView, forLine: Int) -> NSRange? {
        let layoutManager = view.layoutManager!
        var numberOfLines = 1
        let numberOfGlyphs = layoutManager.numberOfGlyphs
        var lineRange = NSRange()
        var indexOfGlyph = 0

        while indexOfGlyph < numberOfGlyphs {
            layoutManager.lineFragmentRect(forGlyphAt: indexOfGlyph, effectiveRange: &lineRange, withoutAdditionalLayout: false)

            // check if we've found our line number
            if numberOfLines == forLine {
                return lineRange
            }

            indexOfGlyph = NSMaxRange(lineRange)
            numberOfLines += 1
        }

        // could not find line number
        return nil
    }


    let lineFromTopToMiddle = 15
    func scroll_to(view: NSTextView, line: Int) {
        if let lineRange = getLineRange(inView: view, forLine: line + lineFromTopToMiddle) {
            view.scrollRangeToVisible(lineRange)
        }
    }


    let lineAttrAtSelected = [
        NSAttributedString.Key.backgroundColor: NSColor.lightGray,
        NSAttributedString.Key.foregroundColor: NSColor.black,
    ]

    let lineAttrAtPC = [
        NSAttributedString.Key.backgroundColor: NSColor.blue,
        NSAttributedString.Key.foregroundColor: NSColor.cyan,
    ]


    func remove_highlight(view: NSTextView, line: Int) {
        if line > 0 {
            if let lineRange = getLineRange(inView: view, forLine: line) {
                view.layoutManager?.removeTemporaryAttribute(NSAttributedString.Key.backgroundColor, forCharacterRange: lineRange)
                view.layoutManager?.removeTemporaryAttribute(NSAttributedString.Key.foregroundColor, forCharacterRange: lineRange)
            }
        }
    }


    func highlight(view: NSTextView, line: Int, attr: [NSAttributedString.Key : Any]) {
        if line > 0 {
            // remove old highlighted line
            remove_highlight(view: view, line: line)
            if let lineRange = getLineRange(inView: view, forLine: line) {
    //            view.selectedRange = lineRange
    //            view.scrollRangeToVisible(lineRange)
//                remove_highlight(view: view, line: line_number_cursor)
//                line_number_cursor = 0
                view.layoutManager?.addTemporaryAttributes(attr, forCharacterRange: lineRange)
            }
        }
    }


    func isMouseInView(view: NSView) -> Bool {
        if let window = view.window {
            return view.isMousePoint(window.mouseLocationOutsideOfEventStream, in: view.frame)
        }
        return false
    }


    let textViewMouseYOffset = CGFloat(-4.0)
    func convertMouseCoordinates(scrollView : NSView, display : NSTextView, mouseLocation : NSPoint) -> NSPoint {
        var location = mouseLocation
        let parent_frame = scrollView.superview?.frame

//        let minX = parent_frame!.minX + scrollView.frame.minX
        let minY = parent_frame!.minY + scrollView.frame.minY
//        let maxX = minX + scrollView.frame.width
        let maxY = minY + scrollView.frame.height

//        location.x = maxX - location.x
        location.y = maxY - location.y + display.visibleRect.origin.y + textViewMouseYOffset

        return location
    }


    func highlightCursor(scrollView : NSView, display : NSTextView, mouseLocation : NSPoint) {
        var location = mouseLocation
        let parent_frame = scrollView.superview?.frame

        let minX = parent_frame!.minX + scrollView.frame.minX
        let minY = parent_frame!.minY + scrollView.frame.minY
        let maxX = minX + scrollView.frame.width
        let maxY = minY + scrollView.frame.height

        if location.x > minX && location.x < maxX
        && location.y > minY && location.y < maxY {
//            location.x = maxX - location.x
            location.y = maxY - location.y + display.visibleRect.origin.y

            let line = getLine(inView: display, forY: location.y)
            highlight(view: display, line: highlighted_line_number, attr: lineAttrAtPC)
            remove_highlight(view: display, line: line_number_cursor)
            highlight(view: display, line: line, attr: lineAttrAtSelected)
            line_number_cursor = line
        }
    }


    // select disassembly line
    override func mouseDown(with event: NSEvent) {
        let location = convertMouseCoordinates(scrollView: Disass_Scroll, display: Disass_Display, mouseLocation: event.locationInWindow)

        if location.x < 30 {
            let line = getLine(inView: Disass_Display, forY: location.y)
            let addr = getAddr(forLine: line)

            if m6502_dbg_bp_is_exists(addr) {
                m6502_dbg_bp_del(addr)
            }
            else {
                m6502_dbg_bp_add(addr)
            }

            // force regenerate disassembly
            disass_addr = 0xFFFF
            DisplayDisassembly(scrollY: Disass_Display.visibleRect.origin.y)
        }
        else {
            highlightCursor(scrollView: Disass_Scroll, display: Disass_Display, mouseLocation: location)
        }
    }


    // context menu
    override func rightMouseDown(with event: NSEvent) {
        let location = event.locationInWindow
        highlightCursor(scrollView: Disass_Scroll, display: Disass_Display, mouseLocation: location)
    }


    var addr_line = [UInt16 : Int]()

    func getLine(forAddr: UInt16) -> Int {
        return addr_line[forAddr] ?? 0
    }

    func getAddr(forLine: Int) -> UInt16 {
        return addr_line.first(where: { $1 == forLine })?.key ?? 0
    }


    func DisplayDisassembly( scrollY : CGFloat = -1 ) {
        var disass = ""

        line_number = 0

        let highlighted = self.highlighted_line_number

        DispatchQueue.main.async {
            self.remove_highlight(view: self.Disass_Display, line: highlighted)
        }

        // TODO: Also check if memory area updated!

//        DispatchQueue.main.sync {
        let addrpc = DisassAddressPC == nil || DisassAddressPC.state == .on
        if addrpc {
            disass_addr_pc = m6502.PC
        }
//        }
        var need_disass = disass_addr_pc <= disass_addr || disass_addr_pc > disass_addr + disass_addr_max
        scroll_line_number = getLine(forAddr: disass_addr_pc)
        highlighted_line_number = getLine(forAddr: m6502.PC)

//        if disass_addr_pc > disass_addr && disass_addr_pc < disass_addr + disass_addr_max {
        if scroll_line_number == 0 || need_disass {
            ViewController.shared?.UpdateSemaphore.wait()

            let m6502_saved = m6502

            if !addrpc {
                m6502.PC = disass_addr_pc
            }

            need_disass = true
            addr_line.removeAll()

            disass_addr = m6502.PC
            if m6502.PC >= disass_addr_min_pre {
                m6502.PC -= disass_addr_min_pre
            }

            // try to sync disassembly code
            let addr_min = disass_addr >= disass_addr_min ? disass_addr - disass_addr_min : disass_addr
            while m6502.PC < addr_min {
                m6502_Disass_1_Instr()
            }

            // hopefully instruction address is in sync
            disass_addr = m6502.PC

            // normal disassembly
            for _ in 1...lines_to_disass {
                // check if this is the current line before disassembling it (that will change PC...)
                line_number += 1
                addr_line.updateValue(line_number, forKey: m6502.PC)

                let isCurrentLine = m6502.PC == m6502_saved.PC

                m6502_Disass_1_Instr()

                let line = ASCII_to_Apple2( line: String(cString: disassemblyLine( isCurrentLine )!) )

                if isCurrentLine {
                    //                line = invertLine(line: line)
                    highlighted_line_number = line_number
                }
                if m6502.PC == disass_addr_pc {
                    scroll_line_number = line_number
                }

                disass += line + "\n"
            }

            m6502 = m6502_saved
            ViewController.shared?.UpdateSemaphore.signal()
        }

        DispatchQueue.main.async {
//            let isEmpty = self.Disass_Display.string.isEmpty
            if need_disass {
                self.Disass_Display.string = disass
            }

            let currentScrollLine = self.get_scroll_line(view: self.Disass_Display) + 1
            if self.highlighted_line_number <= currentScrollLine || self.highlighted_line_number > currentScrollLine + 35 {

                if scrollY < 0 {
                    self.scroll_to(view: self.Disass_Display, line: self.scroll_line_number - 5)

                    // at the beginning it takes a while to fill up the buffer -- maybe allocation issue?
//                    if currentScrollLine == 1 {
//                        // so we need to scroll a bit later when the string is already populated
//    //                    DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
//                            self.scroll_to(view: self.Disass_Display, line: self.line_number_at_PC - 5)
//    //                    }
//                    }
                }
                else {
                    // caller wants a specific scroll location...
                    self.Disass_Display.scroll(NSPoint(x: 0, y: scrollY))
                }
            }
            self.highlight(view: self.Disass_Display, line: self.highlighted_line_number, attr: self.lineAttrAtPC)
        }
//        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            // your code here
//        }
    }


    func UpdateImmediately() {
        DisplayRegisters()
        DisplayStack()
        DisplayMemory()
        DisplayDisassembly()
    }


    let UpdateSemaphore = DispatchSemaphore(value: 1)
    func Update() {
        DispatchQueue.global().async {
            if self.UpdateSemaphore.wait(timeout: .now()) == .success {
                self.UpdateImmediately()
                self.UpdateSemaphore.signal()
            }
        }
    }


    @IBAction func MemoryAddressEntered(_ sender: NSTextFieldCell) {
        NSLog("MemoryAddressEntered %@", sender.stringValue)
        mem_1_addr = UInt16(sender.stringValue.hexValue())
        DisplayMemory()
    }

    @IBAction func DisassAddressEntered(_ sender: NSTextFieldCell) {
        NSLog("DisassAddressEntered %@", sender.stringValue)
//        sender.stringValue = "4321" // MemoryAddressField.stringValue
        DisassAddressPC.state = .off
        disass_addr_pc = UInt16(sender.stringValue.hexValue())
        DisplayDisassembly()
    }

}
