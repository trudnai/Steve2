//
//  disk.c
//  A2Mac
//
//  Created by Tamas Rudnai on 2/15/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#include "disk.h"
#include "6502.h"
#include "common.h"
#include "woz.h"
#include "speaker.h"


disk_t disk = {
    { 0, 0, 0 },    // phase
    0,              // clk_since_last_read
};

const int diskAccelerator_frames  = 2;
int diskAccelerator_count = 0;
int diskAccelerator_speed = 25 * M / fps; // if less than actual CPU speed means no acceleration


// motor position from the magnet state
// -1 means invalid, not supported
const int magnet_to_Poistion[16] = {
//   0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111
       -1,   0,   2,   1,   4,  -1,   3,  -1,   6,   7,  -1,  -1,   5,  -1,  -1,  -1
};

const int position_to_direction[8][8] = {
//     N  NE   E  SE   S  SW   W  NW
//     0   1   2   3   4   5   6   7
    {  0,  1,  2,  3,  0, -3, -2, -1 }, // 0 N
    { -1,  0,  1,  2,  3,  0, -3, -2 }, // 1 NE
    { -2, -1,  0,  1,  2,  3,  0, -3 }, // 2 E
    { -3, -2, -1,  0,  1,  2,  3,  0 }, // 3 SE
    {  0, -3, -2, -1,  0,  1,  2,  3 }, // 4 S
    {  3,  0, -3, -2, -1,  0,  1,  2 }, // 5 SW
    {  2,  3,  0, -3, -2, -1,  0,  1 }, // 6 W
    {  1,  2,  3,  0, -3, -2, -1,  0 }, // 7 NW
};


void disk_accelerator_speedup() {
    if ( diskAccelerator_speed >= clk_6502_per_frm ) {
        clk_6502_per_frm =
        clk_6502_per_frm_max = diskAccelerator_speed;  // clk_6502_per_frm_diskAccelerator;
        diskAccelerator_count = diskAccelerator_frames;
    }
}

void disk_phase() {

    int position = magnet_to_Poistion[disk.phase.magnet];
    if ( position >= 0 ) {
        int lastPosition = disk.phase.count & 7;
        int direction = position_to_direction[lastPosition][position];
    
        disk.phase.count += direction;
        if( disk.phase.count < minDiskPhaseNum ) {
            disk.phase.count = minDiskPhaseNum;
        }
        else
        if( disk.phase.count > maxDiskPhaseNum ) {
            disk.phase.count = maxDiskPhaseNum;
        }
        
        // TODO: Add track positioning sfx
//        spkr_toggle();
        
//        printf("Head Position: p:%d d:%d l:%d: ph:%u)\n", position, direction, lastPosition, disk.phase.count);
                
        disk.clk_last_access = m6502.clktime;
        disk_accelerator_speedup();
    }
    else {
        // invalid magnet config
    }
    
//    printf("\n");
}


void disk_phase_on( uint8_t currentMagnet ) {
    disk.phase.magnet |= 1 << currentMagnet;
    disk_phase();
}


void disk_phase_off( uint8_t currentMagnet ) {
    disk.phase.magnet &= ~(1 << currentMagnet);
    disk_phase();
}


uint8_t disk_read() {
    dbgPrintf("io_DISK_READ (S%u)\n", 6);
    disk.clk_last_access = m6502.clktime;
    disk_accelerator_speedup();

    return woz_read();
}

