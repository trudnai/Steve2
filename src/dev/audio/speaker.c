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


#define SPKR_OVERSAMPLING 32 // 8 and 10 sounds ok, 16 is better, 32 is really good, 64 is not better than 32


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


#define __al_check_error2(file,line,src) \
    for( ALenum err = alGetError(); err != AL_NO_ERROR; err = alGetError() ) { \
        printf( "AL Error %s at %s:%d (%u)\n", al_err_str(err), file, line, src ); \
    }

#define al_check_error2(src) \
    __al_check_error2(__FILE__, __LINE__, src)


ALCdevice *dev = NULL;
ALCcontext *ctx = NULL;


#define SPKR_MIN_VOL    0.0001 // OpenAL cannot change volume to 0.0 for some reason, so we just turn volume really low
int spkr_att = 0;
int spkr_level = SPKR_LEVEL_ZERO;
int spkr_level_ema = SPKR_LEVEL_ZERO;
int spkr_level_dema = SPKR_LEVEL_ZERO;
int spkr_level_tema = SPKR_LEVEL_ZERO;
int spkr_last_level = SPKR_LEVEL_ZERO;


//static const int ema_len_sharper = 7;
//static const int ema_len_sharp = 14;
//static const int ema_len_oversampled = 80;
//static const int ema_len_normal = 16; // 18
//static const int ema_len_soft = 20;
//static const int ema_len_supersoft = 40;

#if (SPKR_OVERSAMPLING >= 64)
int spkr_ema_len = 128;
#elif (SPKR_OVERSAMPLING >= 32)
int spkr_ema_len = 64;
#elif (SPKR_OVERSAMPLING >= 16)
int spkr_ema_len = 32;
#elif (SPKR_OVERSAMPLING >= 8)
int spkr_ema_len = 24;
#elif (SPKR_OVERSAMPLING >= 2)
int spkr_ema_len = 20;
#else
int spkr_ema_len = 16;
#endif

#define BUFFER_COUNT 16
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

#define SPKR_BUF_SIZE           ( spkr_sample_rate * SPKR_CHANNELS / DEFAULT_FPS ) // stereo
#define SPKR_BUF_SLOT(n)        ( SPKR_BUF_SIZE * (n) )
#define SPKR_BUF_SLOT_SIZE(n)   ( SPKR_BUF_SLOT(n) * sizeof(spkr_sample_t) )

#define SPKR_STRM_SIZE          ( spkr_stream_rate * SPKR_CHANNELS / DEFAULT_FPS ) // stereo
#define SPKR_STRM_SLOT(n)       ( SPKR_STRM_SIZE * (n) )
#define SPKR_STRM_SLOT_SIZE(n)  ( SPKR_STRM_SLOT(n) * sizeof(spkr_sample_t) )

#define SPKR_SILENT_SLOT    1



const unsigned spkr_seconds = 1;

#ifdef SPKR_OVERSAMPLING
const unsigned spkr_stream_rate = 44100; // Optimal? 220500; // Best: 321000; // Acceptable: 192000; // higher the sample rate, the better the sound gets
const unsigned spkr_sample_rate = spkr_stream_rate * SPKR_OVERSAMPLING;
#else
const unsigned spkr_sample_rate = 321000; // Optimal? 220500; // Best: 321000; // Acceptable: 192000; // higher the sample rate, the better
#endif

const unsigned sfx_sample_rate =  22050; // original sample rate
//const unsigned sfx_sample_rate =  26000; // bit higher pitch
int spkr_extra_buf = 0; // 26; // 800 / spkr_fps;
typedef int16_t spkr_sample_t;
const unsigned spkr_buf_size = spkr_seconds * spkr_sample_rate * SPKR_CHANNELS / DEFAULT_FPS; // stereo
const unsigned spkr_buf_alloc_size = spkr_buf_size * sizeof(spkr_sample_t);
const unsigned sample_buf_array_len = SPKR_BUF_SLOT(BUFFER_COUNT + SPKR_SILENT_SLOT);
spkr_sample_t spkr_sample_buf [ sample_buf_array_len ]; // can store up to 1 sec of sound
spkr_sample_t * spkr_samples = spkr_sample_buf + SPKR_BUF_SLOT(SPKR_SILENT_SLOT); // keep 1 "empty" frame ahead

#ifdef SPKR_OVERSAMPLING
const unsigned sample_strm_array_len = SPKR_STRM_SLOT(BUFFER_COUNT + SPKR_SILENT_SLOT);
spkr_sample_t spkr_stream_buf [ sample_strm_array_len ]; // can store up to 1 sec of sound
spkr_sample_t * spkr_stream = spkr_stream_buf + SPKR_STRM_SLOT(SPKR_SILENT_SLOT); // keep 1 "empty" frame ahead
#endif

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

#undef SPKR_KEEP_PITCH

#undef SPKR_DEBUG

#ifdef SPKR_DEBUG
static const char * spkr_debug_raw_filename = "steve2_audio_debug_raw.bin";
static const char * spkr_debug_ema_filename = "steve2_audio_debug_ema.bin";
FILE * spkr_debug_raw_file = NULL;
FILE * spkr_debug_ema_file = NULL;
#endif


void spkr_fade(float fadeLevel) {
    // Fade
    
    int idx = 0;
    
    float fadeSlope = fadeLevel / spkr_buf_size * SPKR_CHANNELS;
    
//    while ( ( fabs(fadeLevel) >= fabs(fadeSlope) ) && ( idx < SPKR_BUF_SLOT_SIZE(1) ) ) {
    while ( idx < SPKR_BUF_SLOT_SIZE(1) ) {
        spkr_samples[ idx++ ] = fadeLevel;
        spkr_samples[ idx++ ] = fadeLevel; // stereo
        
        // how smooth we want the speeker to decay, so we will hear no pops and crackles
        fadeLevel -= fadeSlope;
    }
    
//    // Fill with Zero to avoid pops
//    memset(spkr_samples + idx, 0, SPKR_BUF_SLOT_SIZE(1));
    
    spkr_level = fadeLevel;
}


/// opens debug dump files for audio data -- if enabled
#ifdef SPKR_DEBUG
static void spkr_debug_init() {
    spkr_debug_raw_file = fopen(spkr_debug_raw_filename, "w+");
    spkr_debug_ema_file = fopen(spkr_debug_ema_filename, "w+");
}
#else
#define spkr_debug_init()
#endif


/// dumps audio data -- if enabled
#ifdef SPKR_DEBUG
static void spkr_debug(FILE * file) {
    fwrite(spkr_samples, sizeof(spkr_sample_t), spkr_buf_size, file);
    fflush(file);
}
#else
#define spkr_debug(f)
#endif


// initialize OpenAL
void spkr_init() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    dbgPrintf2( "Default device: %s\n", defname );
    
    // opens debug dump files for audio data -- if enabled
    spkr_debug_init();
    
    // restart OpenAL when restarting the virtual machine
    spkr_exit();
    
    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    
    // Fill buffer with zeros
    memset( spkr_sample_buf, 0, SPKR_BUF_SLOT_SIZE(BUFFER_COUNT + SPKR_SILENT_SLOT) );
    
//    freeBuffers--;
    
    // DEBUG ONLY!!!
    spkr_debug(spkr_debug_raw_file);
    spkr_debug(spkr_debug_ema_file);

//    alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, SPKR_BUF_SLOT_SIZE(1), spkr_sample_rate); // spkr_sample_idx * sizeof(spkr_sample_t), spkr_sample_rate);
//    al_check_error();
//    alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
//    al_check_error();

    
    // Create buffer to store samples
    alGenBuffers(BUFFER_COUNT, spkr_buffers);
    al_check_error();
    alGenBuffers(1, &spkr_disk_motor_buf);
    al_check_error();
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
//        al_check_error();
        al_check_error2(src);

    //    printf("%s alGetSourcei(%d)\n", __FUNCTION__, src);
//        printf("p:%d\n", processed);

        if ( processed > 0 ) {
//            printf("pf:%d\n", processed);
            alSourceUnqueueBuffers( src, processed, &spkr_buffers[freeBuffers]);
            al_check_error();
            freeBuffers = clamp( 0, freeBuffers + processed, BUFFER_COUNT );
            
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


#define _NO_SPKR_EARLY_ZERO_LEVEL 500

INLINE void spkr_finish_square(const unsigned new_idx) {
    // only fill small enough gaps and larger ones will go back to 0
#ifdef SPKR_EARLY_ZERO_LEVEL
    if ( (new_idx - spkr_sample_last_idx) < SPKR_EARLY_ZERO_LEVEL ) {
#endif
        // finish the aquare wave
        while ( spkr_sample_last_idx < new_idx ) {
            spkr_samples[ spkr_sample_last_idx++ ] = spkr_level;
            spkr_samples[ spkr_sample_last_idx++ ] = spkr_level; // stereo
        }
#ifdef SPKR_EARLY_ZERO_LEVEL
    }
    else {
        spkr_sample_last_idx = new_idx;
        spkr_level = 0;
    }
#endif
}


INLINE void spkr_toggle_square ( const int level_max ) {
    spkr_finish_square(spkr_sample_idx);
    spkr_level = level_max;
}


void spkr_toggle_tick ( int level_max, const unsigned idx_diff ) {
    spkr_level = level_max;
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
        
        // push a click into the speaker buffer
        // (we will play the entire buffer at the end of the frame)
        double multiplier = (double)spkr_sample_rate / MHZ(MHz_6502);
#ifdef SPKR_KEEP_PITCH
        if ( MHz_6502 < default_MHz_6502 ) {
            multiplier = (double)spkr_sample_rate / MHZ(default_MHz_6502);
//            indexer = (double)spkr_sample_rate / MHZ(default_MHz_6502) * MHz_6502;
        }
#endif

//        spkr_sample_idx = round( (spkr_clk + m6502.clkfrm) / ( MHZ(default_MHz_6502) / (double)spkr_sample_rate)) * SPKR_CHANNELS;
        spkr_sample_idx = round( (double)(spkr_clk + m6502.clkfrm) * multiplier ) * SPKR_CHANNELS;
        spkr_sample_idx &= UINTMAX_MAX - 1;
        
        // if play stopped, we should make sure we are not creating a false initial quare wave
        if (spkr_play_time <= 0) {
            spkr_sample_last_idx = spkr_sample_idx;
        }
        
        spkr_play_time = spkr_play_timeout;
        
        unsigned spkr_sample_idx_diff = spkr_sample_idx - spkr_sample_last_idx;

        if ( (int)spkr_sample_idx_diff < 0 ) {
            spkr_sample_idx_diff = UINT_MAX - spkr_sample_idx_diff;
        }

        if ( spkr_state ) {
            // down edge
            spkr_state = 0;
            
//            spkr_toggle_tick(SPKR_LEVEL_MIN, spkr_sample_idx_diff);
            spkr_toggle_square(SPKR_LEVEL_MIN);
        }
        else {
            // up edge
            spkr_state = 1;
            
//            spkr_toggle_tick(SPKR_LEVEL_MAX, spkr_sample_idx_diff);
            spkr_toggle_square(SPKR_LEVEL_MAX);
        }
        
        //spkr_samples[sample_idx] = spkr_level;
        for ( int i = spkr_sample_idx; i < spkr_buf_size + spkr_extra_buf; i++ ) {
            spkr_samples[i] = spkr_level;
        }
    }
}


#define SPKR_PLAY_DELAY 1
int playDelay = SPKR_PLAY_DELAY;


#define SPKR_FILTER_RESET

#ifdef SPKR_FILTER_RESET
INLINE static void spkr_filter_reset() {
    spkr_level = SPKR_LEVEL_ZERO;
    spkr_level_ema = SPKR_LEVEL_ZERO;
    spkr_level_dema = SPKR_LEVEL_ZERO;
    spkr_level_tema = SPKR_LEVEL_ZERO;
}
#endif

// Exponential Moving Average
INLINE int ema( int val, int prev, float ema_len ) {
    static const float ema_sensitivity = 2;
    //    static const float ema_len = 42;
    float m = ema_sensitivity / ema_len;
    float n = 1 - m;
    
    return m * val + n * prev;
}


INLINE static void spkr_filter_ema(spkr_sample_t * buf, int buf_size) {
    
    // to use with EMA
    //    static const int ema_len_sharper = 30;
    //    static const int ema_len_soft = 70;
    //    static const int ema_len_supersoft = 200;
    
    // to use with TEMA
    //    static const int ema_len_sharper = 7;
    //    static const int ema_len_sharp = 14;
    //    static const int ema_len_normal = 18;
    //    static const int ema_len_soft = 20;
    //    static const int ema_len_supersoft = 40;
    //
    //    static const int ema_len = ema_len_soft;
    
    for ( int i = 0; i < buf_size; ) {
        spkr_level_ema  = ema(buf[i], spkr_level_ema, spkr_ema_len);
        spkr_level_dema  = ema(spkr_level_ema, spkr_level_dema, spkr_ema_len);
        spkr_level_tema  = ema(spkr_level_dema, spkr_level_tema, spkr_ema_len);
        
        // smoothing with Tripple EMA
        buf[i++] = spkr_level_tema;
        buf[i++] = spkr_level_tema;
    }
    
    // Debug SPKR Buffer After EMA
    spkr_debug(spkr_debug_ema_file);
    
//    spkr_level = spkr_level_tema;
}


#ifdef SPKR_FILTER_SMA
INLINE static void spkr_filter_sma(int buf_len) {
    static const unsigned sma_len = 35;
    static spkr_sample_t sma_buf [ sma_len ] = {0};
    static int64_t sum = 0;

    for ( int i = 0; i < spkr_buf_size; ) {
        // before we feed the value, remove the one that comes out
        sum -= sma_buf[0];
        // move array down
        memcpy(sma_buf, sma_buf + sizeof(spkr_sample_t), (sma_len - 1) * sizeof(spkr_sample_t));
        // add new value
        sma_buf[sma_len - 1] = spkr_samples[i];
        sum += spkr_samples[i];
        // calculate average
        spkr_sample_t spkr_level = (int)(sum / sma_len);
        
        spkr_samples[i++] = spkr_level;
        spkr_samples[i++] = spkr_level;
    }
}
#endif


#ifdef SPKR_OVERSAMPLING
INLINE static spkr_sample_t spkr_avg(const spkr_sample_t * buf, const int len) {
    long sum = 0;

    // get the sum for that section
    for (int i = 0; i < len; i++) {
        sum += buf[ i * SPKR_CHANNELS ];
    }

    // get the average for that section
    return sum / len;
}

INLINE static void spkr_downsample() {
    for (int i = 0; i < SPKR_STRM_SLOT_SIZE(1); ) {
        int buf_idx = i * SPKR_OVERSAMPLING;
        
        // get the average for that section
        spkr_sample_t level = spkr_avg(spkr_samples + buf_idx, SPKR_OVERSAMPLING);

        // fill the downsampled version
        spkr_stream[i++] = level;
        spkr_stream[i++] = level;
    }
}
#endif


INLINE static void spkr_filter() {
    // Debug SPKR Buffer Before filters
    spkr_debug(spkr_debug_raw_file);
    

#ifdef SPKR_OVERSAMPLING
    spkr_filter_ema( spkr_samples, SPKR_BUF_SIZE );
    spkr_downsample();
#else
    spkr_filter_ema( spkr_samples, SPKR_BUF_SIZE );
#endif
    

#ifdef SPKR_FILTER_SMA
    spkr_filter_sma(buf_len);
#endif
}


INLINE void spkr_debug_spike() {
//    printf("spkr_debug_spike: %llu\n", m6502.clktime + m6502.clkfrm);
    int level = 28000;
    
    for (int i = 0; i < 20;) {
        spkr_samples[i++] = level;
        spkr_samples[i++] = level; // stereo
        
        level *= -1;
    }
}


void spkr_play() {
    ALenum state;
    alGetSourcei( spkr_src[SPKR_SRC_GAME_SFX], AL_SOURCE_STATE, &state );
//    al_check_error();
    
    switch (state) {
        case AL_PLAYING:
            // already playing, no need to do anything
            break;
            
        case AL_INITIAL:
        case AL_STOPPED:
        case AL_PAUSED:
        default:
            // no we can play this empty buffer first and then later on the real one
            alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
            break;
    }
}


//void spkr_play_with_prebuf() {
//    ALenum state;
//    alGetSourcei( spkr_src[SPKR_SRC_GAME_SFX], AL_SOURCE_STATE, &state );
//    //                al_check_error();
//
//    switch (state) {
//        case AL_PAUSED:
//            // should not be paused
//            alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
//            break;
//
//        case AL_PLAYING:
//            // already playing, no need to do anything
//            break;
//
//        case AL_INITIAL:
//        case AL_STOPPED:
//        default:
//            if (--freeBuffers < 0) {
//                printf("freeBuffer < 0 (%i)\n", freeBuffers);
//                freeBuffers = 0;
//            }
//
//            // Normal Sound Buffer Feed
//            else {
//                const int buf_len = round((double)spkr_buf_size + spkr_extra_buf) * sizeof(spkr_sample_t) * 2;
////                const int buf_len = 500;
////                printf("buf_len=%i\n", buf_len);
//                const spkr_sample_t * silent_samples = spkr_samples + spkr_buf_size * 100;
//
//                alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, silent_samples, buf_len, spkr_stream_rate);
//                al_check_error();
//                alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
//                al_check_error();
//            }
//
//            // no we can play this empty buffer first and then later on the real one
//            alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
//            break;
//    }
//}


void spkr_play_with_pause() {
    ALenum state;
    alGetSourcei( spkr_src[SPKR_SRC_GAME_SFX], AL_SOURCE_STATE, &state );
    //                al_check_error();
    
    switch (state) {
        case AL_PAUSED:
            if ( --playDelay < 0 ) {
//                printf("spkr_play_with_pause: PLAY\n");
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
//            printf("spkr_play_with_pause: PLAY-DELAY\n");
            alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
            // this is so we will set state to AL_PAUSED immediately
            // As a result there will be an extra queued buffer
            // which gives us a glitch free sound
            alSourcePause(spkr_src[SPKR_SRC_GAME_SFX]);
            playDelay = SPKR_PLAY_DELAY;
            break;
    }
}


void spkr_buffer_with_prebuf() {
    ALenum state;
    alGetSourcei( spkr_src[SPKR_SRC_GAME_SFX], AL_SOURCE_STATE, &state );
//    al_check_error();

    switch (state) {
        case AL_PAUSED:
//            printf("spkr_buffer_with_prebuf: AL_PAUSED %llu\n", m6502.clktime + m6502.clkfrm);
        case AL_PLAYING:
//            printf("spkr_buffer_with_prebuf: AL_PLAYING %llu\n", m6502.clktime + m6502.clkfrm);
            // it is already playing so we can just simply feed the next sound block
#ifdef SPKR_OVERSAMPLING
            alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_stream, SPKR_STRM_SLOT_SIZE(1), spkr_stream_rate);
#else
            alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, SPKR_BUF_SLOT_SIZE(1), spkr_sample_rate);
#endif
            al_check_error();
            alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
            al_check_error();
            break;

        case AL_INITIAL:
//            printf("spkr_buffer_with_prebuf: AL_INITIAL %llu\n", m6502.clktime + m6502.clkfrm);
        case AL_STOPPED:
        default:
//            printf("spkr_buffer_with_prebuf: AL_STOPPED %llu\n", m6502.clktime + m6502.clkfrm);
            // start with a silent sound block so later on we will not run out of buffer that easy
#ifdef SPKR_OVERSAMPLING
            alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_stream_buf, SPKR_STRM_SLOT_SIZE(SPKR_SILENT_SLOT + 1), spkr_stream_rate);
#else
            alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_sample_buf, SPKR_BUF_SLOT_SIZE(SPKR_SILENT_SLOT + 1), spkr_sample_rate);
#endif
            al_check_error();
            alSourceQueueBuffers(spkr_src[SPKR_SRC_GAME_SFX], 1, &spkr_buffers[freeBuffers]);
            al_check_error();
            
            break;
    }
    
    // play if needed
    switch (state) {
        case AL_PLAYING:
            break;
            
        case AL_INITIAL:
        case AL_PAUSED:
        case AL_STOPPED:
        default:
            // no we can play this empty buffer first and then later on the real one
            alSourcePlay(spkr_src[SPKR_SRC_GAME_SFX]);
            break;
    }

#ifdef SPKR_OVERSAMPLING
    memset(spkr_stream, 0, SPKR_STRM_SLOT_SIZE(1));
#else
    memset(spkr_samples, 0, SPKR_BUF_SLOT_SIZE(1));
#endif
}


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
        
        if ( queued < SPKR_MAX_QUEUED ) {
            if ( spkr_play_time > 0) {
                if ( freeBuffers ) {
//                    double multiplier = 1;
//    #ifdef SPKR_KEEP_PITCH
//                    if (MHz_6502 > default_MHz_6502 ) {
//                        multiplier =  1 / MHz_6502;
//                    }
//    #endif
                    
                    // in Game Mode do not fade out and stop playing
                    if ( /*( cpuMode_game != cpuMode ) && */( --spkr_play_time == SPKR_PLAY_QUIET ) ) {
                        
                        if (--freeBuffers < 0) {
                            printf("freeBuffer < 0 (%i)\n", freeBuffers);
                            freeBuffers = 0;
                        }
                        
                        // Need to Cool Down Speaker -- Soft Fade to Zero
                        else {
                            
                            spkr_fade(spkr_level);
                            spkr_filter();

    #ifdef SPKR_DEBUG
                            spkr_debug(spkr_debug_raw_file);
                            spkr_debug(spkr_debug_ema_file);
    #endif

                            spkr_buffer_with_prebuf();
                        }
                        
                        memset(spkr_samples, 0, SPKR_BUF_SLOT_SIZE(BUFFER_COUNT) );
                        spkr_sample_idx = 0;
                        spkr_sample_last_idx = 0;

#ifdef SPKR_FILTER_RESET
                        spkr_filter_reset();
#endif
                    }
                    else {
                        // push a click into the speaker buffer
                        // spkr_debug_spike();

                        if (--freeBuffers < 0) {
                            printf("freeBuffer < 0 (%i)\n", freeBuffers);
                            freeBuffers = 0;
                        }
                        
                        // Normal Sound Buffer Feed
                        else {
                            // finish the aquare wave
                            spkr_finish_square(spkr_buf_size);
                            // digital filtering the audio stream -- most notably smoothing
                            spkr_filter();
                            
//                            spkr_debug_spike();
                            
                            // play slot
                            spkr_buffer_with_prebuf();
                        }
                    }

//                    spkr_play();
//                    spkr_play_with_pause();

                    // shift sample buffer
                    memcpy(spkr_samples, spkr_samples + SPKR_BUF_SLOT_SIZE(1), SPKR_BUF_SLOT_SIZE(1) );
                    memset(spkr_samples + SPKR_BUF_SLOT_SIZE(1), 0, SPKR_BUF_SLOT_SIZE(1) );
                    
                    spkr_sample_idx -= spkr_buf_size;
                    while (spkr_sample_idx >= spkr_buf_size) {
                        spkr_sample_idx -= spkr_buf_size;
                    }
                    if ((int)spkr_sample_idx < 0) {
                        spkr_sample_idx = 0;
                    }
                    
                    spkr_sample_last_idx -= spkr_buf_size;
                    while (spkr_sample_last_idx >= spkr_buf_size) {
                        spkr_sample_last_idx -= spkr_buf_size;
                    }
                    
                    if ((int)spkr_sample_last_idx < 0) {
                        spkr_sample_last_idx = 0;
                    }
                    
                    if (spkr_sample_last_idx >= spkr_buf_size) {
                        printf("spkr_sample_last_idx >= spkr_buf_size\n");
                    }

                    // make sure it never goes below 0 (never overflows)
                    if ( (int)spkr_play_time < 0 ) {
                        spkr_play_time = 0;
                    }
                    
                }
                else {
                    printf("Warning: No FreeBuffers!\n");
                }
                
            }
//            else {
//              printf("spkr_play_time: %u, queued: %i\n", spkr_play_time, queued);
//            }
        }
        else {
            printf("spkr_update: Warning: Queued is too high: %i\n", queued);
        }
        
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
            spkr_unqueue(src);
            alSourcei(src, AL_BUFFER, 0);
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
            spkr_play_disk_arm_time = fps / 15;
        }
    }
}


void spkr_play_disk_ioerr() {
    if ( ( disk_sfx_enabled ) && ( clk_6502_per_frm <= FRAME(iicplus_MHz_6502) ) ) {
        spkr_playqueue_sfx( spkr_src[SPKR_SRC_DISK_IOERR_SFX], diskioerr_sfx, diskioerr_sfx_len);
        spkr_play_disk_ioerr_time = fps / 10; // 4 for 30 FPS, 8 for 60 FPS
    }
}


void spkr_stopAll() {
    for ( int i = 0; i < SOURCES_COUNT; i++ ) {
        spkr_stop_sfx( spkr_src[i] );
    }
}


void spkr_stop_game_sfx() {
    spkr_stop_sfx( spkr_src[SPKR_SRC_GAME_SFX] );
}


void spkr_stop_disk_sfx() {
    spkr_stop_sfx( spkr_src[SPKR_SRC_DISK_ARM_SFX] );
    spkr_stop_sfx( spkr_src[SPKR_SRC_DISK_MOTOR_SFX] );
    spkr_stop_sfx( spkr_src[SPKR_SRC_DISK_IOERR_SFX] );
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
    
    update_disk_sfx( &spkr_play_disk_arm_time, spkr_src[SPKR_SRC_DISK_ARM_SFX] );
    update_disk_sfx( &spkr_play_disk_motor_time, spkr_src[SPKR_SRC_DISK_MOTOR_SFX] );
    update_disk_sfx( &spkr_play_disk_ioerr_time, spkr_src[SPKR_SRC_DISK_IOERR_SFX] );

}

