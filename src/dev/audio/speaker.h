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

#define SPKR_LEVEL_MIN      0
#define SPKR_LEVEL_MAX      255
#define SPKR_LEVEL_ZERO     ( SPKR_LEVEL_MIN + SPKR_LEVEL_MAX / 2 )

extern void spkr_init(void);
extern void spkr_exit(void);
extern void spkr_update(void);

extern const unsigned spkr_sample_rate;
extern const unsigned spkr_buf_size;
extern const int spkr_fps;
extern char spkr_samples [];
extern unsigned spkr_sample_idx;
extern int spkr_level;
extern int freeBuffers;
extern unsigned spkr_extra_buf;

extern const unsigned spkr_play_timeout;
extern unsigned spkr_play_time;



#endif /* speaker_h */
