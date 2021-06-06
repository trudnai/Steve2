//
//  hires.c
//  A2Mac
//
//  Created by Tamas Rudnai on 5/28/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

#include "hires.h"
#include "mmio.h"

static const uint16_t PageSize  = 0x2000;
static const uint16_t Page1Addr = 0x2000;
static const uint16_t Page2Addr = 0x4000;

static const uint16_t PixelWidth  = 280;
static const uint16_t PixelMixedHeight = 160;
static const uint16_t PixelHeight = 192;
static const uint16_t MixedHeight = 160;
static const uint8_t MixedTextHeight = 4;
static const uint8_t blockRows = 24;
static const uint8_t blockCols = 40;
static const uint8_t blockWidth = PixelWidth / blockCols;
static const uint8_t blockHeight = PixelHeight / blockRows;

const uint8_t* HiResRawPointer = Apple2_64K_RAM + Page1Addr;
const uint8_t* HiResBuffer1 = Apple2_64K_MEM + Page1Addr;
const uint8_t* HiResBuffer2 = Apple2_64K_MEM + Page2Addr;
uint8_t* HiResBufferPointer = Apple2_64K_MEM + Page1Addr;

// holds the starting addresses for each lines minus the screen page starting address
uint16_t HiResLineAddrTbl[PixelHeight];

typedef enum {
    _B = 0,
    _G = 1,
    _R = 2,
    _A = 3,
} RGBA_t;


// HiRes Colors for the SRGB color space
const uint32_t color_black      = 0x00000000;
const uint32_t color_white      = 0xFFEEEEEE;
const uint32_t color_purple     = 0xFFDD55FF;
const uint32_t color_green      = 0xFF2BD84A;
const uint32_t color_blue       = 0xFF5599FF;
const uint32_t color_orange     = 0xFFFF6302;

// for debugging only:
const uint32_t color_turquis    = 0xFF11BBBB;
const uint32_t color_yellow     = 0xFFBBBB11;

// default is green
const uint32_t color_mono        = 0xFF2BD84A;

static const int ScreenBitmapSize = (PixelWidth * PixelHeight * 4);

const uint8_t color_R = 2;
const uint8_t color_G = 1;
const uint8_t color_B = 0;
const uint8_t color_A = 3;

//static uint32_t pixelsSRGB[blockRows * blockCols];
uint32_t * pixelsSRGB;
static uint8_t _blockChanged[blockRows * blockCols];
uint8_t * blockChanged = _blockChanged;
static uint8_t _shadowScreen[PageSize];
uint8_t * shadowScreen = _shadowScreen;

int was = 0;


void initHiResLineAddresses() {
    int i = 0;
    
    for( int x = 0; x <= 0x50; x += 0x28 ) {
        for( int y = 0; y <= 0x380; y += 0x80 ) {
            for( int z = 0; z <= 0x1C00; z += 0x400 ) {
                HiResLineAddrTbl[i++] = x + y + z;
            }
        }
    }
}


void init() {
    initHiResLineAddresses();
}


void hires_clearChanges() {
    memset(_blockChanged, 0, sizeof(_blockChanged));
}


void hires_renderMono() {
    int height = PixelHeight;
    
    // do not even render it...
    if( videoMode.text == 1 ) {
        return;
    }
    else {
        if( videoMode.mixed == 1 ) {
            height = MixedHeight;
        }
        if( MEMcfg.txt_page_2 == 1 ) {
            HiResBufferPointer = (uint8_t*)HiResBuffer2;
        }
        else {
            HiResBufferPointer = (uint8_t*)HiResBuffer1;
        }
    }
    
    int pixelAddr = 0;

    hires_clearChanges();
    
    for( int y = 0;  y < height; y++ ) {
        int lineAddr = HiResLineAddrTbl[y];
        
        int blockVertIdx = y / blockHeight * blockCols;
        
        for( int blockHorIdx = 0; blockHorIdx < blockCols; blockHorIdx++ ) {
            int block = HiResBufferPointer[ lineAddr + blockHorIdx ];
            int screenIdx = y * blockCols + blockHorIdx;
            
            // get all changed blocks
            _blockChanged[ blockVertIdx + blockHorIdx ] |= _shadowScreen[ screenIdx ] != block;
            _shadowScreen[ screenIdx ] = block;
            
            for( int bit = 0; bit <= 6; bit++ ) {
                uint8_t bitMask = 1 << bit;
                if ( (block & bitMask) ) {
                    pixelsSRGB[pixelAddr] = color_mono;
                }
                else {
                    pixelsSRGB[pixelAddr] = color_black;
                }
                
                pixelAddr++;
            }
        }
    }
    
//    refreshChanged(blockSize: 1)
}



