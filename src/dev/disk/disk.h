//
//  disk.h
//  A2Mac
//
//  Created by Tamas Rudnai on 2/15/20.
//  Copyright © 2019,2020 Tamas Rudnai. All rights reserved.
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

#ifndef disk_h
#define disk_h

#include "../../util/common.h"


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
    uint64_t clk_last_access;
    uint64_t clk_last_read;
    uint8_t drive;
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

extern const int diskAccelerator_frames;    // number of frames disk acceleration will be on
extern int diskAccelerator_count;           // counter for the disk acceleration frames
extern int diskAccelerator_speed;           // if less than the actual CPU speed, no acceleration
extern int diskAccelerator_enabled;
extern int disk_sfx_enabled;

extern void disk_phase(void);
extern void disk_phase_on( uint8_t currentMagnet );
extern void disk_phase_off( uint8_t currentMagnet );
extern uint8_t disk_read(void);

extern void disk_motor_on(void);
extern void disk_motor_off(void);

extern void disk_accelerator_speedup(void);


#endif /* disk_h */
