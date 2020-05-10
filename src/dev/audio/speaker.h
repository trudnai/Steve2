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


extern void spkr_init(void);
extern void spkr_exit(void);
extern void spkr_play(void);

extern const unsigned spkr_buf_size;
extern char spkr_samples [];


#endif /* speaker_h */
