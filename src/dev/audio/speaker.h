//
//  speaker.h
//  Steve ][
//
//  Created by Tamas Rudnai on 5/9/20.
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

#ifndef speaker_h
#define speaker_h

#include <stdio.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

// very loud
//#define SPKR_LEVEL_MIN      (-28000)
//#define SPKR_LEVEL_MAX      (+28000)

// loud
//#define SPKR_LEVEL_MIN      (-5000)
//#define SPKR_LEVEL_MAX      (+5000)

// medium
#define SPKR_LEVEL_MIN      (-3072)
#define SPKR_LEVEL_MAX      (+3072)

#define SPKR_FADE_LEADING_EDGE      0.32
#define SPKR_FADE_TRAILING_EDGE     0.16
#define SPKR_INITIAL_DUMPING        0.90

// quiet
//#define SPKR_LEVEL_MIN      (-1000)
//#define SPKR_LEVEL_MAX      (+1000)

#define SPKR_LEVEL_ZERO     0 // as defined in OpenAL documentation for 8bit PCM


enum {
    SPKR_SRC_GAME_SFX = 0,
    SPKR_SRC_DISK_MOTOR_SFX,
    SPKR_SRC_DISK_ARM_SFX,
    SPKR_SRC_DISK_IOERR_SFX,
};


extern const unsigned spkr_sample_rate;
extern const unsigned spkr_buf_alloc_size;
extern unsigned spkr_buf_size;
extern const unsigned spkr_fps;
extern unsigned spkr_fps_divider;
extern int16_t spkr_samples [];
extern unsigned spkr_sample_idx;
extern int spkr_level;
extern int freeBuffers;
extern int spkr_extra_buf;

extern unsigned spkr_play_timeout;
extern unsigned spkr_play_time;

extern float spkr_vol;

extern void spkr_vol_up(void);
extern void spkr_vol_dn(void);
extern void spkr_mute(void);

extern void spkr_init(void);
extern void spkr_exit(void);
extern void spkr_update(void);
extern void spkr_toggle(void);

extern void spkr_load_sfx( const char * bundlePath );

extern void spkr_play_disk_motor(void);
extern void spkr_stop_disk_motor( int time );
extern void spkr_update_disk_sfx(void);
extern void spkr_stop_sfx( ALuint src );
extern void spkr_stopAll(void);

extern void spkr_play_disk_arm(void);
extern void spkr_play_disk_ioerr(void);

#endif /* speaker_h */
