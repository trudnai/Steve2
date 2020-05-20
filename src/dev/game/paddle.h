//
//  paddle.h
//  A2Mac
//
//  Created by Tamas Rudnai on 5/19/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#ifndef paddle_h
#define paddle_h

#include "6502.h"

#define PDL_MAX_TIME 3300.0
#define PDL_TIME_DECAY ( 255.0 / PDL_MAX_TIME )

uint64_t pdl_reset_time = 0;
uint64_t pdl_current_time = 0;
uint64_t pdl_elapsed_time = 0;

float pdl_value[4] = { 0.5, 0.0, 1.0, 0.1 };

INLINE uint8_t pdl_reset() {
    pdl_reset_time = m6502.clktime + clkfrm;
    return 0;
}


INLINE uint8_t pdl_read( uint8_t pdl ) {
    pdl_current_time = m6502.clktime + clkfrm;
    pdl_elapsed_time = pdl_current_time - pdl_reset_time;
    
    double normalized_time = pdl_elapsed_time / PDL_MAX_TIME; // 0: started, >= 1 ended

//    printf("PDL%d: %d\n", pdl, (uint8_t) ( pdl_value[pdl] * 512 * ( 1 - (pdl_elapsed_time / PDL_MAX_TIME) )) );

//    return (uint8_t) ( pdl_value[pdl] * 512 * ( 1 - (pdl_elapsed_time / PDL_MAX_TIME) ) );
    return normalized_time >= pdl_value[pdl] ? 0 : 0x80;
}

#endif /* paddle_h */
