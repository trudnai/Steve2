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


const uint8_t phy2log_dos33[16] = {
    0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15
};
const uint8_t log2phy_dos33[16] = {
    0, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2, 15
};

const uint8_t phy2log_pascal[16] = {
    0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15
};
const uint8_t log2phy_pascal[16] = {
    0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15
};

const uint8_t phy2log_cpm[16] = {
    0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5
};
const uint8_t log2phy_cpm[16] = {
    0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13
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
            spkr_play_disk_ioerr();
        }
        else
        if( disk.phase.count > maxDiskPhaseNum ) {
            disk.phase.count = maxDiskPhaseNum;
            spkr_play_disk_ioerr();
        }
        else {
            spkr_play_disk_arm();
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

void disk_motor_on() {
    spkr_play_disk_motor();
    spkr_stop_disk_motor( -1 );
}

void disk_motor_off() {
    spkr_stop_disk_motor( 3 * fps ); // 3 second delay
}

uint8_t disk_read() {
    dbgPrintf("io_DISK_READ (S%u)\n", 6);
    disk.clk_last_access = m6502.clktime;
    disk_accelerator_speedup();

    // Debug disk read
//    spkr_toggle();

    return woz_read();
}

