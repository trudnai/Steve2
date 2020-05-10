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


const int spkr_fps = 20;
const int spkr_seconds = 1;
const unsigned spkr_sample_rate = 44100;
const unsigned spkr_buf_size = spkr_seconds * spkr_sample_rate;
char spkr_samples [ spkr_buf_size ];
unsigned spkr_sample_idx = 0;

// initialize OpenAL
void spkr_init() {
    
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    printf( "Default device: %s\n", defname );
    
    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    
    // Fill buffer with zeros
    memset( spkr_samples, spkr_level, spkr_buf_size );
    
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

void spkr_play() {
    if ( spkr_sample_idx ) {
        
        if ( spkr_src ) {
            alSourceStop(spkr_src);
            al_check_error();
            alSourcei(spkr_src, AL_BUFFER, 0);
            al_check_error();
        }
        
        if ( spkr_buf ) {
            alDeleteBuffers(1, &spkr_buf);
            al_check_error();
        }
        
        // Create buffer to store samples
        spkr_buf = 0;
        alGenBuffers(1, &spkr_buf);
        al_check_error();
        

        // Download buffer to OpenAL
        alBufferData(spkr_buf, AL_FORMAT_MONO8, spkr_samples, spkr_buf_size / spkr_fps, spkr_sample_rate);
        al_check_error();
            
        // Set-up sound source and play buffer
        spkr_src = 0;
        alGenSources(1, &spkr_src);
        alSourcei(spkr_src, AL_BUFFER, spkr_buf);
        alSourcei(spkr_src, AL_LOOPING, 0);
        
        alSourcePlay(spkr_src);
        
        ALint secoffset = 0;
        alGetSourcei( spkr_src, AL_BYTE_OFFSET, &secoffset );
        
        // ccopy slack buffer to the top, so we will not lose the edges
        memcpy(spkr_samples, spkr_samples + secoffset, spkr_buf_size - spkr_buf_size / spkr_fps);
        // clear the slack buffer , so we can fill it up by new data
        memset(spkr_samples + secoffset, spkr_level, spkr_buf_size - spkr_buf_size / spkr_fps);

        // start from the beginning
        spkr_sample_idx = 0;
    }
    else {
        alSourceStop(spkr_src);
    }
}
