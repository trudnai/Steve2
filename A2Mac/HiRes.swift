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

    #if METAL_YES
    var device: MTLDevice!
    var metalLayer: CAMetalLayer!
    var vertexBuffer: MTLBuffer!
    var renderPipelineState: MTLRenderPipelineState!
    var computePipelineState: MTLComputePipelineState!
    var commandQueue: MTLCommandQueue!
//    var timer: CADisplayLink! // iOS only!
    var timer: CVDisplayLink! // MacOS only!
    var defaultLibrary : MTLLibrary!
    var addFunction : MTLFunction!
    var mtlBufferA : MTLBuffer!
    var mtlBufferB : MTLBuffer!
    var mtlBufferC : MTLBuffer!

    let vertexData: [Float] = [
       0.0,  1.0, 0.0,   -1.0, -1.0, 0.0,   1.0, -1.0, 0.0
    ]
    #endif // METAL_YES
    
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
    
    #if METAL_YES
    func initMetal() {
        device = MTLCreateSystemDefaultDevice()
        metalLayer = CAMetalLayer()          // 1
        metalLayer.device = device           // 2
        metalLayer.pixelFormat = .bgra8Unorm // 3
        metalLayer.framebufferOnly = true    // 4
        metalLayer.frame = frame  // 5
//        hires.layer = metalLayer   // 6
        
//        let dataSize = vertexData.count * MemoryLayout.size(ofValue: vertexData[0]) // 1
//        vertexBuffer = device.makeBuffer(bytes: vertexData, length: dataSize, options: []) // 2
        
        // 1
        defaultLibrary = device.makeDefaultLibrary()!
        addFunction = defaultLibrary.makeFunction(name: "add_arrays")
        
        computePipelineState = try! device.makeComputePipelineState(function: addFunction)
        
//        let fragmentProgram = defaultLibrary.makeFunction(name: "basic_fragment")
//        let vertexProgram = defaultLibrary.makeFunction(name: "basic_vertex")
            
        // 2
//        let pipelineState
//        let pipelineStateDescriptor = MTLRenderPipelineDescriptor()
//        pipelineStateDescriptor.vertexFunction = vertexProgram
//        pipelineStateDescriptor.fragmentFunction = fragmentProgram
//        pipelineStateDescriptor.colorAttachments[0].pixelFormat = .bgra8Unorm
            
        // 3
//        pipelineState = try! device.makeRenderPipelineState(descriptor: pipelineStateDescriptor)
        
        commandQueue = device.makeCommandQueue()
        
        mtlBufferA = device.makeBuffer(bytes: HiResRawPointer, length: HiRes.PageSize, options: .storageModeShared)
        mtlBufferB = device.makeBuffer(bytes: HiResRawPointer, length: HiRes.PageSize, options: .storageModeShared)
        mtlBufferC = device.makeBuffer(length: HiRes.PageSize * 4, options: .storageModeShared)

        var displayLink : CVDisplayLink!
        let displayID = CGMainDisplayID()
        let error = CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink)
        
//        timer = CVDisplayLink(   (target: self, selector: #selector(gameloop))
//        timer.add(to: RunLoop.main, forMode: .default)
        
//        CVDisplayLinkSetOutputCallback(displayLink!, renderCallback as? CVDisplayLinkOutputCallback, UnsafeMutableRawPointer( Unmanaged.passUnretained(self).toOpaque() ))
//        CVDisplayLinkStart(displayLink!)

    }
    #endif // METAL_YES
    
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


    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initHiResLineAddresses()
        HiRes.context?.clear( CGRect(x: 0, y: 0, width: frame.width, height: frame.height) )

//        currentContext?.setShouldAntialias(false)
//        currentContext?.interpolationQuality = CGInterpolationQuality.none
        
//        let scaleSizeW = Double((frame.size).width) / Double(HiRes.PixelWidth)
//        let scaleSizeH = Double((frame.size).height) / Double(HiRes.PixelHeight)

//        let scaleSizeW = 4
//        let scaleSizeH = 4
//        scaleUnitSquare(to: NSSize(width: scaleSizeW, height: scaleSizeH))
        
        // create smaller box views for draw optimization
        createHiRes()
        
        #if METAL_YES
        initMetal()
        #endif
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
    
    #if METAL_YES
    func compute() {
        let commandBuffer = commandQueue.makeCommandBuffer()!
        let computeEncoder = commandBuffer.makeComputeCommandEncoder()

        computeEncoder?.setComputePipelineState(computePipelineState)
        computeEncoder?.setBuffer(mtlBufferA, offset: 0, index: 0)
        computeEncoder?.setBuffer(mtlBufferA, offset: 0, index: 1)
        computeEncoder?.setBuffer(mtlBufferC, offset: 0, index: 2)
        
        let gridSize = MTLSizeMake(HiRes.PageSize, 1, 1)
        let threadGroupSize = min( computePipelineState.maxTotalThreadsPerThreadgroup, HiRes.PageSize )
        let threadgroupSize = MTLSizeMake(threadGroupSize, 1, 1)
        // Encode the Compute Command to Execute the Threads
        computeEncoder?.dispatchThreadgroups(gridSize, threadsPerThreadgroup: threadgroupSize)
        // no more compute passes
        computeEncoder?.endEncoding()
        // Commit the Command Buffer to Execute Its Commands
        commandBuffer.commit()
        // Wait for the Calculation to Complete
        commandBuffer.waitUntilCompleted()
        // Alternatively, to be notified when Metal has processed all of the commands,
        // add a completion handler to the command buffer (addCompletedHandler(_:)),
        // or check the status of a command buffer by reading its status property
        
        let result = UnsafeRawBufferPointer(start: mtlBufferC.contents(), count: HiRes.PageSize)
    }


    func render() {
        
        guard let drawable = metalLayer?.nextDrawable() else { return }
        let renderPassDescriptor = MTLRenderPassDescriptor()
        renderPassDescriptor.colorAttachments[0].texture = drawable.texture
        renderPassDescriptor.colorAttachments[0].loadAction = .clear
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColor(
            red: 0.0,
            green: 104.0/255.0,
            blue: 55.0/255.0,
            alpha: 1.0)
        
        let commandBuffer = commandQueue.makeCommandBuffer()!
        
        let renderEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)!
        renderEncoder.setRenderPipelineState(renderPipelineState)
        renderEncoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
        renderEncoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3, instanceCount: 1)
        renderEncoder.endEncoding()
        
        // committing buffer
        commandBuffer.present(drawable)
        commandBuffer.commit()
    }
    #endif // METAL_YES

    //    @objc func gameloop() {
    //      autoreleasepool {
    //        self.render()
    //      }
    //    }

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
        guard let colorSpace = CGColorSpace(name: CGColorSpace.genericRGBLinear) else { return nil }

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

    #if HIRESLOW || HIRESLOWCOLOR
    static let ScreenBitmapSize = (PixelWidth * PixelHeight * 4)
    static let context = createBitmapContext(pixelsWide: PixelWidth, PixelHeight)
    static let pixels = UnsafeMutableRawBufferPointer(start: context?.data, count: ScreenBitmapSize)
    static var typedPointer = pixels.bindMemory(to: UInt32.self)
    #endif
    
    let R = 2
    let G = 1
    let B = 0
    let A = 3
    
    var blockChanged = [Bool](repeating: false, count: HiRes.blockRows * HiRes.blockCols / 2)
    var shadowScreen = [Int](repeating: 0, count: PageSize)

    var was = 0;
    
    #if HIRESLOW
    override func draw(_ rect: CGRect) {
        // print("HIRESSLOW\n")

//        if was > 100 {
//            return
//        }
//        was += 1

        var pixelAddr = 0

        var minX = 9999
        var minY = 9999
        var maxX = 0
        var maxY = 0

        var x = 0
        var y = 0

        for lineAddr in HiResLineAddrTbl {
            for blockAddr in 0..<HiRes.blockCols {
                let block = Int(HiResBufferPointer[ Int(lineAddr + blockAddr) ])
                let screenIdx = y * HiRes.blockCols + x

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

                        if ( minX > x ) { minX = x }
                        if ( minY > y ) { minY = y }
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
        let boundingBox = CGRect(x: 0, y: 0, width: CGFloat(HiRes.PixelWidth), height: CGFloat(HiRes.PixelHeight))
        currentContext!.draw  (image, in: boundingBox)
    }
    
    #elseif HIRESLOWCOLOR
    
    
    let color_black     : UInt32 = 0x00000000;
    let color_white     : UInt32 = 0xFFFFFFFF;
    let color_purple    : UInt32 = 0xFFBB11EE;
    let color_green     : UInt32 = 0xFF0BA212;
    let color_blue      : UInt32 = 0xFF1166EE;
    let color_orange    : UInt32 = 0xFFEE2211;

    func hiresColorPixel ( pixelAddr : Int, pixel : Int, prev : Int ) {
        let colorAddr = pixelAddr / 4
        
        switch ( pixel ) {
        case 0x00: // black
//            HiRes.typedPointer[colorAddr] = color_black;
//            HiRes.typedPointer[colorAddr + 1] = color_black;
            break

        case 0x01: // purple (bits are in reverse!)
            HiRes.typedPointer[colorAddr] = color_purple;
//            HiRes.typedPointer[colorAddr + 1] = color_black;
            
        case 0x02: // green
            if  (prev == 0x02) ||
                (prev == 0x06) ||
                (prev == 0x03) || (prev == 0x07) ||
                (prev == 0x00) || (prev == 0x04) ||
                (prev == 0x04)
            {
                HiRes.typedPointer[colorAddr] = color_green;
            }
//            else {
//                HiRes.typedPointer[colorAddr] = color_black;
//            }
            
            // reducing color bleeding
            if  (prev == 0x01) ||
                (prev == 0x05)
            {
//                HiRes.typedPointer[colorAddr + 1] = color_black;
            }
            else {
                HiRes.typedPointer[colorAddr + 1] = color_green;
            }

        case 0x03: // white
            HiRes.typedPointer[colorAddr] = color_white;
            HiRes.typedPointer[colorAddr + 1] = color_white;

        case 0x04: // black 2
//            HiRes.typedPointer[colorAddr] = color_black;
//            HiRes.typedPointer[colorAddr + 1] = color_black;
            break
            
        case 0x05: // blue
            HiRes.typedPointer[colorAddr] = color_blue;
//            HiRes.typedPointer[colorAddr + 1] = color_black;

        case 0x06: // orange
            // do we need to extend the color?
            if  (prev == 0x06) ||
                (prev == 0x03) || (prev == 0x07)
            {
                HiRes.typedPointer[colorAddr] = color_orange;
            }
            else {
//                HiRes.typedPointer[colorAddr] = color_black;
            }

            HiRes.typedPointer[colorAddr + 1] = color_orange;

        case 0x07: // white 2
            HiRes.typedPointer[colorAddr] = color_white;
            HiRes.typedPointer[colorAddr + 1] = color_white;

        default:
//            HiRes.typedPointer[colorAddr] = color_black;
//            HiRes.typedPointer[colorAddr + 1] = color_black;
            break
        }
        
        // white adjustment
        if ( (prev & 2) == 2 ) && ( (pixel & 1) == 1 ) {
            HiRes.typedPointer[colorAddr] = color_white;
            HiRes.typedPointer[colorAddr - 1] = color_white;

            // TODO: Need better check if extra green was created
            if (HiRes.pixels[pixelAddr - 8 + G] == 0xA2 ) {
                HiRes.typedPointer[colorAddr - 2] = color_black;
            }
        }

        // purple adjustment -- followed by white
        else if (prev == 0x01) && (
            (pixel == 0x01) || (pixel == 0x03) ||
            (pixel == 0x07) || (pixel == 0x00) ||
            (pixel == 0x04)
        ) {
            // was the previous purple pixel promoted to white or is it still purple?
            if ( HiRes.pixels[pixelAddr - 8 + R] == 0xBB ) {
                HiRes.typedPointer[colorAddr - 1] = color_purple;
            }
        }

        // blue adjustment
        else if (prev == 0x05) && (
            (pixel == 0x05) ||
            (pixel == 0x03) || (pixel == 0x07)
        ) {
            HiRes.typedPointer[colorAddr - 1] = color_blue;
        }

    }

    
    func Update() {
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
            
            let blockVertIdx = y / 8 * HiRes.blockCols / 2
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
                    hiresColorPixel(pixelAddr: pixelAddr, pixel: pixel, prev: prev )
                    pixelAddr += 8
                    prev = pixel
                }
                
                let pixel = blockH7 | ( (block >> (3 * 2)) & 3 )
                hiresColorPixel(pixelAddr: pixelAddr, pixel: pixel, prev: prev )
                pixelAddr += 8
                prev = pixel
                
                for px in 4 ... 6  {
                    //                        let bitMask = 3 << ( px * 2 )
                    let pixel = blockL7 | ( (block >> (px * 2)) & 3 )
                    hiresColorPixel(pixelAddr: pixelAddr, pixel: pixel, prev: prev )
                    pixelAddr += 8
                    prev = pixel
                }
            }
            y += 1
        }

        
        // refresh changed block only
        
        let blockScreenWidth = Int(frame.width) / HiRes.blockCols * 2
        let blockScreenHeigth = Int(frame.height) / HiRes.blockRows

        for blockVertIdx in 0 ..< HiRes.blockRows {
            for blockHorIdx in 0 ..< HiRes.blockCols / 2 {
                if blockChanged[ blockVertIdx * HiRes.blockCols / 2 + blockHorIdx ] {
                    // refresh the entire screen
                    let boundingBox = CGRect(
                        x: blockHorIdx * blockScreenWidth - 2,
                        y: Int(frame.height) - blockVertIdx * blockScreenHeigth - blockScreenHeigth - 2,
                        width: blockScreenWidth + 4,
                        height: blockScreenHeigth + 4)
                    
                    self.setNeedsDisplay( boundingBox )
                }
            }
        }
        
//        needsDisplay = true // refresh the entire screen

    }
    
    override func draw(_ rect: CGRect) {
        // print("HIRESSLOW\n")

//        if was > 100 {
//            return
//        }
//        was += 1
        

//        HiRes.context?.setShouldAntialias(true)
//        HiRes.context?.interpolationQuality = CGInterpolationQuality.low

        guard let image = HiRes.context?.makeImage() else { return }

//        let blockScreenWidth = HiRes.PixelWidth * 4 / (HiRes.blockCols / 2)
//        let blockScreenHeigth = HiRes.PixelHeight * 4 / HiRes.blockRows
//
//        for y in 0 ..< HiRes.blockRows {
//            for x in 0 ..< HiRes.blockCols / 2 {
//                // refresh the entire screen
//                let boundingBox = CGRect(x: x * blockScreenWidth, y: y * blockScreenHeigth, width: blockScreenWidth, height: blockScreenHeigth)
//                currentContext!.draw  (image, in: boundingBox)
//return
//            }
//        }
        
        
        // refresh the entire screen
        let boundingBox = CGRect(x: 0, y: 0, width: frame.width, height: frame.height)
        currentContext!.draw(image, in: boundingBox)
    }

    #elseif HIRESDRAWCOLOR
    
    let colorPalette : [NSColor] = [
        NSColor(calibratedRed: 0.0000, green: 0.000, blue: 0.000, alpha: 0.0), // black
        NSColor(calibratedRed: 0.0314, green: 0.635, blue: 0.071, alpha: 1.0), // green
        NSColor(calibratedRed: 0.0314, green: 0.635, blue: 0.071, alpha: 1.0), // purple
        NSColor(calibratedRed: 1.0000, green: 1.000, blue: 1.000, alpha: 1.0), // white
        NSColor(calibratedRed: 0.0000, green: 0.000, blue: 0.000, alpha: 0.0), // black
        NSColor(calibratedRed: 0.0314, green: 0.635, blue: 0.071, alpha: 1.0), // orange
        NSColor(calibratedRed: 0.0314, green: 0.635, blue: 0.071, alpha: 1.0), // blue
        NSColor(calibratedRed: 1.0000, green: 1.000, blue: 1.000, alpha: 1.0), // white
    ]
    
    let path = NSBezierPath()
    
    override func draw(_ rect: CGRect) {
//        NSColor.green.setFill()
        NSColor(calibratedRed: 0.0314, green: 0.635, blue: 0.071, alpha: 1.0).setStroke()

        path.removeAllPoints()
        path.lineWidth = 0.7
        path.move(to: NSPoint(x: 0, y: 0))
        
//        path.appendRect(NSRect(x: 0, y: 0, width: 10, height: 10))

        for y in 0 ..< HiRes.PixelHeight {
            var color : UInt = 0
            var lastColor : UInt = 0
            path.move(to: NSPoint(x: 0, y: y))

            // for color screen we need to process blocks in a pair
            for blockX in 0 ..< HiRes.blockCols / 2 {
                
                let lineAddr = HiResLineAddrTbl[y]
                var x = blockX * HiRes.blockWidth * 2

                var block0 = UInt(HiResBufferPointer[ Int(lineAddr + blockX * 2) + 0 ])
                var block1 = UInt(HiResBufferPointer[ Int(lineAddr + blockX * 2) + 1 ])
                let b70 : UInt = (block0 & 0x80) >> 5
                let b71 : UInt = (block1 & 0x80) >> 5
                
                var block7 : [UInt] = [0,0,0,0,0,0,0]
                var i = 0
                
                for _ in 0 ... 2 {
                    block7[i]  = b70
                    block7[i] |= block0 & 3
                    block0 >>= 2
                    i += 1
                }

                block7[i]  = b70
                block7[i] |= ((block0 & 1) << 1) | (block1 & 1)
                block1 >>= 1
                i += 1
                
                for _ in 0 ... 2 {
                    block7[i]  = b71
                    block7[i] |= block1 & 3
                    block1 >>= 2
                    i += 1
                }


                for px in 0 ... 6 { // stride(from: 0, through: 6, by: 1) {
                    color = block7[px]
                    if (color != lastColor) {
                        colorPalette[Int(color)].setStroke()
                        path.line(to: NSPoint(x: x, y: y))
                        path.stroke()
                        path.removeAllPoints()
                        lastColor = color
                    }
                    
                    x += 1
                }
            } // x
            // make sure we close the path at the end of the horizontal line
            if (color != lastColor) {
                path.line(to: NSPoint(x: 279, y: y))
                lastColor = color
            }
        }
//        path.fill()
        
        path.stroke()

    }
    #elseif HIRESDRAW
    let path = NSBezierPath()
    override func draw(_ rect: CGRect) {
        let pixelWidth = bounds.width / CGFloat(HiRes.PixelWidth)
        let pixelHeight = bounds.height / CGFloat(HiRes.PixelHeight)

    //        self.hidden = videoMode.text == 1
        
//        NSColor.green.setFill()
//        NSColor(calibratedRed: 0.0314, green: 0.635, blue: 0.071, alpha: 1.0).setStroke()
//        NSColor(calibratedRed: 0.05, green: 0.7, blue: 0.1, alpha: 1.0).setStroke()
        NSColor.systemGreen.setStroke()

        path.removeAllPoints()
        path.lineWidth = 0.7 * pixelHeight
        path.move(to: NSPoint(x: 0, y: 0))
        
//        path.appendRect(NSRect(x: 0, y: 0, width: 10, height: 10))
        
        var height = HiRes.PixelHeight

        // do not even render it...
        if videoMode.text == 1 {
            return
        }
        else {
            if videoMode.mixed == 1 {
                height = HiRes.MixedHeight
            }
            if videoMode.page == 1 {
                HiResBufferPointer = HiResBuffer2
            }
            else {
                HiResBufferPointer = HiResBuffer1
            }
        }

        
        for y in 0 ..< height {
            var inX = false
            path.move(to: NSPoint(x: 0, y: y))

            for blockX in 0 ..< HiRes.blockCols {
                
                let lineAddr = HiResLineAddrTbl[y]
                let block = UInt(HiResBufferPointer[ Int(lineAddr + blockX) ])

    //                    if( shadowScreen[ screenIdx ] != block ) {
    //                        shadowScreen[ screenIdx ] = block
    //
                var x = blockX * HiRes.blockWidth
                if block != 0 && block != 0x80 {
                    for bit in 0 ... 6 { // stride(from: 0, through: 6, by: 1) {
                        let bitMask : UInt = 1 << bit
                        if (block & bitMask) == 0 {
                            if inX {
                                inX = false
                                path.line(to: NSPoint(x: CGFloat(x) * pixelWidth, y: CGFloat(192 - y) * pixelHeight ))
                            }
                        }
                        else { // 28CD41
                            if ( inX == false ) {
                                inX = true
                                path.move(to: NSPoint(x: CGFloat(x) * pixelWidth, y: CGFloat(192 - y) * pixelHeight ))
                            }
                        }
                        
                        x += 1
                    }
                }
                else {
                    // make sure we close the path if the next block is completely zero
                    if inX {
                        inX = false
                        path.line(to: NSPoint(x: CGFloat(x) * pixelWidth, y: CGFloat(192 - y) * pixelHeight ))
                    }
                }
            } // x
            // make sure we close the path at the end of the horizontal line
            if inX {
                inX = false
                path.line(to: NSPoint(x: 279 * pixelWidth, y: CGFloat(192 - y) * pixelHeight ))
            }
        }
//        path.fill()
        path.stroke()

    }
    #elseif HIRES
    override func draw(_ rect: CGRect) {
        
        // print("HIRESBLOCKS\n")
        
//        if was > 100 {
//            return
//        }
//        was += 1
        
        for blockY in 0 ..< HiRes.blockRows {
            for blockX in 0 ..< HiRes.blockCols {
                let blockView = HiResSubView[blockY][blockX]
                
                let bitmapSize = HiRes.blockWidth * HiRes.blockHeight * 4
                let context = HiRes.createBitmapContext(pixelsWide: HiRes.blockWidth, HiRes.blockHeight)
                let pixels = UnsafeMutableRawBufferPointer(start: context?.data, count: bitmapSize)  // UnsafeMutablePointer<CUnsignedChar>.allocate(capacity: byteCount)


                var blockNeedsDisplay = false
                for line in 0 ... 7 {
                    let y = blockY + line
                    let screenIdx = y * HiRes.blockCols + blockX
                    let pixelAddr = line

                    let lineAddr = HiResLineAddrTbl[y]
                    let block = Int(HiResBufferPointer[ Int(lineAddr + blockX) ])

                    if( shadowScreen[ screenIdx ] != block ) {
                        shadowScreen[ screenIdx ] = block

                        blockNeedsDisplay = true

                        var x = blockX * HiRes.blockWidth
                        for bit in 0 ... 6 { // stride(from: 0, through: 6, by: 1) {
                            let bitMask = 1 << bit
                            if (block & bitMask) == 0 {
                                pixels[pixelAddr + R] = 0x00;
                                pixels[pixelAddr + G] = 0x00;
                                pixels[pixelAddr + B] = 0x00;
                                pixels[pixelAddr + A] = 0x00;
                            }
                            else { // 28CD41
                                pixels[pixelAddr + R] = 0x08;
                                pixels[pixelAddr + G] = 0xA2;
                                pixels[pixelAddr + B] = 0x12;
                                pixels[pixelAddr + A] = 0x7F;
                            }
                            
                            x += 1
                        }
                    }
                }

                if blockNeedsDisplay {
                    blockView.needsDisplay = true
//                    print("block(\(blockX),\(blockY))")
                    guard let image = context?.makeImage() else { return }
                    let boundingBox = CGRect(x: 0, y: 0, width: CGFloat(HiRes.PixelWidth), height: CGFloat(HiRes.PixelHeight))
                    currentContext!.draw(image, in: boundingBox)
                }

            }
        }

    }
    #endif

    
}
