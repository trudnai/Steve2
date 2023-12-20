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
    @IBOutlet var Mem1_Addr_Nibbles: NSTextField!
    @IBOutlet var Mem1_Display: DisplayView!
    @IBOutlet var ZP_Display: DisplayView!
    @IBOutlet var IO_Display: DisplayView!
    @IBOutlet var Disass_Scroll: DisplayScrollView!
    @IBOutlet var Disass_Display: DisplayView!
    @IBOutlet weak var MemoryAddressField: NSTextField!
    @IBOutlet weak var MemoryTextField: NSTextField!
    @IBOutlet weak var DisassAddressField: NSTextField!
    @IBOutlet weak var DisassAddressPC: NSButton!
    @IBOutlet weak var DisassTextField: NSTextField!
    @IBOutlet weak var DisassHighlighter: NSTextField!
    @IBOutlet weak var DisassHightlighterContriant: NSLayoutConstraint!
    @IBOutlet weak var DisassCursor: NSTextField!
    @IBOutlet weak var DisassCursorContraint: NSLayoutConstraint!
    @IBOutlet weak var DisassBackground: NSTextField!


    let infoFont : NSFont = NSFont(name: "Print Char 21", size: 10.0)!
    let disassFont : NSFont = NSFont(name: "Print Char 21", size: 10.0)!
    let textColor : NSColor = NSColor.white
    let highlightColor : NSColor = NSColor.blue
    let infoTextParagraph : NSMutableParagraphStyle = NSMutableParagraphStyle()
    let disassTextParagraph : NSMutableParagraphStyle = NSMutableParagraphStyle()
    let infoTextAttribs : [NSAttributedString.Key : NSObject]
    let disassTextAttribs : [NSAttributedString.Key : NSObject]
    let infoHighlightAttribs : [NSAttributedString.Key : NSObject]
    let disassHighlightAttribs : [NSAttributedString.Key : NSObject]

    required init?(coder: NSCoder) {
//        textParagraph.lineSpacing = 10.0  /*this sets the space BETWEEN lines to 10points*/
//        textParagraph.maximumLineHeight = 12.0/*this sets the MAXIMUM height of the lines to 12points*/
        disassTextParagraph.lineHeightMultiple = 1.15
        disassTextAttribs = [
            NSAttributedString.Key.font: disassFont,
            NSAttributedString.Key.foregroundColor: textColor,
            NSAttributedString.Key.paragraphStyle: disassTextParagraph
        ]
        disassHighlightAttribs = [
            NSAttributedString.Key.font: disassFont,
            NSAttributedString.Key.foregroundColor: textColor,
            NSAttributedString.Key.backgroundColor: highlightColor,
            NSAttributedString.Key.paragraphStyle: disassTextParagraph
        ]
        infoTextParagraph.lineHeightMultiple = 1.15
        infoTextAttribs = [
            NSAttributedString.Key.font: disassFont,
            NSAttributedString.Key.foregroundColor: textColor,
            NSAttributedString.Key.paragraphStyle: infoTextParagraph
        ]
        infoHighlightAttribs = [
            NSAttributedString.Key.font: disassFont,
            NSAttributedString.Key.foregroundColor: textColor,
            NSAttributedString.Key.backgroundColor: highlightColor,
            NSAttributedString.Key.paragraphStyle: infoTextParagraph
        ]

        super.init(coder: coder)
        DebuggerViewController.shared = self
    }


    func disassDisplay(str : String) {
        let disassAttrString = NSAttributedString.init(string: String(str.dropLast()), attributes: disassTextAttribs)
        DisassTextField.attributedStringValue = disassAttrString
    }


    override func viewDidLoad() {
        super.viewDidLoad()
        self.preferredContentSize = NSMakeSize(self.view.frame.size.width, self.view.frame.size.height)

        // For the fake text view scroller
        // 64K RAM/2 as an average bytes / instruction
        Disass_Display.string = ""
        // for proper address scrolling
        var r = Disass_Display.frame
        r.size.height = 65535
        Disass_Display.frame = r
        
        
        CPU_Display.font = infoFont
        Stack_Display.font = infoFont
        Mem1_Addr_Nibbles.font = infoFont
        Mem1_Display.font = infoFont
        ZP_Display.font = infoFont
        IO_Display.font = infoFont

    }

    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        UpdateImmediately()

        if let debugger = DebuggerWindowController.shared {
            debugger.PauseButtonUpdate(needUpdateMainToolbar: false)
        }

        MemoryAddressField.formatter = HexDigitFormatter(maxLen: 4)
        DisassAddressField.formatter = HexDigitFormatter(maxLen: 4)
    }


    let maxAddr = Float(0xFFFF)

    /// Disassembly View Scroll changed
    func disassScroller(needScroll : Bool = false) {
        var scrollTo = Disass_Display.visibleRect.origin
        let scrollPos = Disass_Scroll.verticalScroller?.floatValue ?? 0
        let addr = scrollPos * maxAddr
        disass_addr_pc = addr < maxAddr ? UInt16(addr) : UInt16(maxAddr)

//        print("disassScroller ", "scrollTo", scrollTo, "scrollPos", scrollPos, " addr:", disass_addr_pc)

        if needScroll {
            scrollTo.y = Disass_Display.frame.height * CGFloat(scrollPos)
        }

        DisassAddressPC.state = .off
        DisplayDisassembly(scrollY: scrollTo.y)
    }


    func scrollEvent(location: NSPoint, scrollView: NSScrollView, deltaY: Float, action: () -> Void) {
        if location.x > scrollView.frame.minX
        && location.x < scrollView.frame.maxX
        && location.y > scrollView.frame.minY
        && location.y < scrollView.frame.maxY
        {
            if let documentView = scrollView.documentView {
                scrollView.verticalScroller?.floatValue += deltaY / Float(documentView.frame.height)
            }

            action()
        }
    }


    override func scrollWheel(with event: NSEvent) {
//        print("scrollWheel")
        super.scrollWheel(with: event)

        let location = event.locationInWindow

//        print("scrollWheel, location:", location,
//              Disass_Scroll.frame.minX,
//              Disass_Scroll.frame.maxX,
//              Disass_Scroll.frame.minY,
//              Disass_Scroll.frame.maxY
//              )

        let deltaY = Float(event.scrollingDeltaY)
//        print("scrollWheel:", event.scrollingDeltaY, deltaY)

        scrollEvent(location: location, scrollView: Disass_Scroll, deltaY: deltaY, action: {
            disassScroller(needScroll: true)
        })

    }


    /// Disassemby View had been Scrolled using the ScrollBar Y
    /// - Parameter sender: ScrollBar
    @IBAction func DisassScrolled(_ sender: NSScroller) {
//        print("DisassScrolled")
        disassScroller(needScroll: true)
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

STK REA WRI LNG BNK PG2 80S CXR
mne mne mne rom  2  off off off

KEY TXT MIX HGR 80C ALT
93  on  off off off off
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
        for i : UInt16 in stride(from: mem_1_addr, to: mem_1_addr + 0x21F, by: 16) {
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


    let disass_addr_min : UInt16 = 0 // 320
    let disass_addr_max : UInt16 = 50 // 512
    var disass_addr : UInt16 = 0 /// Address disassembled in the window
    var disass_addr_pc : UInt16 = 0 /// Address to disassemble
    let disass_addr_pre : UInt16 = 20
    let disass_addr_min_pre : UInt16 = 32 // how many bytes we need to start to disassemble before our target address
    var line_number = 0
    var scroll_line_number = 0
    var highlighted_line_number = 0
    var line_number_cursor = 0
    let lines_to_disass = 50


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
        let fontPointSize = CGFloat(10) // view.font!.pointSize
        let lineHeight = fontPointSize * lineSpacing

        let line = round(forY) / lineHeight

        return Int(line) + 1
    }


    func getLineRange_old(inView view: NSTextView, forLine: Int) -> NSRange? {
        let layoutManager = view.layoutManager!
        var numberOfLines = 0
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


    func getLineRange(_ lineRange : [LineRange_t], forLine: Int) -> NSRange? {
//        print("disassLineRange.count:", disassLineRange.count)
        if forLine >= 0 && forLine < lineRange.count {
            let disassRange = lineRange[forLine]
            return NSRange(location: disassRange.loc, length: disassRange.len)
        }

        // could not find line number
        return nil
    }


    let lineFromTopToMiddle = 0

    func scroll_to(view: NSTextView, line: Int) {
//        print("scroll_to", line)
        let lineSpacing = 1.5
        let fontPointSize = 10.0 // Disass_Display.font!.pointSize
        let lineHeight = fontPointSize * lineSpacing
        let line = line > 0 ? line - 1 : 0

        if getLineRange(disassLineRange, forLine: line + lineFromTopToMiddle) != nil {

            view.scroll( NSPoint(x: 0, y: Double(line) * lineHeight ) )
        }
    }


    func scroll_to_disass(addr: UInt16) {
        Disass_Scroll.verticalScroller?.floatValue = Float(addr) / Float(Disass_Display.frame.height)
    }


    let lineAttrAtSelected = [
        NSAttributedString.Key.backgroundColor: NSColor.lightGray,
        NSAttributedString.Key.foregroundColor: NSColor.black,
    ]

    let lineAttrAtPC = [
        NSAttributedString.Key.backgroundColor: NSColor.blue,
        NSAttributedString.Key.foregroundColor: NSColor.cyan,
    ]


    func remove_highlight(view : NSTextView, lineRange : NSRange) {
        DispatchQueue.main.async {
            if let layoutManager = view.layoutManager {
                layoutManager.removeTemporaryAttribute(NSAttributedString.Key.backgroundColor, forCharacterRange: lineRange)
                layoutManager.removeTemporaryAttribute(NSAttributedString.Key.foregroundColor, forCharacterRange: lineRange)
            }
        }
    }

    func remove_highlight(view : NSTextView, line : Int) {
        if line > 0 {
            if let lineRange = getLineRange(disassLineRange, forLine: line) {
                remove_highlight(view: view, lineRange: lineRange)
            }
        }
    }


    let lineHeight = CGFloat(14.96) // magic number... No idea why... 10pt font size + 1.5 lineSpacing

    func remove_highlight(view: NSTextField) {
        view.isHidden = true
    }


    func remove_highlight_attr(view: NSTextView) {
        if highlighted_line_number > 0 {
            if let lineRange = getLineRange(disassLineRange, forLine: highlighted_line_number) {
                if let layoutManager = view.layoutManager {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 0.03, execute: {
                        layoutManager.removeTemporaryAttribute(NSAttributedString.Key.backgroundColor, forCharacterRange: lineRange)
                        layoutManager.removeTemporaryAttribute(NSAttributedString.Key.foregroundColor, forCharacterRange: lineRange)
                    })
                }
            }
        }
        highlighted_line_number = 0
    }


    func highlight(view: NSTextField, constraint: NSLayoutConstraint, line: Int) {
        if line > 0 {
            let line = line > 0 ? line - 1 : 0

            if getLineRange(disassLineRange, forLine: line) != nil {
                constraint.constant = CGFloat(line) * lineHeight + 1
                view.isHidden = false
                // to make sure not to remove higlight
                return
            }
        }

        // remove old highlighted line
        remove_highlight(view: view)
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


    /// Highlight the entire line at the mouse location
    /// - Parameters:
    ///   - scrollView: ScrollView of the scrollable TextView
    ///   - display: The TextView itself
    ///   - mouseLocation: Mouse location locally inside the ScrollView
    func highlightCursor(scrollView : NSScrollView, mouseLocation : NSPoint) {
        let display = scrollView.documentView as! NSTextView

        // covering rectangle of the entire document
        let minX = scrollView.frame.minX
        let minY = scrollView.frame.minY
        let maxX = scrollView.frame.maxX
        let maxY = scrollView.frame.maxY - 4

        if mouseLocation.x > minX && mouseLocation.x < maxX
        && mouseLocation.y > minY && mouseLocation.y < maxY
        {
            let line = getLine(inView: display, forY: mouseLocation.y)
            highlight(view: DisassCursor, constraint: DisassCursorContraint, line: line)
            line_number_cursor = line
        }
    }

    let normalBackground = NSColor(red: 38/255, green: 38/255, blue: 38/255, alpha: 1)
    let selectedBackground = NSColor(red: 38/255, green: 38/255, blue: 64/255, alpha: 1.0)

    func unselectViews() {
        DisassBackground.backgroundColor = normalBackground
    }

    func selectView(view: NSTextView) {
        unselectViews()
        view.backgroundColor = normalBackground
    }

    let leftSideSize = CGFloat(30)

    // select disassembly line
    override func mouseDown(with event: NSEvent) {
        unselectViews()

        let location = convertMouseCoordinates(scrollView: Disass_Scroll, display: Disass_Display, mouseLocation: event.locationInWindow)

        if location.x < leftSideSize {
            let line = getLine(inView: Disass_Display, forY: location.y)
            let addr = getAddr(forLine: line)

            if m6502_dbg_bp_exists(breakpoints, addr) {
                m6502_dbg_bp_del(breakpoints, addr)
            }
            else {
                m6502_dbg_bp_add(breakpoints, addr)
            }

            // force regenerate disassembly
            disass_addr = 0xFFFF
            DisplayDisassembly(scrollY: Disass_Display.visibleRect.origin.y)
        }
        else {
            highlightCursor(scrollView: Disass_Scroll, mouseLocation: location)
        }
    }


    // context menu
    override func rightMouseDown(with event: NSEvent) {
        let location = event.locationInWindow
        highlightCursor(scrollView: Disass_Scroll, mouseLocation: location)
    }


    var addr_line = [UInt16 : Int]()

    func getLine(forAddr: UInt16) -> Int {
        return addr_line[forAddr] ?? -1
    }

    func getAddr(forLine: Int) -> UInt16 {
        return addr_line.first(where: { $1 == forLine })?.key ?? 0
    }


    struct LineRange_t {
        var loc : Int
        var len : Int
    }

    var disassLineRange = [LineRange_t]()


    func TrunDisassAddressPC(_ on: NSControl.StateValue = .on) {
        if let disassAddressPC = DisassAddressPC {
            disassAddressPC.state = on
        }
    }


    var isCurrentLine = false

    var disass = ""
    var loc = 0

    func AddDisassLine() {
        let line = ASCII_to_Apple2( line: String(cString: disassemblyLine()!) )
        let len = disassLineLength + 1
        let lineRange = LineRange_t(loc: loc, len: len)
        disassLineRange.append(lineRange)
        loc += len

        disass += line + "\n"
    }


    func DisplayDisassembly( scrollY : CGFloat = -1 ) {
        disass = ""
        loc = 0
        isCurrentLine = false
        highlighted_line_number = -1 // getLine(forAddr: m6502.PC)

        if cpuState == cpuState_running {
            remove_highlight(view: DisassHighlighter)
        }

        line_number = 0

        // TODO: Also check if memory area updated!

        let addrpc = DisassAddressPC == nil || DisassAddressPC.state == .on
        if addrpc {
            disass_addr_pc = m6502.PC
        }

        let need_scroll = scrollY > 0 || disass_addr_pc < disass_addr || UInt(disass_addr_pc) > UInt(disass_addr) + UInt(disass_addr_max)

        scroll_line_number = getLine(forAddr: disass_addr_pc)

        disassLineRange.removeAll()
        ViewController.shared?.UpdateSemaphore.wait()

        let m6502_saved = m6502

        if !addrpc {
            m6502.PC = disass_addr_pc
        }

        addr_line.removeAll()

        // de we need to scroll or prell at the same location?
        if need_scroll {
            disass_addr = m6502.PC
        }
        else {
            m6502.PC = disass_addr
        }

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

        // Scroll by address is needed only when address is NOT calculated from scroll position...
        if scrollY < 0 {
            scroll_to_disass(addr: disass_addr)
        }

        // normal disassembly
        for _ in 1...lines_to_disass {
            // check if this is the current line before disassembling it (that will change PC...)
            line_number += 1
            addr_line.updateValue(line_number, forKey: m6502.PC)

            isCurrentLine = m6502.PC == m6502_saved.PC
            if isCurrentLine {
//                line = invertLine(line: line)
                highlighted_line_number = line_number
            }

            if m6502.PC == disass_addr {
                scroll_line_number = line_number
            }

            m6502_Disass_1_Instr()
            AddDisassLine()
        }

        m6502 = m6502_saved
        ViewController.shared?.UpdateSemaphore.signal()

        DispatchQueue.main.async {
            self.disassDisplay(str: self.disass)
            self.highlight(view: self.DisassHighlighter, constraint: self.DisassHightlighterContriant, line: self.highlighted_line_number)
        }
    }


    func UpdateImmediately() {
        DispatchQueue.main.async {
            self.DisplayRegisters()
            self.DisplayStack()
            self.DisplayMemory()
            self.DisplayDisassembly()
        }
    }


    let UpdateSemaphore = DispatchSemaphore(value: 1)
    func Update() {
        if self.UpdateSemaphore.wait(timeout: .now()) == .success {
            if Disass_Display != nil {
                self.UpdateImmediately()
            }
            self.UpdateSemaphore.signal()
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
//        Disass_Display.scroll(NSPoint(x: 0, y: Int(disass_addr_pc)))

//        scroll_to_disass(addr: disass_addr_pc)

        DisplayDisassembly()
    }

}
