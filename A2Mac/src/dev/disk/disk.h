//
//  disk.h
//  A2Mac
//
//  Created by Tamas Rudnai on 2/15/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#ifndef disk_h
#define disk_h

#include "common.h"


#define minDiskTrackNum      0
#define maxDiskTrackNum     39
#define minDiskPhaseStates   8      // 4 quarters * 2 because of two neighbouring magnets can be activated at the same time which leaves motor in a half quarter movement
#define minDiskPhaseNum      0
#define maxDiskPhaseNum     (minDiskPhaseStates * maxDiskTrackNum)

typedef struct phase_s {
    uint8_t lastMagnet  : 4;
    uint8_t magnet      : 4;
    int     count;
} phase_t;

typedef struct disk_s {
    phase_t phase;
    uint64_t clk_since_last_read;
} disk_t;

extern disk_t disk;


// Magnet States --> Stepper Motor Position
//
//                N
//               0001
//        NW      |      NE
//       1001     |     0011
//                |
// W 1000 ------- o ------- 0010 E
//                |
//       1100     |    0110
//        SW      |     SE
//               0100
//                S

extern const int magnet_to_Poistion[16];
extern const int position_to_direction[8][8];
extern const unsigned long long clk_6502_per_frm_diskAccelerator;
extern const unsigned long long clk_diskAcceleratorTimeout;

extern void disk_phase(void);
extern uint8_t disk_read(void);


#endif /* disk_h */
