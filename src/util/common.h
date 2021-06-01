//
//  common.h
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <unistd.h>


#ifdef DEBUG
#define INLINE
#else
#define INLINE static __attribute__((always_inline))
#endif


#ifdef DEBUG
//#define dbgPrintf(format, ...) if(outdev) fprintf(outdev, format, ## __VA_ARGS__)
#define dbgPrintf(format, ...)
#define dbgPrintf2(format, ...) if(outdev) fprintf(outdev, format, ## __VA_ARGS__)
#else
#define dbgPrintf(format, ...)
#define dbgPrintf2(format, ...)
#endif


#define K 1000ULL
#define M (K * K)
#define G (M * K)
#define T (G * K)

#define PG 256ULL
#define KB 1024ULL
#define MB (KB * KB)
#define GB (MB * KB)
#define TB (GB * KB)


// calculates number of cycles in one frame
#define MHZ(mhz) ( (double)( (mhz) * M ) )
#define FRAME(mhz) ( MHZ(mhz) / fps )
#define FRAME_INIT(mhz) ( MHZ(mhz) / DEFAULT_FPS )


typedef
union {
    uint8_t bits;
    struct {
        uint8_t b0:1;
        uint8_t b1:1;
        uint8_t b2:1;
        uint8_t b3:1;
        uint8_t b4:1;
        uint8_t b5:1;
        uint8_t b6:1;
        uint8_t b7:1;
    };
} bits_t;


//#define BITTEST(n,x) ((bits_t)(n)).b##x
//#define BITTEST(n,x) (( (n) >> (x) ) & 1)
#define BITTEST(n,x) ( (n) & (1 << (x)) )


extern FILE * outdev;


#endif // __COMMON_H__
