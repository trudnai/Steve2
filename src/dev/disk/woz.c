//
//  woz.c
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

#include <stdlib.h>
#include <string.h>

#include "woz.h"
#include "disk.h"
#include "6502.h"
#include "common.h"


WOZread_t WOZread = {0};
WOZread_t WOZwrite = {0};


unsigned trackNextOffset = 0;
unsigned trackOffset = 0;
unsigned bitOffset = 0;
uint64_t clkelpased;

int extraForward = 6; // we search for 7 bit high a bit further to speed up disk read...

char woz_filename[MAXFILENAME];
woz_flags_t woz_flags = {0,0,0,0};
size_t woz_file_size = 0;
uint8_t * woz_file_buffer = NULL;
woz_header_t * woz_header;
woz_chunk_header_t * woz_chunk_header;
woz_tmap_t * woz_tmap;
woz1_trks_t * woz_trks;
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

trackEntry_t prepared_track[WOZ1_TRACK_BYTE_COUNT];

typedef enum readState_e {
    readNormal = 0,
    readHold,
} readState_t;

readState_t readState = readNormal;
uint8_t readLatch;



/*
 CRC generator. Essentially that of Gary S. Brown from 1986, but I've
 fixed the initial value. This is exactly the code advocated by the
 WOZ file specifications (with some extra consts).
 */
static const uint32_t crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/*!
 Computes the CRC32 of an input buffer.
 
 @param buf A pointer to the data to compute a CRC32 from.
 @param size The size of the data to compute a CRC32 from.
 @return The computed CRC32.
 */
static uint32_t crc32(const uint8_t *buf, size_t size) {
    uint32_t crc = ~0;
    size_t byte = 0;
    while (size--) {
        crc = crc32_tab[(crc ^ buf[byte]) & 0xFF] ^ (crc >> 8);
        ++byte;
    }
    return ~crc;
}


void woz_loadTrack_old( int track ) {
    trackEntry_t reg = {0};

    reg.shift = (*woz_trks)[track].data[0];
    reg.data =  (*woz_trks)[track].data[1];
    prepared_track[0] = reg;

    for ( int offs = 1; offs < WOZ1_TRACK_BYTE_COUNT; offs++ ) {
        
        for ( int i = 0; i < 8; i++ ) {
            if (reg.shift & 0x80) {
                reg.shift = 0;
            }
            
            reg.shift16 <<= 1;
        }
        
        reg.data = (*woz_trks)[track].data[ (offs + 1) % WOZ1_TRACK_BYTE_COUNT ];
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
    
    for ( int byteOffs = 0; byteOffs < WOZ1_TRACK_BYTE_COUNT; byteOffs++ ) {

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


// number needs to be unsigned to work with these macros
INLINE unsigned woz_incTrackOffset( unsigned ofs, unsigned limit) {
    return (ofs + 1) % limit;
}

INLINE uint8_t woz_readByte(unsigned trk, unsigned ofs) {
    return (*woz_trks)[trk].data[ofs];
}

uint8_t woz_read() {

    if ( woz_tmap && woz_trks ) {
        int track = woz_tmap->phase[disk.phase.count];
        if (outdev) fprintf(outdev, "track: %d (%d)\n", track, disk.phase.count);
        if ( track >= 40 ) {
            dbgPrintf("TRCK TOO HIGH!\n");
            return rand();
        }
        
        static uint8_t latch = 0;

        uint64_t clktime = m6502.clktime + m6502.clkfrm;
        clkelpased = clktime - m6502.clklast;
        m6502.clklast = clktime;
        
        uint16_t usedBytes = (*woz_trks)[track].bytes_used < WOZ1_TRACK_BYTE_COUNT ? (*woz_trks)[track].bytes_used : WOZ1_TRACK_BYTE_COUNT;
        
        if ( usedBytes ) {
//            static const int extraForward = 4; // we search for 7 bit high a bit further to speed up disk read...
            uint64_t bitForward = (clkelpased >> 2) + extraForward;
            if ( bitForward > 1000 ) {
                bitForward = 4;
            }
            
            // Simulate idle spinning until a close point to the actual turn position
            while ( bitForward-- ) {
                if ( ++bitOffset > 7 ) {
                    bitOffset = 0;
                    trackOffset = woz_incTrackOffset(trackOffset, usedBytes);
                    
                    WOZwrite.data =
                    WOZread.data = woz_readByte(track, trackOffset);
//if (outdev) fprintf(outdev, "(%02X.%u:%u):   d:%02X\n", track, trackOffset, bitOffset, WOZread.data);
                }
                
                WOZread.shift <<= 1;
                WOZwrite.shift <<= 1;

                if ( WOZread.valid ) {
                    latch = WOZread.latch;
//if (outdev) fprintf(outdev, "(%02X.%u:%u):   r:%02X\n", track, trackOffset, bitOffset, WOZread.latch);
                    // latch is cleared when bit 7 is high
//                    WOZwrite.latch =
                    WOZread.latch = 0;
                    // but we do not want to miss that latch valid nibble...
                    // in other words synchronization is needed because of imperfect cycle calculation
                    if ( bitForward < 18 ) { // for 30 Hz FPS 18 is better ) {
                        return latch;
                    }
                }
            }

            // return nibble
            return WOZread.latch;
        }
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
    if ( woz_tmap && woz_trks ) {
        int track = woz_tmap->phase[disk.phase.count];
//if (outdev) fprintf(outdev, "track: %d (%d)\n", track, disk.phase.count);
        if ( track >= 40 ) {
            dbgPrintf("TRACK TOO HIGH!\n");
            return;
        }
        
        woz_flags.disk_modified = 1;
        
        uint64_t clktime = m6502.clktime + m6502.clkfrm;
        clkelpased = clktime - m6502.clklast;
        m6502.clklast = clktime;
        
        uint16_t usedBytes = (*woz_trks)[track].bytes_used < WOZ1_TRACK_BYTE_COUNT ? (*woz_trks)[track].bytes_used : WOZ1_TRACK_BYTE_COUNT;
        
        if ( usedBytes ) {
            uint64_t bitForward = (clkelpased >> 2) + 1;
            
//            uint64_t bitOffsShouldBe = (bitOffset + bitForward) % 8;
//            uint64_t trkOffsShouldBe = (trackOffset + bitForward / 8) % usedBytes;

//if (outdev) fprintf(outdev, "[%02X.%llu:%llu]: *:%02X\n", track, trkOffsShouldBe, bitOffsShouldBe, data);
            
            // for DEBUG ONLY!!!
//if (outdev) fprintf(outdev, "elpased:%llu  data:$%02X\n", clkelpased, data);
            printWozBuffer("*start", 0, WOZwrite);

//printf("(");
            // Simulate idle spinning until s close point to the actual turn position
                while ( bitForward-- ) {
                if ( ++bitOffset > 7 ) {
                        bitOffset = 0;
                    trackOffset = woz_incTrackOffset(trackOffset, usedBytes);

                    WOZwrite.data =
                    WOZread.data = woz_readByte(track, trackOffset);
//if (outdev) fprintf(outdev, "[%02X.%u:%u]:   d:%02X (%016llX)\n", track, trackOffset, bitOffset, WOZwrite.data, WOZwrite.shift);

                    trackNextOffset = woz_incTrackOffset(trackOffset, usedBytes);

                    // load original following data
                    WOZwrite.next = woz_readByte(track, trackNextOffset);
//if (outdev) fprintf(outdev, "[%02X.%u:%u]:   n:%02X (%016llX)\n", track, trackNextOffset, bitOffset, WOZwrite.next,  WOZwrite.shift);
                    
                    }

                    WOZread.shift <<= 1;
                    WOZwrite.shift <<= 1;

                    if ( WOZread.valid ) {
//if (outdev) fprintf(outdev, "[%02X.%u:%u]:   r:%02X (%016llX) {%llu}\n", track, trackOffset, bitOffset, WOZwrite.latch, WOZwrite.shift, bitForward);
//printf("%2X ", WOZread.latch);
                    
                    // we do not clear write latch as we need that
                    // for writing data in byte alignment
                        WOZread.latch = 0;
                    // synchronize
                    if ( bitForward < 8 ) {
                        break;
                    }
                }
            }
            
            // ok now we can latch data
            
            // feed write buffer
            WOZwrite.latch = data;
//if (outdev) fprintf(outdev, "[%02X.%u:%u]:   D:%02X (%016llX)\n", track, trackNextOffset, bitOffset, WOZwrite.latch,  WOZwrite.shift);

            // shift to byte alignment so we can write to the byte stream
            WOZwrite.shift >>= bitOffset + 1;
            
            // write out upper part
//if (outdev) fprintf(outdev, "[%02X.%u:%u]:   R:%02X -> %02X (%016llX)\n", track, trackOffset, 0, (*woz_trks)[track].data[trackOffset], WOZwrite.latch,  WOZwrite.shift);
            (*woz_trks)[track].data[trackOffset] = WOZwrite.latch;

            // write out lower part
//if (outdev) fprintf(outdev, "[%02X.%u:%u]:   R:%02X -> %02X (%016llX)\n", track, trackNextOffset, 0, (*woz_trks)[track].data[trackNextOffset], WOZwrite.data, WOZwrite.shift);
            trackNextOffset = woz_incTrackOffset(trackOffset, usedBytes);
            (*woz_trks)[track].data[trackNextOffset] = WOZwrite.data;

            // shift back to actual bit alignment
            WOZwrite.shift <<= bitOffset + 1;
        }
    }
}


void woz_free_buffer() {
    if ( woz_file_buffer ) {
        free(woz_file_buffer);
        woz_file_buffer = NULL;
        woz_header = NULL;
        woz_tmap = NULL;
        woz_trks = NULL;
    }
}


int woz_parseBuffer() {
    
    woz_flags.disk_modified = 0;
    woz_flags.disk_write_protected = 0;
    woz_flags.image_file_readonly = 0;
    woz_flags.image_loaded = 1;
    
    woz_header = (woz_header_t*)woz_file_buffer;
    
    // to simulate file read
    long bufOffs = 0;
    
    // if this really a WOZ file?
    switch ( woz_header->magic ) {
        case WOZ1_MAGIC:
            break;
            
        case WOZ2_MAGIC:
            break;
            
        default:
            woz_free_buffer();
            return WOZ_ERR_NOT_WOZ_FILE;
    }

//    woz_header->crc
    

    // check if bits and line ends not been altered by data transmission protocols
    if ( woz_header->bit_correctness != 0x0A0D0AFF ) {
        woz_free_buffer();
        return WOZ_ERR_NOT_WOZ_FILE;
    }

    bufOffs += sizeof(woz_header_t);
    
    uint32_t crc = crc32( &woz_file_buffer[bufOffs], woz_file_size - bufOffs);
    if ( crc != woz_header->crc ) {
        printf("WOZ Invalid CRC (%u vs %u)\n", woz_header->crc, crc);
    }
    else {
        dbgPrintf2("WOZ valid CRC (%u vs %u)\n", woz_header->crc, crc);
    }


    while ( bufOffs < woz_file_size ) {
        // beginning of the chunk, so we can skip it later

        woz_chunk_header = (woz_chunk_header_t*) &woz_file_buffer[bufOffs];

        bufOffs += sizeof(woz_chunk_header_t);

        switch ( woz_chunk_header->magic ) {
            case WOZ_INFO_CHUNK_ID:
                break;

            case WOZ_TMAP_CHUNK_ID:
                woz_tmap = (woz_tmap_t*) &woz_file_buffer[bufOffs];
                break;

            case WOZ_TRKS_CHUNK_ID:
                woz_trks = (woz1_trks_t*) &woz_file_buffer[bufOffs];
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
    
    // we eject here so if file not exists, we still have the old disk inserted
    woz_eject();

    // get file size
    fseek(f, 0, SEEK_END);
    woz_file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
        
    woz_file_buffer = malloc(woz_file_size);
    if (woz_file_buffer == NULL) {
        perror("Not Enough Memory: ");
        return WOZ_ERR_BAD_DATA;
    }
    
    size_t len = fread( woz_file_buffer, 1, woz_file_size, f);
    fclose(f);
    
    if ( len != woz_file_size ) {
        perror("Probably bad WOZ file: ");
        return WOZ_ERR_BAD_DATA;
    }
    
    return woz_parseBuffer();
}


void woz_ask_to_save(void);


void woz_eject() {
    if ( woz_flags.disk_modified ) {
        woz_ask_to_save();
    }
    
    woz_flags.disk_modified = 0;
    woz_flags.disk_write_protected = 0;
    woz_flags.image_file_readonly = 0;
    woz_flags.image_loaded = 0;

    woz_file_size = 0;

    woz_free_buffer();
}


int woz_saveFile( const char * filename ) {
    
    if ( filename == NULL ) {
        filename = woz_filename;
    }
    
    woz_header->crc = crc32( woz_file_buffer + sizeof(woz_header_t), woz_file_size - sizeof(woz_header_t));
    
    FILE * f = fopen( filename, "wb" );
    if (f == NULL) {
        perror("Failed to crete WOZ: ");
        return WOZ_ERR_FILE_NOT_FOUND;
    }
    
    fwrite( woz_file_buffer, woz_file_size, 1, f );
    fclose(f);
    
    woz_flags.disk_modified = 0;
    
    return WOZ_ERR_OK;
}


