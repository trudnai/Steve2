//
//  common.h
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef __COMMON_H__
#define __COMMON_H__

#include "6502.h"


#define K 1000ULL
#define M (K * K)
#define G (M * K)
#define T (G * K)

#define KB 1024ULL
#define MB (KB * KB)
#define GB (MB * KB)
#define TB (GB * KB)


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


#define BITTEST(n,x) ((bits_t)(n)).b##x

static inline void set_flags_N( uint8_t test ) {
    m6502.flags.N = BITTEST(test, 7);
    dbgPrintf("%c", m6502.flags.N ? 'N' : 'n');
}

static inline void set_flags_V( uint8_t test ) {
    m6502.flags.V = BITTEST(test, 6);
    dbgPrintf("%c", m6502.flags.V ? 'V' : 'v');
}

static inline void set_flags_Z( uint8_t test ) {
    m6502.flags.Z = test == 0;
    dbgPrintf("%c", m6502.flags.Z ? 'Z' : 'z');
}

static inline void set_flags_C( int test ) {
    m6502.flags.C = test >= 0;
    dbgPrintf("%c", m6502.flags.C ? 'C' : 'c');
}

static inline void set_flags_NZ( uint8_t test ) {
    set_flags_N(test);
    set_flags_Z(test);
}

static inline void set_flags_NVZ( uint8_t test ) {
    set_flags_NZ(test);
    set_flags_V(test);
}

static inline void set_flags_NZC( int test ) {
    set_flags_NZ(test);
    set_flags_C(test);
}

static inline void set_flags_NZCV( int test ) {
    set_flags_NZC(test);
    set_flags_V(test);
}




#endif // __COMMON_H__
