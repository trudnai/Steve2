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

class DebuggerViewController: NSViewController, NSTextFieldDelegate {
    static var shared : DebuggerViewController? = nil

    @IBOutlet var CPU_Display: DisplayView!
    @IBOutlet var Stack_Display: DisplayView!
    @IBOutlet var Mem1_Display: DisplayView!
    @IBOutlet var Disass_Scroll: DisplayScrollView!
    @IBOutlet var Disass_Display: DisplayView!

    //    required init?(coder: NSCoder) {
//        super.init(coder: coder)
//        DebuggerViewController.shared = self
//    }

    override func viewDidLoad() {
        super.viewDidLoad()
        self.preferredContentSize = NSMakeSize(self.view.frame.size.width, self.view.frame.size.height)
        DebuggerViewController.shared = self
    }

    
    override func viewDidAppear() {
        super.viewDidAppear()
        
//        // Update window title with the active TableView Title
//        self.parent?.view.window?.title = self.title!

        Update()

        if let debugger = DebuggerWindowController.current {
            debugger.PauseButtonUpdate(needUpdateMainToolbar: false)
        }
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


    func DisplayMemory() {
        var memory = ""
        for i : UInt16 in stride(from: 0x400, to: 0x4FF, by: 16) {
            memory += hexLine16(addr: i) + " " + textLine16(addr: i) + "\n"
        }

        DispatchQueue.main.async {
            self.Mem1_Display.string = memory
        }
    }


    func ASCII_to_Apple2(line : String) -> String {
        var converted = ""

        for chr in line {
            let c = Int(chr.asciiValue!) & 0x3F | 0x80
            converted.append(ViewController.charConvTbl[c])
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
    var disass_addr : UInt16 = 0
    let disass_addr_pre : UInt16 = 20
    let disass_addr_min_pre : UInt16 = 320 - 20
    var line_number = 0
    var line_number_at_PC = 0
    var line_number_cursor = 0
    let lines_to_disass = 300


    func get_scroll_line(view: DisplayView) -> Int {
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
        var scrollTo = view.visibleRect.origin
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
    func scroll_to(view: DisplayView, line: Int) {
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

    func highlight(view: DisplayView, line: Int, attr: [NSAttributedString.Key : Any]) {
        if let lineRange = getLineRange(inView: view, forLine: line) {
//            view.selectedRange = lineRange
//            view.scrollRangeToVisible(lineRange)
            if let oldLineRange = getLineRange(inView: view, forLine: line_number_cursor) {
                view.layoutManager?.removeTemporaryAttribute(NSAttributedString.Key.backgroundColor, forCharacterRange: oldLineRange)
                view.layoutManager?.removeTemporaryAttribute(NSAttributedString.Key.foregroundColor, forCharacterRange: oldLineRange)
            }
            view.layoutManager?.addTemporaryAttributes(attr, forCharacterRange: lineRange)
        }
    }


    func isMouseInView(view: NSView) -> Bool {
        if let window = view.window {
            return view.isMousePoint(window.mouseLocationOutsideOfEventStream, in: view.frame)
        }
        return false
    }


    override func mouseMoved(with event: NSEvent) {
        var location = event.locationInWindow
        let disass_frame = Disass_Scroll.superview?.frame
        let minX = disass_frame!.minX + Disass_Scroll.frame.minX
        let minY = disass_frame!.minY + Disass_Scroll.frame.minY
        let maxX = minX + Disass_Scroll.frame.width
        let maxY = minY + Disass_Scroll.frame.height

//        if isMouseInView(view: Disass_Scroll) {
        if location.x > minX && location.x < maxX
        && location.y > minY && location.y < maxY {
//            location.x -= Disass_Scroll.frame.origin.x
//            location.y -= Disass_Scroll.frame.origin.y
            location.x = maxX - location.x
            location.y = maxY - location.y + Disass_Display.visibleRect.origin.y
            //                print(location)

            let line = getLine(inView: Disass_Display, forY: location.y)
            highlight(view: Disass_Display, line: line_number_at_PC, attr: lineAttrAtPC)
            highlight(view: Disass_Display, line: line, attr: lineAttrAtSelected)
            line_number_cursor = line
        }
    }


    func DisplayDisassembly() {
        let m6502_saved = m6502
        var disass = ""

        line_number = 0
        line_number_at_PC = 0

        if m6502.PC > disass_addr && m6502.PC < disass_addr + disass_addr_max {
            m6502.PC = disass_addr
        }
        else {
            disass_addr = m6502.PC
            if m6502.PC >= disass_addr_min_pre {
                m6502.PC -= disass_addr_min_pre
            }

            // try to sync disassembly code
            let addr_min = disass_addr >= disass_addr_min ? disass_addr - disass_addr_min : disass_addr
            while m6502.PC < addr_min {
                m6502_Disass_1_Instr()
//                line_number += 1
            }

            // hopefully instruction address is in sync
            disass_addr = m6502.PC
        }

        // normal disassembly
        for _ in 1...lines_to_disass {
            // check if this is the current line before disassembling it (that will change PC...)
            let isCurrentLine = m6502.PC == m6502_saved.PC

            m6502_Disass_1_Instr()
            line_number += 1

            let line = ASCII_to_Apple2( line: String(cString: disassemblyLine( isCurrentLine )!) )

            if isCurrentLine {
//                line = invertLine(line: line)
                line_number_at_PC = line_number
            }

            disass += line + "\n"
        }

        DispatchQueue.main.async {
//            let isEmpty = self.Disass_Display.string.isEmpty
            self.Disass_Display.string = disass
            let currentScrollLine = self.get_scroll_line(view: self.Disass_Display) + 1
            if self.line_number_at_PC <= currentScrollLine || self.line_number_at_PC > currentScrollLine + 35 {
                self.scroll_to(view: self.Disass_Display, line: self.line_number_at_PC - 5)

                // at the beginning it takes a while to fill up the buffer -- maybe allocation issue?
                if currentScrollLine == 1 {
                    // so we need to scroll a bit later when the string is already populated
                    DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                        self.scroll_to(view: self.Disass_Display, line: self.line_number_at_PC - 5)
                    }
                }
            }
            self.highlight(view: self.Disass_Display, line: self.line_number_at_PC, attr: self.lineAttrAtPC)
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            // your code here
        }
        m6502 = m6502_saved
    }


    let UpdateSemaphore = DispatchSemaphore(value: 1)
    func Update() {
        UpdateSemaphore.wait()

        DisplayRegisters()
        DisplayStack()
        DisplayMemory()
        DisplayDisassembly()

        UpdateSemaphore.signal()
    }

}
