//
//  common.h
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
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
#define MHZ(mhz) ( (mhz) * M )
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
