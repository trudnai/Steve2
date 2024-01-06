//
//  woz.h
//  Steve ][
//
//  Created by Tamas Rudnai on 2/15/20.
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

#ifndef woz_h
#define woz_h

#include <stdio.h>
#include "common.h"


#define MAXFILENAME 4096

#define WOZ1_FILE_SIZE 233216

#define DISKII_MAXTRACKS 80
#define DISKII_PHASES 4

#define WOZ1_MAGIC  0x315A4F57
#define WOZ2_MAGIC  0x325A4F57
#define WOZ_INFO_CHUNK_ID  0x4F464E49
#define WOZ_TMAP_CHUNK_ID  0x50414D54
#define WOZ_TRKS_CHUNK_ID  0x534B5254
#define WOZ_META_CHUNK_ID  0x4154454D
#define WOZ_WRIT_CHUNK_ID  0x54495257

#define WOZ_ERR_OK               0
#define WOZ_ERR_FILE_NOT_FOUND  -1
#define WOZ_ERR_NOT_WOZ_FILE    -2
#define WOZ_ERR_BAD_CHUNK_HDR   -3
#define WOZ_ERR_BAD_DATA        -4


#pragma pack(push)
#pragma pack(1)

typedef struct woz_header_s {
    uint32_t    magic;
    union {
        struct {
            uint8_t     no7;
            char        lineend [3];
        };
        uint32_t    bit_correctness;
    };
    uint32_t    crc;
} woz_header_t;

typedef struct woz_chunk_header_s {
    uint32_t    magic;
    uint32_t    size;
} woz_chunk_header_t;

// chunk data only
typedef struct woz_info_s {
    uint8_t     version;                // Version number of the INFO chunk.
                                        //      WOZ1 version is 1
                                        //      WOZ2 version is 2
    uint8_t     disk_type;              // 1 = 5.25, 2 = 3.5
    uint8_t     is_write_protected;     // 1 = Floppy is write protected
    uint8_t     sync;                   // 1 = Cross track sync
    uint8_t     cleaned;                // 1 = MC3470 fake bits removed
    char        creator [32];           // Name of software created this file (UTF-8, 0x20 padded, NOT zero terminated)
    uint8_t     disk_sides;             // The number of disk sides contained within this image. A 5.25 disk will always be 1. A 3.5 disk can be 1 or 2.
    uint8_t     boot_sec_format;        // The type of boot sector found on this disk. This is only for 5.25 disks. 3.5 disks should just set this to 0.
                                        //      0 = Unknown
                                        //      1 = Contains boot sector for 16-sector
                                        //      2 = Contains boot sector for 13-sector
                                        //      3 = Contains boot sectors for both
    uint8_t     opt_bit_timing;         // The ideal rate that bits should be delivered to the disk controller card.
                                        // This value is in 125 nanosecond increments, so 8 is equal to 1 microsecond.
                                        // And a standard bit rate for a 5.25 disk would be 32 (4µs).
    uint16_t    compatible_hw;          // Bit field with a 1 indicating known compatibility. Multiple compatibility flags are possible.
                                        // A 0 value represents that the compatible hardware list is unknown.
                                        //      0x0001 = Apple ][
                                        //      0x0002 = Apple ][ Plus
                                        //      0x0004 = Apple //e (unenhanced)
                                        //      0x0008 = Apple //c
                                        //      0x0010 = Apple //e Enhanced
                                        //      0x0020 = Apple IIgs
                                        //      0x0040 = Apple //c Plus
                                        //      0x0080 = Apple ///
                                        //      0x0100 = Apple /// Plus
    uint16_t    required_ram;           // Minimum RAM size needed for this software. This value is in K (1024 bytes).
                                        // If the minimum size is unknown, this value should be set to 0. So, a requirement
                                        // of 64K would be indicated by the value 64 here.
    uint16_t    largest_track;          // The number of blocks (512 bytes) used by the largest track.
                                        // Can be used to allocate a buffer with a size safe for all tracks.
} woz_info_t;

// chunk data only
typedef struct woz_tmap_s {
    uint8_t     phase [DISKII_MAXTRACKS * DISKII_PHASES];
} woz_tmap_t;

#define WOZ1_TRACK_BYTE_COUNT 6646
#define WOZ2_TRACK_BYTE_COUNT 6680

// chunk data only

typedef struct woz1_track_s {
    uint8_t     data [WOZ1_TRACK_BYTE_COUNT];
    uint16_t    bytes_used;
    uint16_t    bit_count;
    uint16_t    splice_point;
    uint8_t     splice_nibble;
    uint8_t     splice_bit_count;
    uint16_t    reserved;
} woz1_track_t;


typedef struct woz2_trk_s {
    uint16_t    starting_block;     // First block of BITS data. This value is relative to the start of the file,
                                    // so the first possible starting block is 3. Multiply this value by 512 (x << 9)
                                    // to get the starting byte of the BITS data
    uint16_t    block_count;        // Number of blocks for this BITS data
    uint32_t    bit_count;          // The number of bits in the bitstream
} woz2_trk_t;

#define WOZ_MAX_TRK     160

typedef struct woz2_track_s {
    uint8_t     trks [WOZ_MAX_TRK];
    uint16_t    bytes_used;
    uint16_t    bit_count;
    uint16_t    splice_point;
    uint8_t     splice_nibble;
    uint8_t     splice_bit_count;
    uint16_t    reserved;
} woz2_track_t;

// chunk data only
typedef woz1_track_t woz1_trks_t[DISKII_MAXTRACKS];

#pragma pack(pop)



typedef union {
    struct {
        // for buffering for shifting during write process
        // also for debug and diag purposes
        uint8_t next_next;
        uint8_t next;
        
        uint8_t data;
        uint8_t latch;
        
        // for buffering for shifting during write process
        // also for debug and diag purposes
        uint8_t out[4];
    };
    struct {
        uint16_t next16; // unused
        uint16_t lower15 : 15;
        uint16_t valid : 1;
        uint32_t out32; // unused
    };
    uint64_t shift;
} WOZread_t;


typedef struct woz_flags_s {
    uint8_t     image_loaded : 1;
    uint8_t     image_file_readonly : 1;
    uint8_t     disk_write_protected : 1;
    uint8_t     disk_modified : 1;
} woz_flags_t;


extern WOZread_t WOZread;
extern WOZread_t WOZwrite;
extern uint8_t   WOZlatch;
extern char woz_filename[MAXFILENAME];
extern woz_flags_t woz_flags;
extern size_t woz_file_size;
extern uint8_t * woz_file_buffer;

extern int extraForward;

//extern woz_header_t woz_header;
//extern woz_chunk_header_t woz_chunk_header;
//extern woz_tmap_t woz_tmap;
//extern woz_trks_t woz_trks;


extern uint8_t woz_read(void);
extern void woz_write( uint8_t data );
extern int woz_parseBuffer(void);
extern int woz_loadFile( const char * filename );
extern int woz_saveFile( const char * filename );
extern void woz_eject(void);
extern uint8_t woz_is_loaded(void);


#endif /* woz_h */
