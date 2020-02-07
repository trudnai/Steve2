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

#define PG 256ULL
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

INLINE void set_flags_N( const uint8_t test ) {
    m6502.N = BITTEST(test, 7);
}

INLINE void set_flags_V( const uint8_t test ) {
    m6502.V = BITTEST(test, 6);
}

INLINE void set_flags_Z( const uint8_t test ) {
    m6502.Z = test == 0;
}

INLINE void set_flags_C( const int16_t test ) {
    m6502.C = test >= 0;
}

INLINE void set_flags_NZ( const uint8_t test ) {
    set_flags_N(test);
    set_flags_Z(test);
}

INLINE void set_flags_NV( const uint8_t test ) {
    set_flags_N(test);
    set_flags_V(test);
}

INLINE void set_flags_NVZ( const uint8_t test ) {
    set_flags_NZ(test);
    set_flags_V(test);
}

INLINE void set_flags_NZC( const int16_t test ) {
    set_flags_NZ(test);
    set_flags_C(test);
}

//INLINE void set_flags_NZCV( int test ) {
//    set_flags_NZC(test);
//    set_flags_V(test);
//}




#endif // __COMMON_H__
