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
    static let LineBlocks = 40

    let HiResBufferPointer = UnsafeRawBufferPointer(start: &RAM + Page1Addr, count: PageSize * 2)
    
    // holds the starting addresses for each lines minus the screen page starting address
    var HiResLineAddrTbl = [UInt16](repeating: 0, count: PixelHeight)

    func initHiResLineAddresses() {
        var i = 0
        for x : UInt16 in stride(from: 0, through: 0x50, by: 0x28) {
            for y : UInt16 in stride(from: 0, through: 0x380, by: 0x80) {
                for z : UInt16 in stride(from: 0, through: 0x1C00, by: 0x400) {
                    HiResLineAddrTbl[i] = x + y + z
                    i += 1
                }
            }
        }
    }
    
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initHiResLineAddresses()
        
        
        let scaleSizeW = Double((frame.size).width) / Double(HiRes.PixelWidth)
        let scaleSizeH = Double((frame.size).height) / Double(HiRes.PixelHeight)
        scaleUnitSquare(to: NSSize(width: scaleSizeW, height: scaleSizeH))
        
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
    
    static func createBitmapContext(pixelsWide: Int, _ pixelsHigh: Int) -> CGContext? {
        let bytesPerPixel = 4
        let bytesPerRow = bytesPerPixel * pixelsWide
        let bitsPerComponent = 8
        
        let byteCount = (bytesPerRow * pixelsHigh)
        
        guard let colorSpace = CGColorSpace(name: CGColorSpace.linearSRGB) else { return nil }
        
        let pixels = UnsafeMutablePointer<CUnsignedChar>.allocate(capacity: byteCount)

        let bitmapInfo = CGImageAlphaInfo.premultipliedFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue
        
        let context = CGContext(data: pixels, width: pixelsWide, height: pixelsHigh, bitsPerComponent: bitsPerComponent, bytesPerRow: bytesPerRow, space: colorSpace, bitmapInfo: bitmapInfo)
        
        return context
    }
    
//    override func draw(_ rect: CGRect) {
//        let width  = 200
//        let height = 300
//        let boundingBox = CGRect(x: 0, y: 0, width: CGFloat(width), height: CGFloat(height))
//        let context = createBitmapContext(pixelsWide: width, height)
//
//        let data = context?.data
//        var currentPixel: [UInt32] = unsafeBitCast(data, to: [UInt32].self)
//
//        var n = 0
//        for _ in 0..<height {
//            for _ in 0..<width {
//                currentPixel[n] = 0
//                n += 1
//            }
//        }
//
//        guard let image = context?.makeImage() else { return }
//        context?.draw(image, in: boundingBox)
//    }

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

    
    static let byteCount = (HiRes.PixelWidth * HiRes.PixelHeight * 4)
    static let context = createBitmapContext(pixelsWide: HiRes.PixelWidth, HiRes.PixelHeight)
    static let pixels = UnsafeMutableRawBufferPointer(start: context?.data, count: byteCount)  // UnsafeMutablePointer<CUnsignedChar>.allocate(capacity: byteCount)

    let R = 2
    let G = 1
    let B = 0
    let A = 3
    
    var shadowScreen = [Int](repeating: 1, count: PageSize)

    override func draw(_ rect: CGRect) {
        var pixelAddr = 0
        
        var minX = 0
        var minY = 0
        var maxX = 0
        var maxY = 0

        var x = 0
        var y = 0
        
        for lineAddr in HiResLineAddrTbl {
            for blockAddr in 0..<UInt16(HiRes.LineBlocks) {
                let block = Int(HiResBufferPointer[ Int(lineAddr + blockAddr) ])
                let screenIdx = y * HiRes.LineBlocks + x

                if ( shadowScreen[ screenIdx ] != block ) {
                    shadowScreen[ screenIdx ] = block
                    
                    for bit in stride(from: 0, through: 6, by: 1) {
                        let bitMask = 1 << bit
                        if (block & bitMask) == 0 {
                            HiRes.pixels[pixelAddr + R] = 0x00;
                            HiRes.pixels[pixelAddr + G] = 0x00;
                            HiRes.pixels[pixelAddr + B] = 0x00;
                            HiRes.pixels[pixelAddr + A] = 0x00;
                        }
                        else { // 28CD41
                            HiRes.pixels[pixelAddr + R] = 0x08;
                            HiRes.pixels[pixelAddr + G] = 0xA2;
                            HiRes.pixels[pixelAddr + B] = 0x12;
                            HiRes.pixels[pixelAddr + A] = 0xFF;
                        }
                        
                        if ( minX == 0 ) { minX = x }
                        if ( minY == 0 ) { minY = y }
                        if ( maxX < x ) { maxX = x }
                        if ( maxY < y ) { maxY = y }
                        
                        pixelAddr += 4
                        x += 1
                    }
                }
                else {
                    pixelAddr += 4 * 7
                    x += 7
                }
            }
            
            y += 1
            x = 0
        }

        guard let image = HiRes.context?.makeImage() else { return }

        if let currentContext: CGContext = currentContext {
//            UIImagePNGRepresentation(UIImage(CGImage:image!))?.writeToFile("/Users/admin/Desktop/aaaaa.png", atomically: true)
//            if ( maxX > minX ) && ( maxY > minY ) {
                let boundingBox = CGRect(x: 0, y: 0, width: CGFloat(HiRes.PixelWidth), height: CGFloat(HiRes.PixelHeight))
//                let boundingBox = CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
                currentContext.draw  (image, in: boundingBox)
//            }
        }
    }
    
}
