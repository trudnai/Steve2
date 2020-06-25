//
//  woz.h
//  A2Mac
//
//  Created by Tamas Rudnai on 2/15/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#ifndef woz_h
#define woz_h

#include <stdio.h>
#include "common.h"


#define DISKII_MAXTRACKS 80
#define DISKII_PHASES 4

#define WOZ1_MAGIC  0x315A4F57
#define WOZ2_MAGIC  0x325A4F57
#define WOZ_INFO_CHUNK_ID  0x4F464E49
#define WOZ_TMAP_CHUNK_ID  0x50414D54
#define WOZ_TRKS_CHUNK_ID  0x534B5254
#define WOZ_META_CHUNK_ID  0x4154454D

#define WOZ_ERR_OK               0
#define WOZ_ERR_FILE_NOT_FOUND  -1
#define WOZ_ERR_NOT_WOZ_FILE    -2
#define WOZ_ERR_BAD_CHUNK_HDR   -3
#define WOZ_ERR_BAD_DATA        -4


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



#define __NO__WOZ_REAL_SPIN2

#ifdef WOZ_REAL_SPIN

typedef union {
    struct {
        uint8_t next;
        uint8_t data;
        uint8_t prev;
        uint8_t shift;
    };
    struct {
        uint32_t lower : 31;
        uint32_t valid : 1;
    };
    uint32_t shift32;
} WOZread_t;

#else // WOZ_REAL_SPIN

typedef union {
    struct {
        uint8_t data;
        uint8_t latch;
        
        // for debug and diag purposes
        uint8_t out[6];
    };
    struct {
        uint16_t lower15 : 15;
        uint16_t valid : 1;
    };
    uint64_t shift;
} WOZread_t;

#endif // WOZ_REAL_SPIN


extern WOZread_t WOZread;
extern uint8_t   WOZlatch;

//extern woz_header_t woz_header;
//extern woz_chunk_header_t woz_chunk_header;
//extern woz_tmap_t woz_tmap;
//extern woz_trks_t woz_trks;


extern uint8_t woz_read(void);
extern void woz_write( uint8_t data );
extern int woz_loadFile( const char * filename );
extern int woz_saveFile( const char * filename );

#endif /* woz_h */
