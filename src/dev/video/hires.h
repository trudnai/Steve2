//
//  hires.h
//  A2Mac
//
//  Created by Tamas Rudnai on 5/28/21.
//  Copyright © 2021 GameAlloy. All rights reserved.
//

#ifndef hires_h
#define hires_h

#include <stdio.h>

extern uint32_t * pixelsSRGB;
extern uint8_t * blockChanged;
extern uint8_t * shadowScreen;

extern void hires_clearChanges(void);
extern void hires_renderMono(void);


#endif /* hires_h */
