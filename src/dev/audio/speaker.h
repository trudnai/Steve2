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

#define SPKR_LEVEL_ZERO     0 // as defined in OpenAL documentation for 8bit PCM

// very loud
//#define SPKR_LEVEL_MIN      (-28000)
//#define SPKR_LEVEL_MAX      (+28000)

// loud
//#define SPKR_LEVEL_MIN      (-5000)
//#define SPKR_LEVEL_MAX      (+5000)

// medium
//#define SPKR_LEVEL_MIN      (-3072)
//#define SPKR_LEVEL_MAX      (+3072)
#define SPKR_LEVEL_MAX      32767 // 8192
#define SPKR_LEVEL_MIN      (-SPKR_LEVEL_MAX)
#define SPKR_PLAY_TIMEOUT   8U
#define SPKR_PLAY_QUIET     0
//#define SPKR_PLAY_QUIET     (SPKR_PLAY_TIMEOUT - 2)

// quiet
//#define SPKR_LEVEL_MIN      (-1000)
//#define SPKR_LEVEL_MAX      (+1000)


//#define SPKR_FADE_LEADING_EDGE      0.64
//#define SPKR_FADE_TRAILING_EDGE     0.32
//#define SPKR_INITIAL_LEADING_EDGE   0.82 // leading edge should be pretty steep to get sharp sound plus to avoid Wavy Navy high pitch sound
//#define SPKR_INITIAL_TRAILING_EDGE  0.64 // need a bit of slope to get Xonix sound good

extern float SPKR_FADE_LEADING_EDGE;
extern float SPKR_FADE_TRAILING_EDGE;
extern float SPKR_INITIAL_LEADING_EDGE; // leading edge should be pretty steep to get sharp sound plus to avoid Wavy Navy high pitch sound
extern float SPKR_INITIAL_TRAILING_EDGE; // need a bit of slope to get Xonix sound good

extern int spkr_ema_len;


#define SPKR_SAMPLE_PWM_THRESHOLD   32    // to detect PWM controlled speaker control like in Wavy Navy or Xonix



enum {
    SPKR_SRC_GAME_SFX = 0,
    SPKR_SRC_DISK_MOTOR_SFX,
    SPKR_SRC_DISK_ARM_SFX,
    SPKR_SRC_DISK_IOERR_SFX,
    SOURCES_COUNT
};

typedef int16_t spkr_sample_t;

extern const unsigned spkr_sample_rate;
extern const unsigned spkr_buf_alloc_size;
extern const unsigned spkr_buf_size;
extern const unsigned spkr_fps;
extern unsigned spkr_fps_divider;
extern spkr_sample_t * spkr_samples;
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
