//
//  speaker.c
//  A2Mac
//
//  Created by Tamas Rudnai on 5/9/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "speaker.h"
#include "6502.h"
#include "disk.h" // to be able to disable disk acceleration


#define min(x,y) (x) < (y) ? (x) : (y)
#define max(x,y) (x) > (y) ? (x) : (y)
#define clamp(min,num,max) (num) < (min) ? (min) : (num) > (max) ? (max) : (num)


#define CASE_RETURN(err) case (err): return #err
const char* al_err_str(ALenum err) {
    switch(err) {
            CASE_RETURN(AL_NO_ERROR);
            CASE_RETURN(AL_INVALID_NAME);
            CASE_RETURN(AL_INVALID_ENUM);
            CASE_RETURN(AL_INVALID_VALUE);
            CASE_RETURN(AL_INVALID_OPERATION);
            CASE_RETURN(AL_OUT_OF_MEMORY);
    }
    printf("alError: 0x%04X\n", err);
    return "unknown";
}
#undef CASE_RETURN

#define __al_check_error(file,line) \
    do { \
        ALenum err = alGetError(); \
        for(; err != AL_NO_ERROR; err = alGetError()) { \
            printf( "AL Error %s at %s:%d\n", al_err_str(err), file, line ); \
        } \
    } while(0)

#define al_check_error() \
    __al_check_error(__FILE__, __LINE__)


ALCdevice *dev = NULL;
ALCcontext *ctx = NULL;

ALuint spkr_src [4] = { 0, 0, 0, 0 };

int spkr_level = SPKR_LEVEL_ZERO;


#define BUFFER_COUNT 256
#define SOURCES_COUNT 1

ALuint spkr_buffers[BUFFER_COUNT];
ALuint spkr_disk_motor_buf = 0;
ALuint spkr_disk_arm_buf = 0;
ALuint spkr_disk_ioerr_buf = 0;


const int spkr_fps = fps;
const int spkr_seconds = 1;
const unsigned spkr_sample_rate = 44100;
const unsigned sfx_sample_rate =  22050; // original sample rate
//const unsigned sfx_sample_rate =  26000; // bit higher pitch
unsigned spkr_extra_buf = 800 / spkr_fps;
const unsigned spkr_buf_size = spkr_seconds * spkr_sample_rate * 2 / spkr_fps;
int16_t spkr_samples [ spkr_buf_size * spkr_fps * BUFFER_COUNT * 2]; // stereo
unsigned spkr_sample_idx = 0;

const unsigned spkr_play_timeout = 8; // increase to 32 for 240 fps
unsigned spkr_play_time = 0;
unsigned spkr_play_disk_motor_time = 0;
unsigned spkr_play_disk_arm_time = 0;
unsigned spkr_play_disk_ioerr_time = 0;

uint8_t * diskmotor_sfx = NULL;
int       diskmotor_sfx_len = 0;
uint8_t * diskarm_sfx = NULL;
int       diskarm_sfx_len = 0;
uint8_t * diskioerr_sfx = NULL;
int       diskioerr_sfx_len = 0;


static int load_sfx( const char * bundlePath, const char * filename, uint8_t ** buf ) {
    char fullPath[256];
    
    strcpy( fullPath, bundlePath );
    strcat( fullPath, "/");
    strcat( fullPath, filename );
    
    FILE * f = fopen(fullPath, "rb");
    if (f == NULL) {
        perror("Failed to read SFX: ");
        return -1;
    }
    
    fseek(f, 0L, SEEK_END);
    uint16_t flen = ftell(f);
    fseek(f, 0L, SEEK_SET);

    if (flen <= 0) {
        printf("Failed to read SFX or 0 size\n");
        return -1;
    }

    *buf = malloc(flen);
    
    if ( *buf == NULL ) {
        printf("Not enough memory for SFX\n");
        return -1;
    }
    
    fread( *buf, 1, flen, f);
    fclose(f);

    if ( flen == 0 ) {
        printf("Error loading SFX file\n");
        free( *buf );
        return -1; // there was an error
    }

    // everything seems to be ok
    return flen;
}


void spkr_load_sfx( const char * bundlePath ) {
    diskmotor_sfx_len = load_sfx(bundlePath, "diskmotor.raw", &diskmotor_sfx);
    diskarm_sfx_len = load_sfx(bundlePath, "diskarm.raw", &diskarm_sfx);
    diskioerr_sfx_len = load_sfx(bundlePath, "diskioerr.raw", &diskioerr_sfx);
}


// initialize OpenAL
void spkr_init() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    printf( "Default device: %s\n", defname );

    // restart OpenAL when restarting the virtual machine
    spkr_exit();
    
    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    
    // Fill buffer with zeros
    memset( spkr_samples, spkr_level, spkr_buf_size * sizeof(spkr_samples[0]) );
    
    // Create buffer to store samples
    alGenBuffers(BUFFER_COUNT, spkr_buffers);
    alGenBuffers(1, &spkr_disk_motor_buf);
    alGenBuffers(1, &spkr_disk_arm_buf);
    al_check_error();
    
    // Set-up sound source and play buffer
    alGenSources(4, spkr_src);
    al_check_error();
    alSourcei(spkr_src[0], AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src[0], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[0], AL_POSITION, 0.0, 8.0, 0.0);
    al_check_error();
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
//    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
//    al_check_error();

    
    // Set-up disk motor sound source and play buffer
    alSourcei(spkr_src[1], AL_LOOPING, AL_TRUE);
    al_check_error();
    alSourcef(spkr_src[1], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[1], AL_POSITION, 0.0, 8.0, 0.0);
    al_check_error();
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
//    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
//    al_check_error();

    
    // Set-up disk arm sound source and play buffer
    alSourcei(spkr_src[2], AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src[2], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[2], AL_POSITION, 0.0, 8.0, 0.0);
    al_check_error();
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
//    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
//    al_check_error();
    
    
    // Set-up disk io error sound source and play buffer
    alSourcei(spkr_src[3], AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src[3], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[3], AL_POSITION, 0.0, 8.0, 0.0);
    al_check_error();
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
//    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
//    al_check_error();
    
    
    // start from the beginning
    spkr_sample_idx = 0;

    // make sure we have free buffers initialized here
    freeBuffers = BUFFER_COUNT;
}

// Dealloc OpenAL
void spkr_exit() {
    if ( spkr_src[0] ) {
        alSourceStop( spkr_src[0] );
        
        ALCdevice *dev = NULL;
        ALCcontext *ctx = NULL;
        ctx = alcGetCurrentContext();
        dev = alcGetContextsDevice(ctx);
        
        alcMakeContextCurrent(NULL);
        alcDestroyContext(ctx);
        alcCloseDevice(dev);
        
        al_check_error();
        
        memset(spkr_src, 0, sizeof(spkr_src));
        memset(spkr_buffers, 0, sizeof(spkr_buffers));
        spkr_disk_motor_buf = 0;
        spkr_disk_arm_buf = 0;
    }
}


char spkr_state = 0;

void spkr_toggle() {
    // TODO: This is very slow!
    
    spkr_play_time = 0;
    
    if ( diskAccelerator_count ) {
        // turn off disk acceleration immediately
        diskAccelerator_count = 0;
        clk_6502_per_frm = clk_6502_per_frm_max = clk_6502_per_frm_set;
    }
    
    if ( clk_6502_per_frm_set <  clk_6502_per_frm_max_sound ) {
        
        spkr_play_time = spkr_play_timeout;
        
        // push a click into the speaker buffer
        // (we will play the entire buffer at the end of the frame)
        spkr_sample_idx = (clkfrm / (default_MHz_6502 / spkr_sample_rate)) * 2;
        
        if ( spkr_state ) {
            // down edge
            spkr_state = 0;
            
            float fadeLevel = spkr_level - SPKR_LEVEL_MIN;
            
            while ( fadeLevel > +1 ) {
                spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_MIN + fadeLevel;
                spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_MIN + fadeLevel;
                
                // how smooth we want the speeker to decay, so we will hear no pops and crackles
                // 0.9 gives you a kind of saw wave at 1KHz (beep)
                // 0.7 is better, but Xonix gives you a bit distorted speech and Donkey Kong does not sound the same
                fadeLevel *= 0.16;
            }
            spkr_level = SPKR_LEVEL_MIN;
        }
        else {
            // up edge
            spkr_state = 1;
            
            float fadeLevel = spkr_level - SPKR_LEVEL_MAX;
            
            while ( fadeLevel < -1 ) {
                spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_MAX + fadeLevel;
                spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_MAX + fadeLevel;
                
                // how smooth we want the speeker to decay, so we will hear no pops and crackles
                // 0.9 gives you a kind of saw wave at 1KHz (beep)
                // 0.7 is better, but Xonix gives you a bit distorted speech and Donkey Kong does not sound the same
                fadeLevel *= 0.32;
            }
            spkr_level = SPKR_LEVEL_MAX;
        }

        
        //spkr_samples[sample_idx] = spkr_level;
        for ( int i = spkr_sample_idx; i < spkr_buf_size + spkr_extra_buf; i++ ) {
            spkr_samples[i] = spkr_level;
        }
    //    memset(spkr_samples + spkr_sample_idx, spkr_level, spkr_buf_size * sizeof(spkr_samples[0]));
        
    }
}



ALint freeBuffers = BUFFER_COUNT;
//ALuint alBuffers[BUFFER_COUNT];

int spkr_unqueue( ALuint src ) {
    ALint processed = 0;
    
    alGetSourcei ( src, AL_BUFFERS_PROCESSED, &processed );
    al_check_error();
    
    if ( processed ) {
        alSourceUnqueueBuffers( src, processed, &spkr_buffers[freeBuffers]);
        al_check_error();
    }
    
    return processed;
}

void spkr_update() {
    if ( spkr_play_time ) {
        // free up unused buffers
        freeBuffers += spkr_unqueue( spkr_src[0] );
        freeBuffers = clamp( 1, freeBuffers, BUFFER_COUNT );

        if ( freeBuffers ) {
        
            ALenum state;
            alGetSourcei( spkr_src[0], AL_SOURCE_STATE, &state );
    //        al_check_error();

            if ( --spkr_play_time == 0 ) {
                float fadeLevel = spkr_level - SPKR_LEVEL_ZERO;
                
                if ( spkr_level != SPKR_LEVEL_ZERO ) {
                    spkr_sample_idx = 0;

                    while ( ( fadeLevel < -1 ) || ( fadeLevel > 1 ) ) {
                        spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_ZERO + fadeLevel;
                        spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_ZERO + fadeLevel;
                        
                        // how smooth we want the speeker to decay, so we will hear no pops and crackles
                        fadeLevel *= 0.999;
                    }
                    spkr_level = SPKR_LEVEL_ZERO;

                    //spkr_samples[sample_idx] = spkr_level;
                    memset(spkr_samples + spkr_sample_idx, SPKR_LEVEL_ZERO, spkr_extra_buf * sizeof(spkr_samples[0]));

                    freeBuffers--;
                    alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, spkr_sample_idx * sizeof(spkr_samples[0]), spkr_sample_rate);
                    al_check_error();
                    alSourceQueueBuffers(spkr_src[0], 1, &spkr_buffers[freeBuffers]);
                    al_check_error();
                }
            }
            else {
                freeBuffers--;
                alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, (spkr_buf_size + spkr_extra_buf) * sizeof(spkr_samples[0]), spkr_sample_rate);
                al_check_error();
                alSourceQueueBuffers(spkr_src[0], 1, &spkr_buffers[freeBuffers]);
                al_check_error();
            }
            
            switch (state) {
                case AL_PAUSED:
                    alSourcePlay(spkr_src[0]);
                    break;

                case AL_PLAYING:
                    // already playing
                    break;
                    
                default:
                    alSourcePlay(spkr_src[0]);
                    alSourcePause(spkr_src[0]);
                    break;
            }
            
            // clear the slack buffer , so we can fill it up by new data
            for ( int i = 0; i < spkr_buf_size + spkr_extra_buf; i++ ) {
                spkr_samples[i] = spkr_level;
            }
            
        }
        
        // start from the beginning
        spkr_sample_idx = 0;

    }
    
}


void spkr_playqueue_sfx( ALuint src, uint8_t * sfx, int len ) {
    
//    freeBuffers += spkr_unqueue( src );
//    freeBuffers = clamp( 1, freeBuffers, BUFFER_COUNT );

    if ( freeBuffers ) {
        ALenum queued;
        alGetSourcei( src, AL_BUFFERS_QUEUED, &queued );
    //    printf("Q:%u\n", queued);

        if ( queued < 32 ) {
            freeBuffers--;
            alBufferData( spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, sfx, len, sfx_sample_rate );
            al_check_error();
            alSourceQueueBuffers( src, 1, &spkr_buffers[freeBuffers] );
            al_check_error();
            
            ALenum state;
            alGetSourcei( src, AL_SOURCE_STATE, &state );
        //        al_check_error();
            
            switch (state) {
                case AL_PLAYING:
                    // already playing
                    break;
                    
                default:
                    alSourcePlay( src );
                    break;
            }
        }
    }
}


void spkr_play_sfx( ALuint src, uint8_t * sfx, int len ) {
    if ( freeBuffers ) {
        ALenum state;
        alGetSourcei( src, AL_SOURCE_STATE, &state );
    //        al_check_error();
        
        switch (state) {
            case AL_PAUSED:
                alSourcePlay( src );
                break;
                
            case AL_PLAYING:
                // already playing
                break;
                
            default:
                freeBuffers--;
                alBufferData( spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, sfx, len, sfx_sample_rate );
                al_check_error();
                alSourceQueueBuffers( src, 1, &spkr_buffers[freeBuffers] );
                al_check_error();
                
                alSourcePlay( src );
                break;
        }
    }
}


void spkr_stop_sfx( ALuint src ) {
    ALenum state;
    alGetSourcei( src, AL_SOURCE_STATE, &state );
//        al_check_error();
    
    switch (state) {
        case AL_PAUSED:
        case AL_PLAYING:
            alSourceStop( src );
            freeBuffers += spkr_unqueue( src );
            freeBuffers = clamp( 1, freeBuffers, BUFFER_COUNT );
            break;
            
        default:
            break;
    }
}


void spkr_play_disk_motor() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= iicplus_MHz_6502 / fps ) ) {
        spkr_play_sfx( spkr_src[1], diskmotor_sfx, diskmotor_sfx_len );
    }
}

void spkr_stop_disk_motor( int time ) {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= iicplus_MHz_6502 / fps ) ) {
        spkr_play_disk_motor_time = time;
    }
}


void spkr_play_disk_arm() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= iicplus_MHz_6502 / fps ) ) {
        if ( spkr_play_disk_ioerr_time == 0 ) {
            spkr_play_sfx( spkr_src[2], diskarm_sfx, diskarm_sfx_len );
            spkr_play_disk_arm_time = 2;
        }
    }
}


void spkr_play_disk_ioerr() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= iicplus_MHz_6502 / fps ) ) {
        spkr_playqueue_sfx( spkr_src[3], diskioerr_sfx, diskioerr_sfx_len);
        spkr_play_disk_ioerr_time = 4;
    }
}


void update_disk_sfx( unsigned * time, ALuint src ) {
    if ( *time ) {
        if ( --*time == 0 ) {
            spkr_stop_sfx( src );
        }
    }
}

void spkr_update_disk_sfx() {
    // is user speeds up the machine, disk sfx needs to be stopped
    if ( ( ! disk_sfx_enabled ) || ( clk_6502_per_frm > iicplus_MHz_6502 / fps ) ) {
        if ( spkr_play_disk_motor_time ) {
            spkr_play_disk_motor_time = 1; // rest will be taken care below
        }
        if ( spkr_play_disk_arm_time ) {
            spkr_play_disk_arm_time = 1; // rest will be taken care below
        }
    }
    
    update_disk_sfx( &spkr_play_disk_motor_time, spkr_src[1] );
    update_disk_sfx( &spkr_play_disk_arm_time, spkr_src[2] );

    // we do not need to stop playing,
    // however, counter needed to eliminate arm movement noise while in io error
    if ( spkr_play_disk_ioerr_time ) {
        spkr_play_disk_ioerr_time--;
    }
    
}

