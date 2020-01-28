//
//  woz1.h
//  A2Mac
//
//  Created by Tamas Rudnai on 11/18/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#ifndef woz_h
#define woz_h

#define DISKII_MAXTRACKS 80
#define DISKII_PHASES 4


#pragma pack(push)
#pragma pack(1)

typedef struct woz_header_s {
    uint32_t    magic;
    uint8_t     no7;
    char        lineend [3];
    uint32_t    crc;
} woz_header_t;

typedef struct woz_chunk_header_s {
    uint32_t    magic;
    uint32_t    size;
} woz_chunk_header_t;

// chunk data only
typedef struct woz_info_s {
    uint8_t     version;
    uint8_t     disk_type;              // 1 = 5.25, 2 = 3.5
    uint8_t     is_write_protected;
    uint8_t     sync;                   // 1 = Cross track sync
    uint8_t     cleaned;                // 1 = MC3470 fake bits removed
    char        creator [32];           // Name of software created this file (UTF-8, 0x20 padded, NOT zero terminated)
} woz_info_t;

// chunk data only
typedef struct woz_tmap_s {
    uint8_t     phase [DISKII_MAXTRACKS * DISKII_PHASES];
} woz_tmap_t;

#define WOZ_TRACK_BYTE_COUNT 6646
// chunk data only

typedef struct woz_track_s {
    uint8_t     data [WOZ_TRACK_BYTE_COUNT];
    uint16_t    bytes_used;
    uint16_t    bit_count;
    uint16_t    splice_point;
    uint8_t     splice_nibble;
    uint8_t     splice_bit_count;
    uint16_t    reserved;
} woz_track_t;


// chunk data only
typedef woz_track_t woz_trks_t[DISKII_MAXTRACKS];

#pragma pack(pop)

#endif /* woz_h */
