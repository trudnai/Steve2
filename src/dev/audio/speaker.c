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
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include "speaker.h"
#include "6502.h"


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
ALuint spkr_buf = 0;
ALuint spkr_src = 0;

// we start with the max, because otherwise the speaker clicks
int spkr_level = SPKR_LEVEL_MAX;


const int spkr_fps = fps;
const int spkr_seconds = 1;
const unsigned spkr_sample_rate = 44100;
unsigned spkr_extra_buf = 13; // TODO: Should it be a dynamic value calculated by how many bytes we overshot by the edge curve generator?
const unsigned spkr_buf_size = spkr_seconds * spkr_sample_rate / spkr_fps;
char spkr_samples [ spkr_buf_size * spkr_fps * 2]; // 1s of sound
unsigned spkr_sample_idx = 0;

const unsigned spkr_play_timeout = 10;
unsigned spkr_play_time = 0;


#define BUFFER_COUNT 10
#define SOURCES_COUNT 1

ALuint spkr_buffers[BUFFER_COUNT];


// initialize OpenAL
void spkr_init() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    printf( "Default device: %s\n", defname );
    
    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    
    // Fill buffer with zeros
    memset( spkr_samples, spkr_level, spkr_buf_size );
    
    
    // Create buffer to store samples
    alGenBuffers(BUFFER_COUNT, spkr_buffers);
    al_check_error();
    
    // Set-up sound source and play buffer
    alGenSources(1, &spkr_src);
    al_check_error();
    alSourcei(spkr_src, AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src, AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src, AL_POSITION, 0.0, 8.0, 0.0);
    al_check_error();
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
    al_check_error();

    // start from the beginning
    spkr_sample_idx = 0;

}

// Dealloc OpenAL
void spkr_exit() {
    ALCdevice *dev = NULL;
    ALCcontext *ctx = NULL;
    ctx = alcGetCurrentContext();
    dev = alcGetContextsDevice(ctx);
    
    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
    
    al_check_error();
}

void spkr_playStart() {
}


ALint freeBuffers = BUFFER_COUNT;
//ALuint alBuffers[BUFFER_COUNT];

void spkr_update() {
    if ( spkr_play_time ) {
        
//        printf("freeBuffers: %d", freeBuffers);
        
//        if ( spkr_src ) {
//            alSourceStop(spkr_src);
//            al_check_error();
//            alSourcei(spkr_src, AL_BUFFER, 0);
//            al_check_error();
//
//            spkr_src = 0;
//        }
//
//        if ( spkr_buf ) {
//            alDeleteBuffers(1, &spkr_buf);
//            al_check_error();
//            spkr_buf = 0;
//        }

        ALint processed = 0;
        do {
            alGetSourcei (spkr_src, AL_BUFFERS_PROCESSED, &processed);
//            if ( processed )
            freeBuffers += processed;
//            al_check_error();
//            usleep(100);
//            if ( freeBuffers <= 0 ) printf("No Free Buffer\n");
        } while( freeBuffers <= 0 );
        
        freeBuffers = clamp( 1, freeBuffers, BUFFER_COUNT );
        
//        printf("freeBuffers2: %d  processed: %d\n", freeBuffers, processed);
        
        ALenum state;
        alGetSourcei( spkr_src, AL_SOURCE_STATE, &state );
//        al_check_error();

        if ( processed ) {
            alSourceUnqueueBuffers( spkr_src, processed, &spkr_buffers[freeBuffers - processed]);
//            al_check_error();
        }
        
//        spkr_samples[0] = 0;
//        spkr_samples[1] = 255;
//        spkr_samples[2] = 0;
//        spkr_samples[3] = 255;
//        spkr_samples[4] = 0;

        // Download buffer to OpenAL
        
        if ( --spkr_play_time == 0 ) {
            // we need to soft mute the speaker to eliminate clicking noise
            // simple linear mute
            int step = (SPKR_LEVEL_ZERO - (int)spkr_level) / 128;
            if ( step != 0 ) {
                for ( spkr_sample_idx = 0; spkr_level != SPKR_LEVEL_ZERO; spkr_level += step) {
                    for ( int i = 0; i < 4; i++ ) {
                        spkr_samples[ spkr_sample_idx++ ] = spkr_level;
                    }
                }
                spkr_level = SPKR_LEVEL_ZERO;
                spkr_samples[ spkr_sample_idx++ ] = spkr_level;
                //spkr_samples[sample_idx] = spkr_level;
                memset(spkr_samples + spkr_sample_idx, spkr_level, spkr_extra_buf);

                freeBuffers--;
                alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_MONO8, spkr_samples, spkr_sample_idx, spkr_sample_rate);
                al_check_error();
                alSourceQueueBuffers(spkr_src, 1, &spkr_buffers[freeBuffers]);
                al_check_error();
            }
        }
        else {
            freeBuffers--;
            alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_MONO8, spkr_samples, spkr_buf_size + spkr_extra_buf, spkr_sample_rate);
            al_check_error();
            alSourceQueueBuffers(spkr_src, 1, &spkr_buffers[freeBuffers]);
            al_check_error();
        }
        
        switch (state) {
            case AL_PAUSED:
                alSourcePlay(spkr_src);
                break;

            case AL_PLAYING:
                // already playing
                break;
                
            default:
                alSourcePlay(spkr_src);
                alSourcePause(spkr_src);
                break;
        }
        
        // clear the slack buffer , so we can fill it up by new data
        memset(spkr_samples, spkr_level, spkr_buf_size + spkr_extra_buf);

        // start from the beginning
        spkr_sample_idx = 0;

    }
    else {
        // TODO: Need better speaker turn off logic to avoid click noise
////        if ( spkr_src ) {
////            alSourceStop(spkr_src);
////            al_check_error();
////            alSourceUnqueueBuffers( spkr_src, BUFFER_COUNT, spkr_buffers);
////            al_check_error();
////            alSourcei(spkr_src, AL_BUFFER, 0);
////            al_check_error();
////            spkr_src = 0;
////            // clear the buffer
////            memset(spkr_samples, spkr_level, spkr_buf_size);
////        }
////        printf("freeBuffers_nosound: %d\n", freeBuffers);
    }
}


//void spkr_Update() {
//    if ( spkr_src && spkr_buf ) {
//        if ( spkr_src ) {
//            alSourcePause(spkr_src);
//            al_check_error();
//            alSourcei(spkr_src, AL_BUFFER, 0);
//            al_check_error();
//        }
//
//        // Download buffer to OpenAL
//        alBufferData(spkr_buf, AL_FORMAT_MONO8, spkr_samples, spkr_buf_size, spkr_sample_rate);
//        al_check_error();
//
//        alSourcei( spkr_src, AL_BYTE_OFFSET, 0 );
//        al_check_error();
//        alSourcePlay(spkr_src);
//    }
//}
