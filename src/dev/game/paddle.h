//
//  paddle.h
//  Steve ][
//
//  Created by Tamas Rudnai on 5/19/20.
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

#ifndef __PADDLE_H__
#define __PADDLE_H__

#include "6502.h"

// TODO: We may need a joystick calibration
#define PDL_MAX_TIME 3072.0 // 3300.0
#define PDL_TIME_DECAY ( 255.0 / PDL_MAX_TIME )

extern uint64_t pdl_reset_time;
extern uint64_t pdl_current_time;
extern uint64_t pdl_elapsed_time;

extern double pdl_value[4];
extern double pdl_prev[4];
extern double pdl_diff[4];
extern double * pdl_valarr;
extern double * pdl_prevarr;
extern double * pdl_diffarr;


INLINE uint8_t pdl_reset() {
    pdl_reset_time = m6502.clktime + m6502.clkfrm;
    return 0;
}


INLINE uint8_t pdl_read( uint8_t pdl ) {
    pdl_current_time = m6502.clktime + m6502.clkfrm;
    pdl_elapsed_time = pdl_current_time - pdl_reset_time;
    double normalized_time = pdl_elapsed_time / PDL_MAX_TIME; // 0: started, >= 1 ended
    return normalized_time >= pdl_value[pdl] ? 0 : 1 << 7; // TODO: better pdl value simulation, not only the bit 7
}


#endif /* __PADDLE_H__ */
