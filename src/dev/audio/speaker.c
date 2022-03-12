//
//  speaker.c
//  Steve ][
//
//  Created by Tamas Rudnai on 5/9/20.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "limits.h"

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
    return "AL_UNKNOWN_ERROR";
}
#undef CASE_RETURN

#define __al_check_error(file,line) \
    for( ALenum err = alGetError(); err != AL_NO_ERROR; err = alGetError() ) { \
            printf( "AL Error %s at %s:%d\n", al_err_str(err), file, line ); \
    }

#define al_check_error() \
    __al_check_error(__FILE__, __LINE__)


ALCdevice *dev = NULL;
ALCcontext *ctx = NULL;


#define SPKR_MIN_VOL    0.0001 // OpenAL cannot change volume to 0.0 for some reason, so we just turn volume really low
int spkr_att = 0;
int spkr_level = SPKR_LEVEL_ZERO;
int spkr_level_ema = SPKR_LEVEL_ZERO;
int spkr_last_level = SPKR_LEVEL_ZERO;

#define BUFFER_COUNT 256
#define SPKR_CHANNELS 2

ALuint spkr_src [SOURCES_COUNT] = { 0, 0, 0, 0 };

ALuint spkr_buffers[BUFFER_COUNT] = { 0 };
ALuint spkr_disk_motor_buf = 0;
ALuint spkr_disk_arm_buf = 0;
ALuint spkr_disk_ioerr_buf = 0;


float spkr_vol = 0.5;


const unsigned spkr_fps = DEFAULT_FPS;
unsigned spkr_fps_divider = 1;
unsigned spkr_frame_cntr = 0;
unsigned spkr_clk = 0;

const unsigned spkr_seconds = 1;
const unsigned spkr_sample_rate = 192000;
const unsigned sfx_sample_rate =  22050; // original sample rate
//const unsigned sfx_sample_rate =  26000; // bit higher pitch
int spkr_extra_buf = 0; // 26; // 800 / spkr_fps;
typedef int16_t spkr_sample_t;
const unsigned spkr_buf_size = spkr_seconds * spkr_sample_rate * SPKR_CHANNELS / DEFAULT_FPS; // stereo
const unsigned spkr_buf_alloc_size = spkr_buf_size * sizeof(spkr_sample_t);
const unsigned sample_buf_array_len = spkr_buf_size * DEFAULT_FPS * BUFFER_COUNT;
spkr_sample_t spkr_samples [ sample_buf_array_len ]; // can store up to 1 sec of sound
unsigned spkr_sample_idx = 0;
unsigned spkr_sample_last_idx = 0;
unsigned spkr_sample_first_pwm_idx = 0;

unsigned spkr_play_timeout = SPKR_PLAY_TIMEOUT; // increase to 32 for 240 fps, normally 8 for 30 fps
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

ALint freeBuffers = BUFFER_COUNT;


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
    diskmotor_sfx_len = load_sfx(bundlePath, "disk_ii_motor_w_floppy.sfx", &diskmotor_sfx);
    diskarm_sfx_len = load_sfx(bundlePath, "disk_ii_arm.sfx", &diskarm_sfx);
    diskioerr_sfx_len = load_sfx(bundlePath, "disk_ii_io_error.sfx", &diskioerr_sfx);
}

#define NO_SPKR_KEEP_PITCH

#define NO_SPKR_DEBUG
#ifdef SPKR_DEBUG
static const char * spkr_debug_filename = "steve2_audio_debug.bin";
FILE * spkr_debug_file = NULL;
#endif


void spkr_fade(float fadeLevel, unsigned idx) {
    // Fade
    while ( ( fadeLevel < -1 ) || ( fadeLevel > 1 ) ) {
        spkr_samples[ idx++ ] = SPKR_LEVEL_ZERO + fadeLevel;
        spkr_samples[ idx++ ] = SPKR_LEVEL_ZERO + fadeLevel; // stereo
        
        // how smooth we want the speeker to decay, so we will hear no pops and crackles
        fadeLevel *= 0.999;
    }
    
    // Fill with Zero to avoid pops
    memset(spkr_samples + idx * sizeof(spkr_sample_t), 0, spkr_buf_size * sizeof(spkr_sample_t) * SPKR_CHANNELS);
}


// initialize OpenAL
void spkr_init() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    printf( "Default device: %s\n", defname );
    
    // DEBUG ONLY!!!

#ifdef SPKR_DEBUG
        spkr_debug_file = fopen(spkr_debug_filename, "w+");
#endif
    
    // restart OpenAL when restarting the virtual machine
    spkr_exit();
    
    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    
    // Fill buffer with zeros
    memset( spkr_samples, 0, spkr_buf_alloc_size + spkr_extra_buf * sizeof(spkr_sample_t));
    
//    memset(spkr_samples + spkr_sample_idx * sizeof(spkr_sample_t), 0, (sample_buf_array_len - spkr_sample_idx) * sizeof(spkr_sample_t));
    
    freeBuffers--;
    
    // DEBUG ONLY!!!
#ifdef SPKR_DEBUG
        fwrite(spkr_samples, sizeof(spkr_sample_t), spkr_sample_idx, spkr_debug_file);
        fflush(spkr_debug_file);
#endif

    alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, spkr_sample_idx * sizeof(spkr_sample_t), spkr_sample_rate);
    al_check_error();
    alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
    al_check_error();

    
    // Create buffer to store samples
    alGenBuffers(BUFFER_COUNT, spkr_buffers);
    alGenBuffers(1, &spkr_disk_motor_buf);
    alGenBuffers(1, &spkr_disk_arm_buf);
    al_check_error();
    
    // Set-up sound source and play buffer
    alGenSources(SOURCES_COUNT, spkr_src);
    al_check_error();
    
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
    alListenerf(AL_GAIN, spkr_vol);
    al_check_error();
//    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
//    al_check_error();

    alSourcei(spkr_src[SPKR_SRC_GAME_SFX], AL_SOURCE_RELATIVE, AL_TRUE);
    al_check_error();
    alSourcei(spkr_src[SPKR_SRC_GAME_SFX], AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src[SPKR_SRC_GAME_SFX], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[SPKR_SRC_GAME_SFX], AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
    
    
    // Set-up disk motor sound source and play buffer
    alSourcei(spkr_src[SPKR_SRC_DISK_MOTOR_SFX], AL_SOURCE_RELATIVE, AL_TRUE);
    al_check_error();
    alSourcei(spkr_src[SPKR_SRC_DISK_MOTOR_SFX], AL_LOOPING, AL_TRUE);
    al_check_error();
    alSourcef(spkr_src[SPKR_SRC_DISK_MOTOR_SFX], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[SPKR_SRC_DISK_MOTOR_SFX], AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();

    
    // Set-up disk arm sound source and play buffer
    alSourcei(spkr_src[SPKR_SRC_DISK_ARM_SFX], AL_SOURCE_RELATIVE, AL_TRUE);
    al_check_error();
    alSourcei(spkr_src[SPKR_SRC_DISK_ARM_SFX], AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src[SPKR_SRC_DISK_ARM_SFX], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[SPKR_SRC_DISK_ARM_SFX], AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
    
    
    // Set-up disk io error sound source and play buffer
    alSourcei(spkr_src[SPKR_SRC_DISK_IOERR_SFX], AL_SOURCE_RELATIVE, AL_TRUE);
    al_check_error();
    alSourcei(spkr_src[SPKR_SRC_DISK_IOERR_SFX], AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src[SPKR_SRC_DISK_IOERR_SFX], AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src[SPKR_SRC_DISK_IOERR_SFX], AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
    
    
    // start from the beginning
    spkr_sample_idx = 0;

    // make sure we have free buffers initialized here
    freeBuffers = BUFFER_COUNT;
}


void spkr_vol_up() {
    spkr_vol += 0.1;
    if ( spkr_vol > 1 ) {
        spkr_vol = 1;
    }
    alListenerf(AL_GAIN, spkr_vol);
    al_check_error();
}

void spkr_vol_dn() {
    spkr_vol -= 0.1;
    if ( spkr_vol < 0.1 ) {
        // use mute to make it completely silent
        spkr_vol = 0.1;
    }
    alListenerf(AL_GAIN, spkr_vol);
    al_check_error();
}

void spkr_mute() {
    ALfloat vol = 0;
    alGetListenerf(AL_GAIN, &vol);
    al_check_error();

    if ( vol > SPKR_MIN_VOL ) {
        alListenerf(AL_GAIN, SPKR_MIN_VOL);
        al_check_error();
    }
    else {
        alListenerf(AL_GAIN, spkr_vol);
        al_check_error();
    }
}


int spkr_unqueue( ALuint src ) {
    ALint processed = 0;
    
    if ( src ) {
        alGetSourcei ( src, AL_BUFFERS_PROCESSED, &processed );
        al_check_error();
    //    printf("%s alGetSourcei(%d)\n", __FUNCTION__, src);
//        printf("p:%d\n", processed);

        if ( processed > 0 ) {
//            printf("pf:%d\n", processed);
            alSourceUnqueueBuffers( src, processed, &spkr_buffers[freeBuffers]);
            al_check_error();
            freeBuffers = clamp( 1, freeBuffers + processed, BUFFER_COUNT );
            
//            alGetSourcei ( src, AL_BUFFERS_PROCESSED, &processed );
//            al_check_error();
//            printf("pf2:%d\n", processed);
        }
    }
    
    return processed;
}

void spkr_unqueueAll() {
    for ( int i = 0; i < SOURCES_COUNT; i++ ) {
        spkr_unqueue( spkr_src[i] );
    }
}


// Dealloc OpenAL
void spkr_exit() {
    if ( spkr_src[SPKR_SRC_GAME_SFX] ) {
        spkr_stopAll();
        spkr_unqueueAll();
        
        // delete buffers
        alDeleteBuffers(BUFFER_COUNT, spkr_buffers);
        al_check_error();
        alDeleteBuffers(1, &spkr_disk_motor_buf);
        al_check_error();
        alDeleteBuffers(1, &spkr_disk_arm_buf);
        al_check_error();

        // delete sound source and play buffer
        alDeleteSources(SOURCES_COUNT, spkr_src);
        al_check_error();

        ALCdevice *dev = alcGetContextsDevice(ctx);
        ALCcontext *ctx = alcGetCurrentContext();
        
        alcMakeContextCurrent(NULL);
        al_check_error();
        alcDestroyContext(ctx);
        al_check_error();
        alcCloseDevice(dev);
        al_check_error();
        
        memset(spkr_src, 0, sizeof(spkr_src));
        memset(spkr_buffers, 0, sizeof(spkr_buffers));
        spkr_disk_motor_buf = 0;
        spkr_disk_arm_buf = 0;
    }
}


char spkr_state = 0;


// Exponential Moving Average
INLINE int ema( int val, int prev, float ema_len ) {
    static const float ema_sensitivity = 2;
//    static const float ema_len = 42;
    float m = ema_sensitivity / ema_len;
    float n = 1 - m;
    
    return m * val + n * prev;
}


void spkr_toggle_square ( int level_max, const unsigned idx_diff ) {
    // finish the aquare wave
    while ( spkr_sample_last_idx < spkr_sample_idx ) {
        spkr_samples[ spkr_sample_last_idx++ ] = spkr_level;
        spkr_samples[ spkr_sample_last_idx++ ] = spkr_level; // stereo
    }
    
    // start a new speaker level -- it might not be necessary, but...
    spkr_samples[ spkr_sample_idx++ ] = level_max;
    spkr_samples[ spkr_sample_idx++ ] = level_max; // stereo
    
    spkr_level = level_max;
}


void spkr_toggle_tick ( int level_max, const unsigned idx_diff ) {
    spkr_level = SPKR_LEVEL_ZERO;
    spkr_samples[ spkr_sample_idx++ ] = level_max;
    spkr_samples[ spkr_sample_idx++ ] = level_max; // stereo
    spkr_sample_last_idx = spkr_sample_idx;
    spkr_last_level = spkr_level;
}


//float SPKR_FADE_LEADING_EDGE      = 0.92;
//float SPKR_FADE_TRAILING_EDGE     = 0.92;
//float SPKR_INITIAL_LEADING_EDGE   = 0.64; // leading edge should be pretty steep to get sharp sound plus to avoid Wavy Navy high pitch sound
//float SPKR_INITIAL_TRAILING_EDGE  = 0.64; // need a bit of slope to get Xonix sound good

//float SPKR_FADE_LEADING_EDGE      = 0.82;
//float SPKR_FADE_TRAILING_EDGE     = 0.82;
//float SPKR_INITIAL_LEADING_EDGE   = 0.82; // leading edge should be pretty steep to get sharp sound plus to avoid Wavy Navy high pitch sound
//float SPKR_INITIAL_TRAILING_EDGE  = 0.64; // need a bit of slope to get Xonix sound good

float SPKR_FADE_LEADING_EDGE      = 0.64;
float SPKR_FADE_TRAILING_EDGE     = 0.64;
float SPKR_INITIAL_LEADING_EDGE   = 0.64; // leading edge should be pretty steep to get sharp sound plus to avoid Wavy Navy high pitch sound
float SPKR_INITIAL_TRAILING_EDGE  = 0.64; // need a bit of slope to get Xonix sound good


void spkr_toggle() {
    if ( diskAccelerator_count ) {
        // turn off disk acceleration immediately
        diskAccelerator_count = 0;
        clk_6502_per_frm = clk_6502_per_frm_max = clk_6502_per_frm_set;
    }
    
    if ( clk_6502_per_frm_set <  clk_6502_per_frm_max_sound ) {
        
        spkr_play_time = spkr_play_timeout;
        
        // push a click into the speaker buffer
        // (we will play the entire buffer at the end of the frame)
        double multiplier = (double)spkr_sample_rate / MHZ(MHz_6502);
#ifdef SPKR_KEEP_PITCH
        if ( MHz_6502 < default_MHz_6502 ) {
            multiplier = (double)spkr_sample_rate / MHZ(default_MHz_6502);
//            indexer = (double)spkr_sample_rate / MHZ(default_MHz_6502) * MHz_6502;
        }
#endif
        
        spkr_sample_idx = round( (double)(spkr_clk + m6502.clkfrm) * multiplier ) * SPKR_CHANNELS;
        spkr_sample_idx &= UINTMAX_MAX - 1;
        
        unsigned spkr_sample_idx_diff = spkr_sample_idx - spkr_sample_last_idx;

        if ( (int)spkr_sample_idx_diff < 0 ) {
            spkr_sample_idx_diff = UINT_MAX - spkr_sample_idx_diff;
        }

        if ( spkr_state ) {
            // down edge
            spkr_state = 0;
            
//            spkr_toggle_tick(SPKR_LEVEL_MIN, spkr_sample_idx_diff);
            spkr_toggle_square(SPKR_LEVEL_MIN, spkr_sample_idx_diff);
        }
        else {
            // up edge
            spkr_state = 1;
            
//            spkr_toggle_tick(SPKR_LEVEL_MAX, spkr_sample_idx_diff);
            spkr_toggle_square(SPKR_LEVEL_MAX, spkr_sample_idx_diff);
        }
        
        //spkr_samples[sample_idx] = spkr_level;
        for ( int i = spkr_sample_idx; i < spkr_buf_size + spkr_extra_buf; i++ ) {
            spkr_samples[i] = spkr_level;
        }
    }
}


#define SPKR_PLAY_DELAY 2
int playDelay = SPKR_PLAY_DELAY;


void spkr_update() {
    if ( ++spkr_frame_cntr >= spkr_fps_divider ) {
        spkr_frame_cntr = 0;
        
        // Fix: Unqueue was not working properly some cases, so we need to monitor
        //      queued elements and if there are too many, let's just wait
#define SPKR_MAX_QUEUED 10
        ALint queued = 0;
        alGetSourcei ( spkr_src[SPKR_SRC_GAME_SFX], AL_BUFFERS_QUEUED, &queued );
        al_check_error();
        //        printf("q:%d clkfrm:%d frm:%llu max:%llu\n", queued, clkfrm, clk_6502_per_frm, clk_6502_per_frm_max);
        
        if ( ( spkr_play_time ) && ( queued < SPKR_MAX_QUEUED ) ) {
            if ( freeBuffers ) {
                double multiplier = 1;
#ifdef SPKR_KEEP_PITCH
                if (MHz_6502 > default_MHz_6502 ) {
                    multiplier =  1 / MHz_6502;
                }
#endif
                
                // in Game Mode do not fade out and stop playing
                if ( /*( cpuMode_game != cpuMode ) && */( --spkr_play_time == 0 ) ) {
                    
                    if (--freeBuffers < 0) {
                        printf("freeBuffer < 0 (%i)\n", freeBuffers);
                        freeBuffers = 0;
                    }
                    
                    // Need to Cool Down Speaker -- Soft Fade to Zero
                    else {
                        
                        spkr_fade(spkr_level, 0);
                        
#ifdef SPKR_DEBUG
                        fwrite(spkr_samples, sizeof(spkr_sample_t), spkr_buf_size, spkr_debug_file);
                        fflush(spkr_debug_file);
#endif

                        alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, spkr_buf_size * sizeof(spkr_sample_t), spkr_sample_rate);
                        al_check_error();

//                            // DEBUG ONLY!!!
//                            fwrite(spkr_samples + spkr_buf_size * (BUFFER_COUNT - 2), sizeof(spkr_sample_t), spkr_buf_size, af);
//                            fflush(af);

                        alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
                        al_check_error();
                        
                    }
                    
                    memset(spkr_samples, 0, sample_buf_array_len * sizeof(spkr_sample_t)); // spkr_buf_size * sizeof(spkr_sample_t) * SPKR_CHANNELS);
                    spkr_sample_idx = 0;
                    spkr_sample_last_idx = 0;
                    spkr_level = SPKR_LEVEL_ZERO;
                    spkr_level_ema = SPKR_LEVEL_ZERO;
                    
//                    spkr_last_level = SPKR_LEVEL_ZERO;
                    
                }
                else {
// push a click into the speaker buffer
// (we will play the entire buffer at the end of the frame)
//                    spkr_sample_idx = ( (spkr_clk + m6502.clkfrm) / ( MHZ(default_MHz_6502) / spkr_sample_rate)) * SPKR_CHANNELS;

//                    // DEBUG spike
//                    spkr_sample_idx = 0;
//                    spkr_samples[ spkr_sample_idx++ ] = -28000;
//                    spkr_samples[ spkr_sample_idx++ ] = 28000; // stereo
//                    spkr_samples[ spkr_sample_idx++ ] = -28000;
//                    spkr_samples[ spkr_sample_idx++ ] = 28000; // stereo
//                    spkr_samples[ spkr_sample_idx++ ] = spkr_level;
//                    spkr_samples[ spkr_sample_idx++ ] = spkr_level; // stereo


//                    //spkr_samples[sample_idx] = spkr_level;
//                     memset(spkr_samples + spkr_sample_idx, spkr_level, spkr_buf_alloc_size * DEFAULT_FPS - spkr_sample_idx);
                    
                    if (--freeBuffers < 0) {
                        printf("freeBuffer < 0 (%i)\n", freeBuffers);
                        freeBuffers = 0;
                    }
                    
                    // Normal Sound Buffer Feed
                    else {
                        // finish the aquare wave
                        while ( spkr_sample_last_idx < spkr_buf_size ) {
                            spkr_samples[ spkr_sample_last_idx++ ] = spkr_level;
                            spkr_samples[ spkr_sample_last_idx++ ] = spkr_level; // stereo
                        }


                        int buf_len = round((double)spkr_buf_size * multiplier + spkr_extra_buf) * sizeof(spkr_sample_t);
                        
#ifdef SPKR_DEBUG // Debug SPKR Buffer Before EMA
#ifdef SPKR_DEBUG_BEFORE_EMA
                        fwrite(spkr_samples, 1, buf_len, spkr_debug_file);
                        fflush(spkr_debug_file);
#endif
#endif

                        static const int ema_len = 30; // soft: 70;
                        
                        int i = 0;
                        while ( i <= spkr_buf_size ) {
                            spkr_level_ema  = ema(spkr_samples[i], spkr_level_ema, ema_len);

                            // super smooth Quadratic EMA
                            spkr_samples[i++] = spkr_level_ema;
                            spkr_samples[i++] = spkr_level_ema;
                        }
                        
#ifdef SPKR_DEBUG // Debug SPKR Buffer After EMA
#ifdef SPKR_DEBUG_AFTER_EMA
                        fwrite(spkr_samples, 1, buf_len, spkr_debug_file);
                        fflush(spkr_debug_file);
#endif
#endif

                        alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, buf_len, spkr_sample_rate * multiplier);
                        al_check_error();
                        alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
                        al_check_error();
                    }
                }
                
                ALenum state;
                alGetSourcei( spkr_src[SPKR_SRC_GAME_SFX], AL_SOURCE_STATE, &state );
//                al_check_error();
                
                switch (state) {
                    case AL_PAUSED:
                        if ( --playDelay <= 0 ) {
                            alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
                            playDelay = SPKR_PLAY_DELAY;
                        }
                        break;
                        
                    case AL_PLAYING:
                        // already playing
                        break;
                        
                    case AL_INITIAL:
                    case AL_STOPPED:
                    default:
                        alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
                        // this is so we will set state to AL_PAUSED immediately
                        // As a result there will be an extra queued buffer
                        // which gives us a glitch free sound
                        alSourcePause(spkr_src[SPKR_SRC_GAME_SFX]);
                        break;
                }
                
//                int dst = 0;
                int src = spkr_buf_size * multiplier + spkr_extra_buf;
                int size = max(0, (int)spkr_sample_idx - src);
                
                if (size) {
                    memcpy(spkr_samples, spkr_samples + src * sizeof(spkr_sample_t), size * sizeof(spkr_sample_t) * SPKR_CHANNELS );
                }
                
                memset(spkr_samples + size, 0, spkr_buf_size * sizeof(spkr_sample_t));
                
//                if ( spkr_sample_idx >= size ) {
//                    spkr_sample_idx -= size;
//                }
//                else {
                    spkr_sample_idx = 0;
//                }
                
                spkr_sample_last_idx = 0;
                
            }
            else {
                printf("Warning: No FreeBuffers!\n");
            }
            
        }
//        else {
//            printf("spkr_play_time: %u, queued: %i\n", spkr_play_time, queued);
//        }
        
        spkr_clk = 0;
        
    }
    else {
        spkr_clk += m6502.clkfrm;
    }
    
    // free up unused buffers
    spkr_unqueue( spkr_src[SPKR_SRC_GAME_SFX] );
}


void spkr_playqueue_sfx( ALuint src, uint8_t * sfx, int len ) {
    if ( freeBuffers ) {
        ALenum queued;
        alGetSourcei( src, AL_BUFFERS_QUEUED, &queued );
        
        if ( queued < 16 ) {
            if (--freeBuffers < 0) {
                printf("freeBuffer < 0 (%i)\n", freeBuffers);
                freeBuffers = 0;
            }
            else {
                alBufferData( spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, sfx, len, sfx_sample_rate );
                al_check_error();
                alSourceQueueBuffers( src, 1, &spkr_buffers[freeBuffers] );
                al_check_error();
            }
            
            ALenum state;
            alGetSourcei( src, AL_SOURCE_STATE, &state );
//            al_check_error();
            
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
                if (--freeBuffers < 0) {
                    printf("freeBuffer < 0 (%i)\n", freeBuffers);
                    freeBuffers = 0;
                }
                else {
                    alBufferData( spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, sfx, len, sfx_sample_rate );
                    al_check_error();
                    alSourceQueueBuffers( src, 1, &spkr_buffers[freeBuffers] );
                    al_check_error();
                }
                
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
            break;
            
        default:
            break;
    }
    
    // free up unused buffers
    spkr_unqueue( src );
}


void spkr_play_disk_motor() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= FRAME(iicplus_MHz_6502) ) ) {
        spkr_play_sfx( spkr_src[SPKR_SRC_DISK_MOTOR_SFX], diskmotor_sfx, diskmotor_sfx_len );
    }
}

void spkr_stop_disk_motor( int time ) {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= FRAME(iicplus_MHz_6502) ) ) {
        spkr_play_disk_motor_time = time;
    }
}


void spkr_play_disk_arm() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= FRAME(iicplus_MHz_6502) ) ) {
        if ( spkr_play_disk_ioerr_time == 0 ) {
            spkr_play_sfx( spkr_src[SPKR_SRC_DISK_ARM_SFX], diskarm_sfx, diskarm_sfx_len );
            spkr_play_disk_arm_time = 2;
        }
    }
}


void spkr_play_disk_ioerr() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= FRAME(iicplus_MHz_6502) ) ) {
        spkr_playqueue_sfx( spkr_src[SPKR_SRC_DISK_IOERR_SFX], diskioerr_sfx, diskioerr_sfx_len);
        spkr_play_disk_ioerr_time = 4;
    }
}


void spkr_stopAll() {
    for ( int i = 0; i < SOURCES_COUNT; i++ ) {
        spkr_stop_sfx( spkr_src[i] );
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
    if ( ( ! disk_sfx_enabled ) || ( clk_6502_per_frm > FRAME(iicplus_MHz_6502) ) ) {
        if ( spkr_play_disk_motor_time ) {
            spkr_play_disk_motor_time = 1; // rest will be taken care below
        }
        if ( spkr_play_disk_arm_time ) {
            spkr_play_disk_arm_time = 1; // rest will be taken care below
        }
    }
    
    update_disk_sfx( &spkr_play_disk_motor_time, spkr_src[SPKR_SRC_DISK_MOTOR_SFX] );
    update_disk_sfx( &spkr_play_disk_arm_time, spkr_src[SPKR_SRC_DISK_ARM_SFX] );
    update_disk_sfx( &spkr_play_disk_ioerr_time, spkr_src[SPKR_SRC_DISK_IOERR_SFX] );

}

