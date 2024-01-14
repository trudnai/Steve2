//
//  LoRes.swift
//  Steve ][
//
//  Created by Tamas Rudnai on 9/19/19.
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

//import Foundation
import AppKit

class LoRes: NSView {
    
    static let PageSize  = 0x400
    static let Page1Addr = 0x400
    static let Page2Addr = 0x800
    
    static let PixelWidth  = 40
    static let PixelMixedHeight = 40
    static let PixelHeight = 48
    static let MixedTextHeight = 4
    static let blockRows = 24
    static let blockCols = 40
    static let blockWidth = PixelWidth / blockCols
    static let blockHeight = PixelHeight / blockRows

    let LoResIntBuffer1 = UnsafeRawBufferPointer(start: RAM + Page1Addr, count: PageSize * 2)
    let LoResIntBuffer2 = UnsafeRawBufferPointer(start: RAM + Page2Addr, count: PageSize * 2)
    let LoResBuffer1 = UnsafeRawBufferPointer(start: MEM + Page1Addr, count: PageSize * 2)
    let LoResBuffer2 = UnsafeRawBufferPointer(start: MEM + Page2Addr, count: PageSize * 2)
    var LoResBufferPointer = UnsafeRawBufferPointer(start: MEM + Page1Addr, count: PageSize * 2)
    
    let LoResRawPointer = UnsafeRawPointer(RAM + Page1Addr)

    // holds the starting addresses for each lines minus the screen page starting address
    var LoResLineAddrTbl = [Int](repeating: 0, count: PixelHeight * 4)

    func initLoResLineAddresses() {
        var i = 0
        for x in stride(from: 0, through: 0x50, by: 0x28) {
            for y in stride(from: 0, through: 0x380, by: 0x80) {
                for z in stride(from: 0, through: 0x1C00, by: 0x400) {
                    LoResLineAddrTbl[i] = x + y + z
                    i += 1
                }
            }
        }
    }
    
    
    var LoResSubView = [[NSView]]()
    
    func createLoRes() {
        for y in 0 ..< LoRes.blockRows {
            LoResSubView.append([NSView]())
            for x in 0 ..< LoRes.blockCols {
                let blockView = NSView(frame: NSRect(x: x * LoRes.blockWidth, y: y * 8, width: LoRes.blockWidth, height: 8))
                LoResSubView[y].append(blockView)
                self.addSubview(blockView)
            }
        }
    }
    
    
    func clearScreen() {
        LoRes.context?.clear( CGRect(x: 0, y: 0, width: frame.width, height: frame.height) )
        needsDisplay = true
    }


    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initLoResLineAddresses()
        clearScreen()

//        currentContext?.setShouldAntialias(false)
//        currentContext?.interpolationQuality = CGInterpolationQuality.none
        
//        let scaleSizeW = Double((frame.size).width) / Double(LoRes.PixelWidth)
//        let scaleSizeH = Double((frame.size).height) / Double(LoRes.PixelHeight)

//        let scaleSizeW = 4
//        let scaleSizeH = 4
//        scaleUnitSquare(to: NSSize(width: scaleSizeW, height: scaleSizeH))
        
        // create smaller box views for draw optimization
        createLoRes()
        
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
    

    func renderCallback(displayLink : CVDisplayLink,
                               const  inNow : UnsafePointer<CVTimeStamp>,
                               const  inOutputTime : UnsafePointer<CVTimeStamp>,
                               flagsIn : CVOptionFlags,
                               flagsOut : UnsafeMutablePointer<CVOptionFlags>,
                               displayLinkContext : UnsafeMutableRawPointer) -> CVReturn
    {
        /*  It's prudent to also have a brief discussion about the CVTimeStamp.
            CVTimeStamp has five properties.  Three of the five are very useful
            for keeping track of the current time, calculating delta time, the
            frame number, and the number of frames per second.  The utility of
            each property is not terribly obvious from just reading the names
            or the descriptions in the Developer dcumentation and has been a
            mystery to many a developer.  Thankfully, CaptainRedmuff on
            StackOverflow asked a question that provided the equation that
            calculates frames per second.  From that equation, we can
            extrapolate the value of each field.

            @hostTime = current time in Units of the "root".  Yeah, I don't know.
              The key to this field is to understand that it is in nanoseconds
              (e.g. 1/1_000_000_000 of a second) not units.  To convert it to
              seconds divide by 1_000_000_000.  Dividing by videoRefreshPeriod
              and videoTimeScale in a calculation for frames per second yields
              the appropriate number of frames.  This works as a result of
              proportionality--dividing seconds by seconds.  Note that dividing
              by videoTimeScale to get the time in seconds does not work like it
              does for videoTime.

              framesPerSecond:
                (videoTime / videoRefreshPeriod) / (videoTime / videoTimeScale) = 59
              and
                (hostTime / videoRefreshPeriod) / (hostTime / videoTimeScale) = 59
              but
                hostTime * videoTimeScale ≠ seconds, but Units = seconds * (Units / seconds) = Units

          @rateScalar = ratio of "rate of device in CVTimeStamp/unitOfTime" to
            the "Nominal Rate".  I think the "Nominal Rate" is
            videoRefreshPeriod, but unfortunately, the documentation doesn't
            just say videoRefreshPeriod is the Nominal rate and then define
            what that means.  Regardless, because this is a ratio, and the fact
            that we know the value of one of the parts (e.g. Units/frame), we
            then know that the "rate of the device" is frame/Units (the units of
            measure need to cancel out for the ratio to be a ratio).  This
            makes sense in that rateScalar's definition tells us the rate is
            "measured by timeStamps".  Since there is a frame for every
            timeStamp, the rate of the device equals CVTimeStamp/Unit or
            frame/Unit.  Thus,

              rateScalar = frame/Units : Units/frame

          @videoTime = the time the frame was created since computer started up.
            If you turn your computer off and then turn it back on, this timer
            returns to zero.  The timer is paused when you put your computer to
            sleep.  This value is in Units not seconds.  To get the number of
            seconds this value represents, you have to apply videoTimeScale.

          @videoRefreshPeriod = the number of Units per frame (i.e. Units/frame)
            This is useful in calculating the frame number or frames per second.
            The documentation calls this the "nominal update period" and I am
            pretty sure that is quivalent to the aforementioned "nominal rate".
            Unfortunately, the documetation mixes naming conventions and this
            inconsistency creates confusion.

              frame = videoTime / videoRefreshPeriod

          @videoTimeScale = Units/second, used to convert videoTime into seconds
            and may also be used with videoRefreshPeriod to calculate the expected
            framesPerSecond.  I say expected, because videoTimeScale and
            videoRefreshPeriod don't change while videoTime does change.  Thus,
            to calculate fps in the case of system slow down, one would need to
            use videoTime with videoTimeScale to calculate the actual fps value.

              seconds = videoTime / videoTimeScale

              framesPerSecondConstant = videoTimeScale / videoRefreshPeriod (this value does not change if their is system slowdown)

        USE CASE 1: Time in DD:HH:mm:ss using hostTime
          let rootTotalSeconds = inNow.pointee.hostTime
          let rootDays = inNow.pointee.hostTime / (1_000_000_000 * 60 * 60 * 24) % 365
          let rootHours = inNow.pointee.hostTime / (1_000_000_000 * 60 * 60) % 24
          let rootMinutes = inNow.pointee.hostTime / (1_000_000_000 * 60) % 60
          let rootSeconds = inNow.pointee.hostTime / 1_000_000_000 % 60
          Swift.print("rootTotalSeconds: \(rootTotalSeconds) rootDays: \(rootDays) rootHours: \(rootHours) rootMinutes: \(rootMinutes) rootSeconds: \(rootSeconds)")

        USE CASE 2: Time in DD:HH:mm:ss using videoTime
          let totalSeconds = inNow.pointee.videoTime / Int64(inNow.pointee.videoTimeScale)
          let days = (totalSeconds / (60 * 60 * 24)) % 365
          let hours = (totalSeconds / (60 * 60)) % 24
          let minutes = (totalSeconds / 60) % 60
          let seconds = totalSeconds % 60
          Swift.print("totalSeconds: \(totalSeconds) Days: \(days) Hours: \(hours) Minutes: \(minutes) Seconds: \(seconds)")

          Swift.print("fps: \(Double(inNow.pointee.videoTimeScale) / Double(inNow.pointee.videoRefreshPeriod)) seconds: \(Double(inNow.pointee.videoTime) / Double(inNow.pointee.videoTimeScale))")
         */

        /*  The displayLinkContext in CVDisplayLinkOutputCallback's parameter list is the
            view being driven by the CVDisplayLink.  In order to use the context as an
            instance of SwiftOpenGLView (which has our drawView() method) we need to use
            unsafeBitCast() to cast this context to a SwiftOpenGLView.
         */

//        let view = unsafeBitCast(displayLinkContext, to: SwiftOpenGLView.self)
//        //  Capture the current time in the currentTime property.
//        view.currentTime = inNow.pointee.videoTime / Int64(inNow.pointee.videoTimeScale)
//        view.drawView()

//        self.render()
        
        return kCVReturnSuccess
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
    
    var blockChanged = [Bool](repeating: false, count: LoRes.blockRows * LoRes.blockCols)
    var shadowScreen = [Int](repeating: 0, count: PageSize)

    var was = 0;
    
    
//    static let color_black         : UInt32 = 0x00000000; //  0
//    static let color_magenta       : UInt32 = 0xFF660022; //  1
//    static let color_dark_blue     : UInt32 = 0xFF000077; //  2
//    static let color_purple        : UInt32 = 0xFF9908DD; //  3
//    static let color_dark_green    : UInt32 = 0xFF005500; //  4
//    static let color_dark_gray     : UInt32 = 0xFF333333; //  5
//    static let color_medium_blue   : UInt32 = 0xFF0011BB; //  6
//    static let color_light_blue    : UInt32 = 0xFF4488FF; //  7
//    static let color_brown         : UInt32 = 0xFF552200; //  8
//    static let color_orange        : UInt32 = 0xFFFF6611; //  9
//    static let color_gray          : UInt32 = 0xFF888888; // 10
//    static let color_pink          : UInt32 = 0xFFFF8888; // 11
//    static let color_green         : UInt32 = 0xFF0BBB11; // 12
//    static let color_yellow        : UInt32 = 0xFFFFFF00; // 13
//    static let color_aqua          : UInt32 = 0xFF66CC99; // 14
//    static let color_white         : UInt32 = 0xFFEEEEEE; // 15

    static let color_black         : UInt32 = 0x00000000; //  0
    static let color_magenta       : UInt32 = 0xFFDD0077; //  1
    static let color_dark_blue     : UInt32 = 0xFF0006F6; //  2
    static let color_purple        : UInt32 = 0xFFCC00FF; //  3
    static let color_dark_green    : UInt32 = 0xFF009800; //  4
    static let color_dark_gray     : UInt32 = 0xFF888888; //  5 // Darker only on //gs 
    static let color_medium_blue   : UInt32 = 0xFF006FFD; //  6
    static let color_light_blue    : UInt32 = 0xFF5AA3F0; //  7
    static let color_brown         : UInt32 = 0xFF5C341F; //  8
    static let color_orange        : UInt32 = 0xFFFF6302; //  9
    static let color_gray          : UInt32 = 0xFF888888; // 10
    static let color_pink          : UInt32 = 0xFFFF50B9; // 11
    static let color_green         : UInt32 = 0xFF2BD84A; // 12
    static let color_yellow        : UInt32 = 0xFFFFE700; // 13
    static let color_aqua          : UInt32 = 0xFF71EED6; // 14
    static let color_white         : UInt32 = 0xFFEEEEEE; // 15

    
    let colorTable = [
        color_black,
        color_magenta,
        color_dark_blue,
        color_purple,
        color_dark_green,
        color_dark_gray,
        color_medium_blue,
        color_light_blue,
        color_brown,
        color_orange,
        color_gray,
        color_pink,
        color_green,
        color_yellow,
        color_aqua,
        color_white
    ]
    
    // for debugging only:
    let color_turquis       : UInt32 = 0xFF11BBBB;
    let color_blue          : UInt32 = 0xFF1155FF;

    
    func colorPixel ( pixelAddr : Int, color : Int ) {
        LoRes.pixelsSRGB[pixelAddr] = colorTable[color]
    }

    
    func Render() {
        var height = LoRes.PixelHeight / 2
        
        // do not even render it...
        if videoMode.text == 1 {
            return
        }
        else {
            if videoMode.mixed == 1 {
                height = LoRes.PixelMixedHeight / 2
            }
            if MEMcfg.txt_page_2 == 1 {
                if (MEMcfg.RD_AUX_MEM == 0) {
                    LoResBufferPointer = LoResBuffer2
                }
                else {
                    LoResBufferPointer = LoResIntBuffer2
                }
            }
            else {
                if (MEMcfg.RD_AUX_MEM == 0) {
                    LoResBufferPointer = LoResBuffer1
                }
                else {
                    LoResBufferPointer = LoResIntBuffer1
                }
            }
        }
        
        var y = 0
        
        blockChanged = [Bool](repeating: false, count: LoRes.blockRows * LoRes.blockCols)
        
        LoRes.context?.clear( CGRect(x: 0, y: 0, width: frame.width, height: frame.height) )
        
        for lineAddr in ViewController.textLineOfs {
            
            if ( height <= 0 ) {
                break
            }
            height -= 1
            
            let blockVertIdx = y * LoRes.blockCols

            for blockHorIdx in 0 ..< LoRes.blockCols {
//                print("blockVertIdx:", blockVertIdx, "   blockHorIdx:", blockHorIdx)
                
                let block = Int(LoResBufferPointer[ Int(lineAddr + blockHorIdx) ])

                let screenIdx = blockVertIdx + blockHorIdx
                let pixelHAddr = blockVertIdx * 2 + blockHorIdx
                let pixelLAddr = pixelHAddr + LoRes.blockCols

                // get all changed blocks
                blockChanged[ screenIdx ] = blockChanged[ screenIdx ] || shadowScreen[ screenIdx ] != block
                shadowScreen[ screenIdx ] = block
                
                colorPixel(pixelAddr: pixelHAddr, color: block & 0x0F )
                colorPixel(pixelAddr: pixelLAddr, color: (block >> 4) & 0x0F )

            }
            y += 1
            
            if ( y >= LoRes.PixelHeight ) {
                break
            }
        }

        
        // refresh changed block only
        
        let screenBlockMargin = 6
        
        let blockScreenWidth = CGFloat(frame.width) / CGFloat(HiRes.blockCols)
        let blockScreenHeigth = CGFloat(frame.height) / CGFloat(HiRes.blockRows)

        for blockVertIdx in 0 ..< LoRes.blockRows {
            for blockHorIdx in 0 ..< LoRes.blockCols {
                if blockChanged[ blockVertIdx * LoRes.blockCols + blockHorIdx ] {
                    // refresh the entire screen
                    let x = CGFloat(blockHorIdx) * blockScreenWidth - CGFloat(screenBlockMargin)
                    let y = frame.height - CGFloat(blockVertIdx) * blockScreenHeigth - blockScreenHeigth - CGFloat(screenBlockMargin)
                    let w = blockScreenWidth + CGFloat(screenBlockMargin) * 2
                    let h = blockScreenHeigth + CGFloat(screenBlockMargin) * 2
                    
                    let boundingBox = CGRect(x: x, y: y, width: w, height: h)
                    self.setNeedsDisplay( boundingBox )
                }
            }
        }
        
//        needsDisplay = true // refresh the entire screen

    }
    
    override func draw(_ rect: CGRect) {
        guard let image = LoRes.context?.makeImage() else { return }

        // refresh the entire screen
        let boundingBox = CGRect(x: 0, y: 0, width: frame.width, height: frame.height)
        currentContext?.interpolationQuality = .none
        currentContext?.draw(image, in: boundingBox)
    }


    
}
