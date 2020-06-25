//
//  woz.c
//  A2Mac
//
//  Created by Tamas Rudnai on 2/15/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "woz.h"
#include "disk.h"
#include "6502.h"
#include "common.h"


WOZread_t WOZread = {0};
WOZread_t WOZwrite = {0};


int trackWRoffset = 0;
int trackOffset = 0;
int bitOffset = 0;
uint64_t clkelpased;


size_t woz_file_size = 0;
uint8_t * woz_file_buffer = NULL;
woz_header_t * woz_header;
woz_chunk_header_t * woz_chunk_header;
woz_tmap_t * woz_tmap;
woz_trks_t * woz_trks;
int track_loaded = -1;



#pragma pack(push, 1)

typedef union trackEntry_u {
    struct {
        uint8_t data;
        uint8_t shift;
    };
    uint16_t shift16;
} trackEntry_t;

#pragma pack(pop)

trackEntry_t prepared_track[WOZ_TRACK_BYTE_COUNT];

typedef enum readState_e {
    readNormal = 0,
    readHold,
} readState_t;

readState_t readState = readNormal;
uint8_t readLatch;


void woz_loadTrack_old( int track ) {
    trackEntry_t reg = {0};

    reg.shift = (*woz_trks)[track].data[0];
    reg.data =  (*woz_trks)[track].data[1];
    prepared_track[0] = reg;

    for ( int offs = 1; offs < WOZ_TRACK_BYTE_COUNT; offs++ ) {
        
        for ( int i = 0; i < 8; i++ ) {
            if (reg.shift & 0x80) {
                reg.shift = 0;
            }
            
            reg.shift16 <<= 1;
        }
        
        reg.data = (*woz_trks)[track].data[ (offs + 1) % WOZ_TRACK_BYTE_COUNT ];
        prepared_track[offs] = reg;
    }
}


typedef enum wozTrackState_e {
    wozTrackState_Start = 0,
    wozTrackState_D5,
    wozTrackState_D5_AA,
    wozTrackState_D5_AA_96,
    wozTrackState_vol1,
    wozTrackState_vol2,
    wozTrackState_trk1,
    wozTrackState_trk2,
    wozTrackState_sec1,
    wozTrackState_sec2,
    wozTrackState_END,
} wozTrackState_t;


int bitOffs_D5_SecHdr = 0;
int vol = 0;
int trk = 0;
int sec = 0;

wozTrackState_t woz_decodeTrkSec( uint8_t data, uint64_t clkelapsed, int bitOffs ) {
    static wozTrackState_t wozTrackState  = wozTrackState_Start;
    
    if ( clkelpased > 40 ) {
        // spent too much time on reading, we cannot reliably decode sector header
        wozTrackState = wozTrackState_Start;
    }
    
    switch (wozTrackState) {
        case wozTrackState_D5:
            switch (data) {
                case 0xAA:
//                    printf("D5 AA at bitOffset:%d\n", bitOffs);
                    wozTrackState = wozTrackState_D5_AA;
                    break;
                    
                default:
                    wozTrackState = wozTrackState_Start;
                    break;
            }
            
            break;
            
        case wozTrackState_D5_AA:
            switch (data) {
                case 0x96:
                    wozTrackState = wozTrackState_vol1;
//                    printf("D5 AA 96 at bitOffset:%d\n", bitOffs);
//                    printf("Sector Header at bitOffset:%d\n", bitOffs_D5_SecHdr);
                    break;
                    
                default:
                    wozTrackState = wozTrackState_Start;
                    break;
            }
            
            break;
            
        case wozTrackState_vol1:
            vol = (data << 1) | 1;
            wozTrackState = wozTrackState_vol2;
            break;
            
        case wozTrackState_vol2:
            vol &= data;
            wozTrackState = wozTrackState_trk1;
            break;
            
        case wozTrackState_trk1:
            trk = (data << 1) | 1;
            wozTrackState = wozTrackState_trk2;
            break;
            
        case wozTrackState_trk2:
            trk &= data;
            wozTrackState = wozTrackState_sec1;
            break;
            
        case wozTrackState_sec1:
            sec = (data << 1) | 1;
            wozTrackState = wozTrackState_sec2;
            break;
            
        case wozTrackState_sec2:
            sec &= data;
            wozTrackState = wozTrackState_END;
            
//            printf("Vol:%d Track:%d Sector:%d at bitOffset:%d\n", vol, trk, sec, bitOffs_D5_SecHdr);
            
            break;
            
        default:
            if ( data == 0xD5 ) {
//                printf("D5 at bitOffset:%d\n", bitOffs);
                wozTrackState = wozTrackState_D5;
                bitOffs_D5_SecHdr = bitOffs;
            }
            else {
                wozTrackState = wozTrackState_Start;
            }
            break;
    }
    
    return wozTrackState;
}


void woz_loadTrack( int track ) {
    trackEntry_t reg = {0};

    reg.shift = 0;
    reg.data  = 0;
    prepared_track[0] = reg;
    
    int bitOffs = 0;
    
    for ( int byteOffs = 0; byteOffs < WOZ_TRACK_BYTE_COUNT; byteOffs++ ) {

        reg.data = (*woz_trks)[track].data[ byteOffs ];

        for ( int i = 0; i < 8; i++ ) {
            reg.shift16 <<= 1;
            
            if (reg.shift & 0x80) {
                woz_decodeTrkSec( reg.shift, 0, bitOffs );
                reg.shift = 0;
            }
            
            bitOffs++;
        }
    }
}


uint8_t woz_read() {

    int track = woz_tmap->phase[disk.phase.count];
    if (outdev) fprintf(outdev, "track: %d (%d) ", track, disk.phase.count);
    if ( track >= 40 ) {
        dbgPrintf("TRCK TOO HIGH!\n");
        return rand();
    }
    
    static int clkBeforeSync = 0;
    
    clkelpased = m6502.clktime + clkfrm - m6502.clklast;
    m6502.clklast = m6502.clktime + clkfrm;
    
    clkBeforeSync += clkelpased;
    
    const int clkBeforeAdjusting = 512;
    const int magicShiftOffset = 45;
    
    uint16_t usedBytes = (*woz_trks)[track].bytes_used < WOZ_TRACK_BYTE_COUNT ? (*woz_trks)[track].bytes_used : WOZ_TRACK_BYTE_COUNT;
    
    if ( usedBytes ) {
        int shiftOffset = magicShiftOffset;
        
//        printf("elpased : %llu (clkBefRd:%d)\n", clkelpased, clkBeforeSync);
        
        if ( clkelpased > clkBeforeAdjusting ) {
//            printf("NEED SYNC : %llu (clkBefRd:%d)\n", clkelpased, clkBeforeSync);
            clkBeforeSync = 0;
            bitOffset = (clkelpased >> 2) & 7;
            trackOffset += clkelpased >> 5;
            if ( trackOffset >= usedBytes ) {
                bitOffset = 0;
                trackOffset = 0;
                WOZread.shift = 0;
                shiftOffset = 0;
            }
//            trackOffset %= usedBytes;

            // preroll data stream
            WOZread.shift = 0;
            WOZread.data = (*woz_trks)[track].data[trackOffset++];
            trackOffset %= usedBytes;
            trackWRoffset = trackOffset;

            WOZread.shift <<= bitOffset;
            WOZwrite = WOZread;

            for ( int i = 0; i < shiftOffset; i++ ) {
                for ( ; bitOffset < 8; bitOffset++ ) {
                    WOZread.shift <<= 1;
                    WOZwrite.shift <<= 1;

                    if ( WOZread.valid ) {
                        WOZread.latch = 0;
                    }
                }
                trackWRoffset = trackOffset;
                WOZwrite.data = WOZread.data = (*woz_trks)[track].data[trackOffset++];
                trackOffset %= usedBytes;
                bitOffset = 0;
            }
        }
        else {
            uint64_t bitForward = (clkelpased >> 2);

            // to avoid infinite loop and to search for bit 7 high
            for ( uint64_t i = 0; i < bitForward; i++ ) {
                if ( ++bitOffset >= 8 ) {
                    bitOffset = 0;
                    trackWRoffset = trackOffset;
                    trackOffset++;
                    trackOffset %= usedBytes;

                    WOZwrite.data = WOZread.data = (*woz_trks)[track].data[trackOffset];
                }

                WOZread.shift <<= 1;
                WOZwrite.shift <<= 1;

                if ( WOZread.valid ) {
                    WOZread.latch = 0;
                }
            }
        }

        // to avoid infinite loop and to search for bit 7 high
        for ( int i = 0; i < usedBytes * 8; i++ ) {
            if ( WOZread.valid ) {
                WOZread.latch = 0;
//                if (outdev) fprintf(outdev, "byte: %02X\n", byte);

                if ( woz_decodeTrkSec(WOZwrite.latch, clkelpased, trackOffset * 8 + bitOffset) == wozTrackState_END ) {
                    if (outdev) fprintf(outdev, "vol:%d trk:%d sec:%d\n", vol, trk, sec);
                }
                
                if (outdev) fprintf(outdev, "elpased:%lld  read: %02X\n", clkelpased, WOZwrite.latch);

                return WOZwrite.latch;
            }
            
            if ( ++bitOffset >= 8 ) {
                bitOffset = 0;
                trackWRoffset = trackOffset;
                trackOffset++;
                trackOffset %= usedBytes;

                WOZwrite.data = WOZread.data = (*woz_trks)[track].data[trackOffset];
            }
            
            WOZread.shift <<= 1;
            WOZwrite.shift <<= 1;
        }
//        if (outdev) fprintf(outdev, "TIME OUT!\n");
    }
    
    return rand();
}

void printbits ( uint8_t byte ) {
    fprintf(outdev, "%02X:", byte);
    
    for ( int bit = 7; bit >= 0; bit-- ) {
        fprintf(outdev, "%d", byte & (1 << bit) ? 1 : 0 );
    }
    fprintf(outdev, " ");
}

void printWozBuffer (const char * s, int n, WOZread_t WOZbuf ) {
    // for DEBUG ONLY!!!
    if (outdev) {
        fprintf(outdev, "%s (%d) ", s, n);
        
        for ( int i = 5; i >= 0; i-- ) {
            printbits(WOZbuf.out[i]);
        }
        
        printbits(WOZbuf.latch);
        printbits(WOZbuf.data);
        
        fprintf(outdev, "\n");
    }
}

void woz_write( uint8_t data ) {
    
    int track = woz_tmap->phase[disk.phase.count];
    if (outdev) fprintf(outdev, "track: %d (%d) ", track, disk.phase.count);
    if ( track >= 40 ) {
        dbgPrintf("TRACK TOO HIGH!\n");
        return;
    }
    
    clkelpased = m6502.clktime + clkfrm - m6502.clklast;
    m6502.clklast = m6502.clktime + clkfrm;
    
    uint16_t usedBytes = (*woz_trks)[track].bytes_used < WOZ_TRACK_BYTE_COUNT ? (*woz_trks)[track].bytes_used : WOZ_TRACK_BYTE_COUNT;
    
    if ( usedBytes ) {
        
        // for DEBUG ONLY!!!
        if (outdev) fprintf(outdev, "elpased:%llu  data:$%02X\n", clkelpased, data);
        printWozBuffer("*start", 0, WOZwrite);

        
        if ( clkelpased > 32 ) {
//            if (outdev) fprintf(outdev, "I/O ERROR : %llu\n", clkelpased);
        
            uint64_t bitForward = (clkelpased - 32) >> 2;
            
            // simulate disk spin over time
            while ( bitForward-- ) {
                if ( ++bitOffset >= 8 ) {
                    bitOffset = 0;
                    trackWRoffset = trackOffset;
                    trackOffset++;
                    trackOffset %= usedBytes;

//                    WOZwrite.data =
                    WOZread.data = (*woz_trks)[track].data[trackOffset];
                }

                WOZread.shift <<= 1;
                WOZwrite.shift <<= 1;

                if ( WOZread.valid ) {
                    WOZread.latch = 0;
                }
            }
        }
        
        
        // ok now we can latch data
        WOZwrite.data = WOZread.data = data;
        printWozBuffer("datain", 0, WOZwrite);

        int i = 8; // 8 bit to shift in
        
        // shift in 8 bits of data and write it out
        while ( i-- ) {
            if ( ++bitOffset >= 8 ) {
                // write out first part
                (*woz_trks)[track].data[trackWRoffset] = WOZwrite.latch;
                
                bitOffset = 0;
                trackWRoffset = trackOffset;
                trackOffset++;
                trackOffset %= usedBytes;

                // simulate shift in data (path of write latch is already loaded, we should not overwrite it!)
                uint8_t new = (*woz_trks)[track].data[trackOffset];
                new >>= i + 1;
                WOZread.data |= new;
//                WOZwrite.data |= new;

                printWozBuffer("shl1in", i, WOZwrite);
                WOZread.shift <<= 1;
                WOZwrite.shift <<= 1;
                break;
            }
            
            WOZread.shift <<= 1;
            WOZwrite.shift <<= 1;
            printWozBuffer("shift1", i, WOZwrite);
        };
        printWozBuffer("shift1", 9, WOZwrite);

        // write the remaining bits without altering WOZ track offsets and indexes
//        WOZread_t WOZtmp = WOZwrite;
//        int bo = bitOffset;
        
        // second half
        while ( i-- ) {
            if ( ++bitOffset >= 8 ) {
                // write out first part
                (*woz_trks)[track].data[trackWRoffset] = WOZwrite.latch;
                break;
            }
            
            WOZwrite.shift <<= 1;
            printWozBuffer("shift2", i, WOZwrite);
        };
        printWozBuffer("shift2", 9, WOZwrite);

    }
    
}


int woz_loadFile( const char * filename ) {
    
//    char fullpath[256];
//
//    strcpy(fullpath, resourcePath);
//    strcat(fullpath, "/");
//    strcat(fullpath, filename);
    
    FILE * f = fopen(filename, "rb");
    if (f == NULL) {
        perror("Failed to read WOZ: ");
        return WOZ_ERR_FILE_NOT_FOUND;
    }
    
    // get file size
    fseek(f, 0, SEEK_END);
    woz_file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if ( woz_file_buffer ) {
        free(woz_file_buffer);
        woz_file_buffer = NULL;
    }

    woz_file_buffer = malloc(woz_file_size);
    if (woz_file_buffer == NULL) {
        perror("Not Enough Memory: ");
        return WOZ_ERR_BAD_DATA;
    }
    woz_header = (woz_header_t*)woz_file_buffer;
    
    // to simulate file read
    long bufOffs = 0;
    
    fread( woz_file_buffer, woz_file_size, 1, f);
    fclose(f);
    if ( woz_header->magic != WOZ1_MAGIC ) {
        free(woz_file_buffer);
        woz_file_buffer = NULL;
        return WOZ_ERR_NOT_WOZ_FILE;
    }

    bufOffs += sizeof(woz_header_t);

    while ( bufOffs < woz_file_size ) {
        // beginning of the chunk, so we can skip it later
        
        woz_chunk_header = (woz_chunk_header_t*)(woz_file_buffer + bufOffs);
        
        bufOffs += sizeof(woz_chunk_header_t);
        
        switch ( woz_chunk_header->magic ) {
            case WOZ_INFO_CHUNK_ID:
                break;

            case WOZ_TMAP_CHUNK_ID:
                woz_tmap = (woz_tmap_t*)(woz_file_buffer + bufOffs);
                break;

            case WOZ_TRKS_CHUNK_ID:
                woz_trks = (woz_trks_t*)(woz_file_buffer + bufOffs);
                break;

            case WOZ_META_CHUNK_ID:
                break;

            default:
                break;
        }
        
        // make sure we are skipping unhandled chunks correctly
        bufOffs += woz_chunk_header->size;
    }
    
    
    
    // DO NOT COMMIT THIS! ONLY FOR DEBUG!!!
//    woz_loadTrack(0x11);

    return WOZ_ERR_OK;
}

int woz_saveFile( const char * filename ) {
    
    FILE * f = fopen( filename, "wb" );
    if (f == NULL) {
        perror("Failed to crete WOZ: ");
        return WOZ_ERR_FILE_NOT_FOUND;
    }
    
    fwrite( woz_file_buffer, woz_file_size, 1, f );
    fclose(f);
    
    return WOZ_ERR_OK;
}


