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
uint8_t   WOZlatch = 0;

int trackOffset = 0;
int bitOffset = 0;
uint64_t clkelpased;


woz_header_t woz_header;
woz_chunk_header_t woz_chunk_header;
woz_tmap_t woz_tmap;
woz_trks_t woz_trks;
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


void woz_loadTrack( int track ) {
    trackEntry_t reg = {0};

    reg.shift = woz_trks[track].data[0];
    reg.data =  woz_trks[track].data[1];
    prepared_track[0] = reg;

    for ( int offs = 1; offs < WOZ_TRACK_BYTE_COUNT; offs++ ) {
        
        for ( int i = 0; i < 8; i++ ) {
            if (reg.shift & 0x80) {
                reg.shift = 0;
            }
            
            reg.shift16 <<= 1;
        }
        
        reg.data = woz_trks[track].data[ (offs + 1) % WOZ_TRACK_BYTE_COUNT ];
        prepared_track[offs] = reg;
    }
}


uint8_t woz_read() {

    int track = woz_tmap.phase[disk.phase.count];
    if (outdev) fprintf(outdev, "track: %d (%d) ", track, disk.phase.count);
    if ( track >= 40 ) {
        dbgPrintf("TRCK TOO HIGH!\n");
        return rand();
    }
    
#ifdef WOZ_REAL_SPIN
    if ( track != track_loaded ) {
        woz_loadTrack(track);
        track_loaded = track;
    }

    WOZread.shift32 = 0;
    bitOffset = (m6502.clktime >> 2) & 7;
    trackOffset = (m6502.clktime >> 5) % WOZ_TRACK_BYTE_COUNT;
    WOZread.next = woz_trks[track].data[ (trackOffset +1) % WOZ_TRACK_BYTE_COUNT ];
    WOZread.data = woz_trks[track].data[ trackOffset ];
    WOZread.prev = woz_trks[track].data[ (trackOffset -1) % WOZ_TRACK_BYTE_COUNT ];
    WOZread.shift = woz_trks[track].data[ (trackOffset -2) % WOZ_TRACK_BYTE_COUNT ];

    printf("clk:%llu  bo:%u  to:%u  W:%08X\n", m6502.clktime, bitOffset, trackOffset, WOZread.shift32);
    
    uint8_t state = 0;
    // simulating the continous shift register in Disk ][
    for (int i = bitOffset + 16; i; --i) {
        
        WOZread.shift32 <<= 1;
        
        switch (state) {
            case 0:
                WOZlatch = WOZread.shift;
                if ( WOZread.valid ) {
                    state = 1;
                    WOZread.shift = 0;
                }
                break;
                
            case 1:
                // we do not latch the shift register for one 4us cycle
                state = 0;
                break;
                
            default:
                // this should not happen
                state = 0;
                break;
        }
        
//                printf("shft1: W:%08X  L:%02X\n", WOZread.shift32, WOZlatch);

    }

    printf("clk:%llu  bo:%u  to:%u  W:%08X  B:%02X\n", m6502.clktime, bitOffset, trackOffset, WOZread.shift32, WOZlatch);
    if ( WOZlatch & 0x80 ) {
        printf("WOZlatch: %02X\n", WOZlatch);
        
        static int readState = 0;
        static int vol = 0;
        static int trk = 0;
        static int sec = 0;
        
        switch (readState) {
            case 0:
                if ( 0xD5 == WOZlatch ) readState = 1;
                else readState = 0;
                break;
                
            case 1:
                if ( 0xAA == WOZlatch ) readState = 2;
                else readState = 0;
                break;
                
            case 2:
                if ( 0x96 == WOZlatch ) {
                    readState = 3;
                    printf("sector header marker\n");
                }
                else if ( 0xAD == WOZlatch ) {
                    readState = 10;
                    printf("sector data marker\n");
                }
                else readState = 0;
                break;
                
            case 3: // sector header, vol number 1
                readState++;
                vol = (WOZlatch << 1) | 1;
                break;
                
            case 4: // sector header, vol number 2
                readState++;
                vol &= WOZlatch;
                printf("vol:%u ", vol);
                break;
                
            case 5: // sector header, trk number 1
                readState++;
                trk = (WOZlatch << 1) | 1;
                break;
                
            case 6: // sector header, trk number 2
                readState++;
                trk &= WOZlatch;
                printf("trk:%u ", trk);
                break;
                
            case 7: // sector header, sec number 1
                readState++;
                sec = (WOZlatch << 1) | 1;
                break;
                
            case 8: // sector header, sec number 2
                readState = 0;
                sec &= WOZlatch;
                printf("sec:%u\n", sec);
                break;
                    

            default:
                readState = 0;
                break;
        }
    }
    
//            switch ( WOZlatch ) {
//                case 0xAA:
//                case 0x96:
////                    printf("%02X ", WOZlatch);
//                    printf("clk:%llu  bo:%u  to:%u  B:%02X\n", m6502.clktime, bitOffset, trackOffset, WOZlatch);
//                    break;
//
//                default:
//                    break;
//            }

    return WOZlatch;

    
#elif defined( WOZ_REAL_SPIN2 )
//    clkelpased = m6502.clktime - disk.clk_last_read;
//    disk.clk_last_read = m6502.clktime;

    bitOffset = (m6502.clktime >> 2) & 7;
    trackOffset = (m6502.clktime >> 5) % WOZ_TRACK_BYTE_COUNT;
    trackEntry_t reg = prepared_track[trackOffset];
    
    do {
        switch (readState) {
            case readNormal:
                readLatch = reg.shift;
                break;
                
            case readHold:
            default:
                readState = readNormal;
                break;
        }

        if (reg.shift & 0x80) {
            reg.shift = 0;
            readState = readHold;
        }
        
        reg.shift16 <<= 1;
    } while ( --bitOffset > 0 );

    printf("READ: clk:%llu  to:%u  bo:%llu  B:%02X\n", m6502.clktime, trackOffset, (m6502.clktime >> 2) & 7, readLatch);
    return readLatch;

    
#else // WOZ_REAL_SPIN
    clkelpased = m6502.clktime - m6502.clklast;
    m6502.clklast = m6502.clktime;
    
    uint16_t usedBytes = woz_trks[track].bytes_used < WOZ_TRACK_BYTE_COUNT ? woz_trks[track].bytes_used : WOZ_TRACK_BYTE_COUNT;
    if ( usedBytes ) {
        if ( clkelpased > 100 ) {
    //        printf("NEED SYNC : %llu\n", clkelpased);
            bitOffset = (clkelpased >> 2) & 7;
            trackOffset += ((clkelpased >> 5) +100) % usedBytes;
            WOZread.data = woz_trks[track].data[trackOffset];
        }

        // to avoid infinite loop and to search for bit 7 high
        for ( int i = 0; i < usedBytes * 8; i++ ) {
            if ( ++bitOffset >= 8 ) {
                bitOffset = 0;
    //                    if ( ++trackOffset >= WOZ_TRACK_BYTE_COUNT ) {
    //                        trackOffset = 0;
    //                    }
                trackOffset++;
                trackOffset %= usedBytes;

    //                    printf("offs:%u\n", trackOffset);
                WOZread.data = woz_trks[track].data[trackOffset];
            }
            
            WOZread.shift16 <<= 1;
            if ( WOZread.valid ) {
                uint8_t byte = WOZread.shift;
    //                    printf("%02X ", byte);
                WOZread.shift = 0;
                if (outdev) fprintf(outdev, "byte: %02X\n", byte);
                return byte;
            }
        }
        if (outdev) fprintf(outdev, "TIME OUT!\n");
    }
    
    return rand();

#endif // WOZ_REAL_SPIN

}


void woz_loadFile( const char * resourcePath, const char * filename ) {
    
    char fullpath[256];
    
    strcpy(fullpath, resourcePath);
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    
    FILE * f = fopen(fullpath, "rb");
    if (f == NULL) {
        perror("Failed to read WOZ: ");
        return;
    }
    
    fread( &woz_header, 1, sizeof(woz_header_t), f);
    if ( woz_header.magic != WOZ1_MAGIC ) {
        return;
    }
    
    while ( ! feof(f) ) {
        // beginning of the chunk, so we can skip it later
        
        long r = fread( &woz_chunk_header, 1, sizeof(woz_chunk_header_t), f);
        if ( r != sizeof(woz_chunk_header_t) ) {
            break;
        }
        long foffs = ftell(f);
        
        void * buf = NULL;

        switch ( woz_chunk_header.magic ) {
            case WOZ_INFO_CHUNK_ID:
                break;

            case WOZ_TMAP_CHUNK_ID:
                buf = &woz_tmap;
                break;

            case WOZ_TRKS_CHUNK_ID:
                buf = woz_trks;
                break;

            case WOZ_META_CHUNK_ID:
                break;

            default:
                break;
        }
        
        if (buf) {
            r = fread( buf, 1, woz_chunk_header.size, f);
            if ( r != woz_chunk_header.size ) {
                break;
            }
        }

        // make sure we are skipping unhandled chunks correctly
        fseek(f, foffs + woz_chunk_header.size, SEEK_SET);
    }
    
    fclose(f);

}


