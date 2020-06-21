//
//  HiRes.swift
//  A2Mac
//
//  Created by Tamas Rudnai on 9/19/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

//import Foundation
import AppKit

class HiRes: NSView {
    
    static let PageSize  = 0x2000
    static let Page1Addr = 0x2000
    static let Page2Addr = 0x4000
    
    static let PixelWidth  = 280
    static let PixelMixedHeight = 160
    static let PixelHeight = 192
    static let MixedTextHeight = 4
    static let MixedHeight = 160
    static let blockRows = 24
    static let blockCols = 40
    static let blockWidth = PixelWidth / blockCols
    static let blockHeight = PixelHeight / blockRows

    let HiResBuffer1 = UnsafeRawBufferPointer(start: MEM + Page1Addr, count: PageSize * 2)
    let HiResBuffer2 = UnsafeRawBufferPointer(start: MEM + Page2Addr, count: PageSize * 2)
    var HiResBufferPointer = UnsafeRawBufferPointer(start: MEM + Page1Addr, count: PageSize * 2)
    
    let HiResRawPointer = UnsafeRawPointer(RAM + Page1Addr)

    // holds the starting addresses for each lines minus the screen page starting address
    var HiResLineAddrTbl = [Int](repeating: 0, count: PixelHeight)

    func initHiResLineAddresses() {
        var i = 0
        for x in stride(from: 0, through: 0x50, by: 0x28) {
            for y in stride(from: 0, through: 0x380, by: 0x80) {
                for z in stride(from: 0, through: 0x1C00, by: 0x400) {
                    HiResLineAddrTbl[i] = x + y + z
                    i += 1
                }
            }
        }
    }
    
    
    var HiResSubView = [[NSView]]()
    
    func createHiRes() {
        for y in 0 ..< HiRes.blockRows {
            HiResSubView.append([NSView]())
            for x in 0 ..< HiRes.blockCols {
                let blockView = NSView(frame: NSRect(x: x * HiRes.blockWidth, y: y * 8, width: HiRes.blockWidth, height: 8))
                HiResSubView[y].append(blockView)
                self.addSubview(blockView)
            }
        }
    }
    
    
    func clearScreen() {
        HiRes.context?.clear( CGRect(x: 0, y: 0, width: frame.width, height: frame.height) )
        needsDisplay = true
    }


    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initHiResLineAddresses()
        clearScreen()

//        currentContext?.setShouldAntialias(false)
//        currentContext?.interpolationQuality = CGInterpolationQuality.none
        
//        let scaleSizeW = Double((frame.size).width) / Double(HiRes.PixelWidth)
//        let scaleSizeH = Double((frame.size).height) / Double(HiRes.PixelHeight)

//        let scaleSizeW = 4
//        let scaleSizeH = 4
//        scaleUnitSquare(to: NSSize(width: scaleSizeW, height: scaleSizeH))
        
        // create smaller box views for draw optimization
        createHiRes()
        
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
        
    static func createBitmapContext(pixelsWide: Int, _ pixelsHigh: Int) -> CGContext? {
        let bytesPerPixel = 4
        let bytesPerRow = bytesPerPixel * pixelsWide
        
        let byteCount = (bytesPerRow * pixelsHigh)
        
//        guard let colorSpace = CGColorSpace(name: CGColorSpace.linearSRGB) else { return nil }
//        guard let colorSpace = CGColorSpace(name: CGColorSpace.genericRGBLinear) else { return nil }
        guard let colorSpace = CGColorSpace(name: CGColorSpace.sRGB) else { return nil }

        let pixels = UnsafeMutablePointer<CUnsignedChar>.allocate(capacity: byteCount)

        let bitmapInfo = CGImageAlphaInfo.premultipliedFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue
        
        let context = CGContext(
            data: pixels,
            width: pixelsWide,
            height: pixelsHigh,
            bitsPerComponent: 8,
            bytesPerRow: bytesPerRow,
            space: colorSpace,
            bitmapInfo: bitmapInfo)
        
        return context
    }
    

    private var currentContext : CGContext? {
        get {
            if #available(OSX 10.10, *) {
                return NSGraphicsContext.current?.cgContext
            } else if let contextPointer = NSGraphicsContext.current?.graphicsPort {
                let context: CGContext = Unmanaged.fromOpaque(contextPointer).takeUnretainedValue()
                return context
            }
            
            return nil
        }
    }

    static let ScreenBitmapSize = (PixelWidth * PixelHeight * 4)
    static let context = createBitmapContext(pixelsWide: PixelWidth, PixelHeight)
    static let pixels = UnsafeMutableRawBufferPointer(start: context?.data, count: ScreenBitmapSize)
    static var pixelsSRGB = pixels.bindMemory(to: UInt32.self)
    
    let R = 2
    let G = 1
    let B = 0
    let A = 3
    
    var blockChanged = [Bool](repeating: false, count: HiRes.blockRows * HiRes.blockCols)
    var shadowScreen = [Int](repeating: 0, count: PageSize)

    var was = 0;
    
    
    //    let color_black     : UInt32 = 0x00000000;
    //    let color_white     : UInt32 = 0xEEEEEEEE;
    //    let color_purple    : UInt32 = 0xFFBB11EE;
    //    let color_green     : UInt32 = 0xFF0BBB11;
    //    let color_blue      : UInt32 = 0xFF1155FF;
    //    let color_orange    : UInt32 = 0xFFEE2211;
    
    // HiRes Colors for the SRGB color space
    let color_black     : UInt32 = 0x00000000;
    let color_white     : UInt32 = 0xFFEEEEEE;
    let color_purple    : UInt32 = 0xFFDD55FF;
    let color_green     : UInt32 = 0xFF2BD84A;
    let color_blue      : UInt32 = 0xFF5599FF;
    let color_orange    : UInt32 = 0xFFFF6302;
    
    // for debugging only:
    let color_turquis   : UInt32 = 0xFF11BBBB;
    let color_yellow    : UInt32 = 0xFFBBBB11;
    
    
    func refreshChanged( blockSize : Int ) {
        // refresh changed block only
        
        let screenBlockMargin = 12 / blockSize
        
        let blockScreenWidth = Int(frame.width) / HiRes.blockCols * blockSize
        let blockScreenHeigth = Int(frame.height) / HiRes.blockRows
        
        for blockVertIdx in 0 ..< HiRes.blockRows {
            for blockHorIdx in 0 ..< HiRes.blockCols / blockSize {
                if blockChanged[ blockVertIdx * HiRes.blockCols / blockSize + blockHorIdx ] {
                    // refresh the entire screen
                    let boundingBox = CGRect(
                        x: blockHorIdx * blockScreenWidth - screenBlockMargin,
                        y: Int(frame.height) - blockVertIdx * blockScreenHeigth - blockScreenHeigth - screenBlockMargin,
                        width: blockScreenWidth + screenBlockMargin * blockSize,
                        height: blockScreenHeigth + screenBlockMargin * blockSize)
                    
                    self.setNeedsDisplay( boundingBox )
                }
            }
        }
    }
    
    
    func UpdateMono() {
        blockChanged = [Bool](repeating: false, count: HiRes.blockRows * HiRes.blockCols)

        var pixelAddr = 0
        
        var y = 0
        
        for lineAddr in HiResLineAddrTbl {
            let blockVertIdx = y / HiRes.blockHeight * HiRes.blockCols

            for blockHorIdx in 0..<HiRes.blockCols {
                let block = Int(HiResBufferPointer[ Int(lineAddr + blockHorIdx) ])
                let screenIdx = y * HiRes.blockCols + blockHorIdx

                // get all changed blocks
                blockChanged[ blockVertIdx + blockHorIdx ] = blockChanged[ blockVertIdx + blockHorIdx ] || shadowScreen[ screenIdx ] != block
                shadowScreen[ screenIdx ] = block

                for bit in stride(from: 0, through: 6, by: 1) {
                    let bitMask = 1 << bit
                    if (block & bitMask) != 0 {
                        HiRes.pixelsSRGB[pixelAddr] = color_green;
                    }
                    else {
                        HiRes.pixelsSRGB[pixelAddr] = color_black;
                    }

                    pixelAddr += 1
                }
            }
            
            y += 1
        }
        
        refreshChanged(blockSize: 1)
    }
        
    
    func colorPixel ( pixelAddr : Int, pixel : Int, prev : Int ) {
        let colorAddr = pixelAddr / 4
        
        switch ( pixel ) {
        case 1: // purple (bits are in reverse!)
            HiRes.pixelsSRGB[colorAddr] = color_purple;
//            HiRes.pixelsSRGB[colorAddr + 1] = color_purple
            if  (colorAddr >= 1) && (prev != 0x03) && (prev != 0x07) && (prev != 0x00) && (prev != 0x04) {
                HiRes.pixelsSRGB[colorAddr - 1] = color_purple
            }
            
        case 2: // green
            // reducing color bleeding
            if (colorAddr > 0) && (HiRes.pixelsSRGB[colorAddr - 1] != color_black) {
                HiRes.pixelsSRGB[colorAddr] = color_green
            }
            HiRes.pixelsSRGB[colorAddr + 1] = color_green

        case 3: // white 1
//            if ( colorAddr >= 2 ) && ( HiRes.pixelsSRGB[colorAddr - 2] != color_black ) {
//                HiRes.pixelsSRGB[colorAddr - 1] = HiRes.pixelsSRGB[colorAddr - 2]
//            }
//            if (colorAddr >= 1) {
//                HiRes.pixelsSRGB[colorAddr - 1] = color_yellow
//            }
            HiRes.pixelsSRGB[colorAddr] = color_white
            HiRes.pixelsSRGB[colorAddr + 1] = color_white

        case 5: // blue
            HiRes.pixelsSRGB[colorAddr] = color_blue
            if  (colorAddr >= 1) && (prev != 0x00) && (prev != 0x04) {
                HiRes.pixelsSRGB[colorAddr - 1] = color_blue
            }

        case 6: // orange
            // reducing color bleeding
            if (colorAddr > 0) && (HiRes.pixelsSRGB[colorAddr - 1] != color_black) {
                HiRes.pixelsSRGB[colorAddr] = color_orange
            }
            HiRes.pixelsSRGB[colorAddr + 1] = color_orange

        case 7: // white 2
            HiRes.pixelsSRGB[colorAddr] = color_white
            HiRes.pixelsSRGB[colorAddr + 1] = color_white

        default: // 0x00 (black 1), 0x04 (black 2)
            HiRes.pixelsSRGB[colorAddr] = color_black
            HiRes.pixelsSRGB[colorAddr + 1] = color_black
            break
        }

        // white adjustment
        if ( (prev & 2) == 2 ) && ( (pixel & 1) == 1 ) {
            HiRes.pixelsSRGB[colorAddr] = color_white
            if (colorAddr >= 1) {
                HiRes.pixelsSRGB[colorAddr - 1] = color_white
            }
            
            // TODO: Need better check if extra green was created
            if (colorAddr >= 2) && (HiRes.pixelsSRGB[colorAddr - 2] == color_green ) {
                if (colorAddr < 3) || (HiRes.pixelsSRGB[colorAddr - 3] != color_green) {
                    HiRes.pixelsSRGB[colorAddr - 2] = color_black
                }
            }
        }

//        // green adjustment -- followed by white
//        if (colorAddr >= 1) && (prev == 0x03) && (HiRes.pixelsSRGB[colorAddr - 1] = color_green) {
//            HiRes.pixelsSRGB[colorAddr - 1] = color_green
//        }

        // purple adjustment -- followed by white
        if (prev == 0x01) && (
            (pixel == 0x01) ||
            (pixel == 0x03) || (pixel == 0x07)  // white
        ) {
            // was the previous purple pixel promoted to white or is it still purple?
            if (colorAddr >= 2) && ( HiRes.pixelsSRGB[colorAddr - 2] == color_purple ) {
                HiRes.pixelsSRGB[colorAddr - 1] = color_purple
            }
        }

        // blue adjustment -- followed by white
        else if (prev == 0x05) && (
            (pixel == 0x05) ||
            (pixel == 0x03) || (pixel == 0x07)  // white
        ) {
            HiRes.pixelsSRGB[colorAddr - 1] = color_blue
        }
        
    }

    
    func UpdateColor() {
        var height = HiRes.PixelHeight
        
        // do not even render it...
        if videoMode.text == 1 {
            return
        }
        else {
            if videoMode.mixed == 1 {
                height = HiRes.MixedHeight
            }
            if MEMcfg.txt_page_2 == 1 {
                HiResBufferPointer = HiResBuffer2
            }
            else {
                HiResBufferPointer = HiResBuffer1
            }
        }
        
        var pixelAddr = 0
        
        var y = 0
        
        blockChanged = [Bool](repeating: false, count: HiRes.blockRows * HiRes.blockCols / 2)
        HiRes.context?.clear( CGRect(x: 0, y: 0, width: frame.width, height: frame.height) )
        
        for lineAddr in HiResLineAddrTbl {
            
            if ( height <= 0 ) {
                break
            }
            height -= 1
            
            let blockVertIdx = y / HiRes.blockHeight * HiRes.blockCols / 2
            var prev = 0

            for blockHorIdx in 0 ..< HiRes.blockCols / 2 {
//                print("blockVertIdx:", blockVertIdx, "   blockHorIdx:", blockHorIdx)
                
                let blockH = Int(HiResBufferPointer[ Int(lineAddr + blockHorIdx * 2) ])
                let blockH7 = ( blockH >> 5 ) & 0x04
                let blockL = Int(HiResBufferPointer[ Int(lineAddr + blockHorIdx * 2) + 1 ])
                let blockL7 = ( blockL >> 5 ) & 0x04
                
                let block = ( blockL << 7 ) | ( blockH & 0x7F ) & 0x3FFF
                let block8 = ( blockL << 8 ) | blockH

                let screenIdx = y * HiRes.blockCols + blockHorIdx
                
                // get all changed blocks
                blockChanged[ blockVertIdx + blockHorIdx ] = blockChanged[ blockVertIdx + blockHorIdx ] || shadowScreen[ screenIdx ] != block8
                shadowScreen[ screenIdx ] = block8
                
                for px in 0 ... 2  {
                    //                        let bitMask = 3 << ( px * 2 )
                    let pixel = blockH7 | ( (block >> (px * 2)) & 3 )
                    colorPixel(pixelAddr: pixelAddr, pixel: pixel, prev: prev )
                    pixelAddr += 8
                    prev = pixel
                }
                
                let pixel = (blockL7 | blockH7) | ( (block >> (3 * 2)) & 3 )
                colorPixel(pixelAddr: pixelAddr, pixel: pixel, prev: prev )
                pixelAddr += 8
                prev = pixel
                
                for px in 4 ... 6  {
                    //                        let bitMask = 3 << ( px * 2 )
                    let pixel = blockL7 | ( (block >> (px * 2)) & 3 )
                    colorPixel(pixelAddr: pixelAddr, pixel: pixel, prev: prev )
                    pixelAddr += 8
                    prev = pixel
                }
            }
            y += 1
        }

        
        // refresh changed block only
        refreshChanged(blockSize: 2)
//        needsDisplay = true // refresh the entire screen

    }

    
    func Update() {
        if ( ViewController.current?.ColorMonitor ?? true ) {
            UpdateColor()
        }
        else {
            UpdateMono()
        }
    }
        
    func fullUpdate() {
        needsDisplay = true
        Update()
    }
    
    
    override func draw(_ rect: CGRect) {
        guard let image = HiRes.context?.makeImage() else { return }

        // refresh the entire screen
        let boundingBox = CGRect(x: 0, y: 0, width: frame.width, height: frame.height)
        
        if ( ViewController.current?.CRTMonitor ?? false ) {
            currentContext?.interpolationQuality = .high // TODO: Make a switch that lets you turn on and off "old monitor effects"
        }
        else {
            currentContext?.interpolationQuality = .none
        }
        currentContext?.draw(image, in: boundingBox)
    }


    
}
