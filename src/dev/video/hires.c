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


