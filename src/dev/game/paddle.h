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

// TODO: We may need a joystick calibration
#define PDL_MAX_TIME 3072.0 // 3300.0
#define PDL_TIME_DECAY ( 255.0 / PDL_MAX_TIME )

uint64_t pdl_reset_time = 0;
uint64_t pdl_current_time = 0;
uint64_t pdl_elapsed_time = 0;

double pdl_value[4] = { 0.5, 0.5, 0.5, 0.5 };
double pdl_prev[4] = { 0.5, 0.5, 0.5, 0.5 };
double pdl_diff[4] = { 0.0, 0.0, 0.0, 0.0 };
double * pdl_valarr = pdl_value;
double * pdl_prevarr = pdl_prev;
double * pdl_diffarr = pdl_diff;

INLINE uint8_t pdl_reset() {
    pdl_reset_time = m6502.clktime + clkfrm;
    return 0;
}


INLINE uint8_t pdl_read( uint8_t pdl ) {
    pdl_current_time = m6502.clktime + clkfrm;
    pdl_elapsed_time = pdl_current_time - pdl_reset_time;
    double normalized_time = pdl_elapsed_time / PDL_MAX_TIME; // 0: started, >= 1 ended
    return normalized_time >= pdl_value[pdl] ? 0 : 1 << 7;
}

#endif /* paddle_h */
