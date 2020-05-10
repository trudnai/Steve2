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
#define SPKR_LEVEL_MAX      127

extern void spkr_init(void);
extern void spkr_exit(void);
extern void spkr_play(void);

extern const unsigned spkr_buf_size;
extern char spkr_samples [];
extern unsigned spkr_sample_idx;
extern int spkr_level;



#endif /* speaker_h */
