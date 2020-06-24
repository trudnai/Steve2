//
//  speaker.h
//  A2Mac
//
//  Created by Tamas Rudnai on 5/9/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
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
#define SPKR_LEVEL_MIN      (-3000)
#define SPKR_LEVEL_MAX      (+3000)

// quiet
//#define SPKR_LEVEL_MIN      (-1000)
//#define SPKR_LEVEL_MAX      (+1000)

#define SPKR_LEVEL_ZERO     0 // as defined in OpenAL documentation for 8bit PCM


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


extern void spkr_init(void);
extern void spkr_exit(void);
extern void spkr_update(void);
extern void spkr_toggle(void);

extern void spkr_load_sfx( const char * bundlePath );

extern void spkr_play_disk_motor(void);
extern void spkr_stop_disk_motor( int time );
extern void spkr_update_disk_sfx(void);

extern void spkr_play_disk_arm(void);
extern void spkr_play_disk_ioerr(void);

#endif /* speaker_h */
