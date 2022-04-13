#!/usr/local/bin/python3

### Convert SPKR Buffer to WAV -- (c) by Tamas Rudnai 2022
###
### This utility convert SPKR buffer output to an uncompressed WAV file
###
### Purpose is that sound generation can be analyzed by audio editor - Audacity for example
###

import os
import sys
import struct

# binpath = '/Users/trudnai/Library/Containers/com.trudnai.steveii/Data/'
# binfilename = binpath + 'steve2_audio_debug.bin'

def convert_bin_to_audio(filename):
    data_size = os.path.getsize(filename)
    channels = 2
    sample_rate = 192000 # must be the same as in speaker.c : spkr_sample_rate
    bits_per_sample = 16
    header_size = 44

    with open( filename, 'rb') as binfile:
        wavfilename = os.path.splitext( os.path.basename(filename) )[0] + '.wav'

        with open( wavfilename, 'wb+') as wavfile:
            # WAV HEADER
            wavfile.write( 'RIFF'.encode() ) # Marks the file as a riff file. Characters are each 1 byte long
            wavfile.write( struct.pack('I', data_size + header_size ) ) # Size of the overall file - 8 bytes, in bytes (32-bit integer). Typically you’d fill this in after creation
            wavfile.write( 'WAVE'.encode() ) # File Type Header. For our purposes, it always equals “WAVE”
            
            # FMT HEADER
            wavfile.write( 'fmt '.encode() ) # Format chunk marker. Includes trailing null (tr: Space works, null don't)
            wavfile.write( struct.pack('I', 16 ) )  # Length of format data as listed above
            
            # FMT DATA
            wavfile.write( struct.pack('H', 1 ) )   # Type of format (1 is PCM) - 2 byte integer
            wavfile.write( struct.pack('H', channels ) )    # Number of Channels - 2 byte integer
            wavfile.write( struct.pack('I', sample_rate ) )    # Sample Rate - 32 byte integer. Common values are 44100 (CD), 48000 (DAT). Sample Rate = Number of Samples per second, or Hertz
            wavfile.write( struct.pack('I', int(sample_rate * bits_per_sample * channels / 8) ) ) # (Sample Rate * BitsPerSample * Channels) / 8
            wavfile.write( struct.pack('H', 4 ) )   # 0: (BitsPerSample * Channels) / 8
                                                    # 1: 8 bit mono2
                                                    # 2: 8 bit stereo
                                                    # 3: 16 bit mono
                                                    # 4: 16 bit stereo
            wavfile.write( struct.pack('H', bits_per_sample ) )   # Bits per sample
            
            # DATA HEADER
            wavfile.write( 'data'.encode() ) # “data” chunk header. Marks the beginning of the data section
            wavfile.write( struct.pack('I', data_size ) ) # Size of the data section
            
            # // WAV DATA
            wavfile.write( binfile.read() )

for filename in sys.argv:
    convert_bin_to_audio(filename)


