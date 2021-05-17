#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "woz.h"

//
// Original code is from:
//
// MIT License
//
// Copyright (c) 2018 Thomas Harte
//
// https://github.com/TomHarte/dsk2woz
//
// Modified by Tamas Rudnai for Steve ][ emulator project (2020)
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

// Forward declarations; see definitions for documentation.
static uint32_t crc32(const uint8_t *buf, size_t size);
static void serialise_track(uint8_t *dest, const uint8_t *src, uint8_t track_number, bool is_prodos);

void sputs( const char * s, uint8_t ** buf ) {
    size_t len = strlen(s);
    memcpy(*buf, s, len);
    *buf += len;
}

size_t swrite( const uint8_t * s, size_t size, size_t ntimes, uint8_t ** buf ) {
    size_t len = size * ntimes;
    memcpy(*buf, s, len);
    *buf += len;
    return len;
}

int dsk2woz( const char * filename ) {

	// Attempt to read the standard DSK number of bytes into a buffer.
	FILE *const dsk_file = fopen(filename, "rb");
	if(!dsk_file) {
		printf("ERROR: could not open %s for reading\n", filename);
		return -2;
	}
	const size_t dsk_image_size = 35 * 16 * 256;
	uint8_t dsk[dsk_image_size];
	const size_t bytes_read = fread(dsk, 1, dsk_image_size, dsk_file);
	fclose(dsk_file);

	// Determine from the filename whether to use Pro-DOS sector order.
	const char *extension = filename + strlen(filename) - 3;
    // for some reason macos does not have stricmp, so we do it in a not-so-efficient way
    const bool is_prodos = (strcmp(extension, ".po") == 0) || (strcmp(extension, ".PO") == 0);

	// If the DSK image was too short, announce failure. Some DSK files
	// seem empirically to be too long, but it's unclear that the extra
	// bytes actually mean anything — they're usually not many.
	if(bytes_read != dsk_image_size) {
		printf("ERROR: DSK image too small\n");
		return -3;
	}

	// Create a buffer for the portion of the WOZ image that comes after
	// the 12-byte header. The header will house the CRC, which will be
	// calculated later.
	const size_t woz_image_size = 256 - 12 + 35*6656;
	uint8_t woz[woz_image_size];
	memset(woz, 0, sizeof(woz));

#define set_int32(location, value)	\
	woz[location] = (value) & 0xff;	\
	woz[location+1] = ((value) >> 8) & 0xff;	\
	woz[location+2] = ((value) >> 16) & 0xff;	\
	woz[location+3] = (value) >> 24;

	/*
		WOZ image item 1: an INFO chunk.
	*/
	strcpy((char *)&woz[0], "INFO");	// Chunk ID.
	set_int32(4, 60);					// Chunk size.
	woz[8] = 1;							// INFO version: 1.
	woz[9] = 1;							// Disk type: 5.25".
	woz[10] = 0;						// Write protection: disabled.
	woz[11] = 0;						// Cross-track synchronised image: no.
	woz[12] = 1;						// MC3470 fake bits have been removed: yes.
										// (or, rather, were never inserted)
															
	// Append creator, which needs to be padded out to 32
	// bytes with space characters.
	const char creator[] = "dsk2woz 1.0";
	const size_t creator_length = strlen(creator);
	assert(creator_length < 32);

	strcpy((char *)&woz[13], creator);
	memset(&woz[13 + strlen(creator)], 32 - strlen(creator), ' ');

	// Chunk should be padded with 0s to reach 60 bytes in length;
	// the buffer was memset to 0 at initialisation so that's implicit.



	/*
		WOZ image item 2: a TMAP chunk.
	*/
	strcpy((char *)&woz[68], "TMAP");		// Chunk ID.
	set_int32(72, 160);						// Chunk size.

	// This is a DSK conversion, so the TMAP table simply maps every
	// track that exists to:
	// (i) its integral position;
	// (ii) the quarter-track position before its integral position; and
	// (iii) the quarter-track position after its integral position.
	//
	// The remaining quarter-track position maps to nothing, which in
	// WOZ is indicated with a value of 255.

	// Let's start by filling the entire TMAP with empty tracks.
	memset(&woz[76], 0xff, 160);
	// Then we will add in the mappings.
	for(size_t c = 0; c < 35; ++c) {
		const size_t track_position = 76 + (c << 2);
		if(c > 0) woz[track_position - 1] = c;
		woz[track_position] = woz[track_position + 1] = c;
	}



	/*
		WOZ image item 3: a TRKS chunk.
	*/
	strcpy((char *)&woz[236], "TRKS");	// Chunk ID.
	set_int32(240, 35*6656);			// Chunk size.

	// The output pointer holds a byte position into the WOZ buffer.
	size_t output_pointer = 244;

	// Write out all 35 tracks.
	for(size_t c = 0; c < 35; ++c) {
		serialise_track(&woz[output_pointer], &dsk[c * 16 * 256], c, is_prodos);
		output_pointer += 6656;
	}
#undef set_int32



	/*
		WOZ image output.
	*/

    // we will write WOZ file to memory buffer
    
    woz_file_size = WOZ1_FILE_SIZE;
    woz_file_buffer = malloc(WOZ1_FILE_SIZE);
    if (woz_file_buffer == NULL) {
        perror("Not Enough Memory: ");
        return WOZ_ERR_BAD_DATA;
    }

    uint8_t * woz_buf = woz_file_buffer;
    
    sputs("WOZ1", &woz_buf);
    *woz_buf++ = 0xFF;
	sputs("\n\r\n", &woz_buf);

	const uint32_t crc = crc32(woz, sizeof(woz));
    *woz_buf++ = crc & 0xFF;
    *woz_buf++ = (crc >> 8) & 0xFF;
    *woz_buf++ = (crc >> 16) & 0xFF;
    *woz_buf++ = (crc >> 24) & 0xFF;

	const size_t length_written = swrite(woz, 1, sizeof(woz), &woz_buf);

	if(length_written != sizeof(woz)) {
		printf("ERROR: Could not write full WOZ image\n");
		return -6;
	}

	return WOZ_ERR_OK;
}



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



/*
	DSK sector serialiser. Constructs the 6-and-2 DOS 3.3-style on-disk
	representation of a DOS logical-order sector dump.
*/

/*!
	Appends a bit to a buffer at a supplied position, returning the
	position immediately after the bit. The first bit added to a buffer
	will be stored in the MSB of the first byte. The second will be stored in
	bit 6. The eighth will be stored in the MSB of the second byte. Etc.

	@param buffer The buffer to write into.
	@param position The position to write at.
	@param value An indicator of the bit to write. If this is zero then a 0 is written; otherwise a 1 is written.
	@return The position immediately after the bit.
*/
static size_t write_bit(uint8_t *buffer, size_t position, int value) {
	buffer[position >> 3] |= (value ? 0x80 : 0x00) >> (position & 7);
	return position + 1;
}

/*!
	Appends a byte to a buffer at a supplied position, returning the
	position immediately after the byte. This is equivalent to calling
	write_bit eight times, supplying bit 7, then bit 6, down to bit 0.

	@param buffer The buffer to write into.
	@param position The position to write at.
	@param value The byte to write.
	@return The position immediately after the byte.
*/
static size_t write_byte(uint8_t *buffer, size_t position, int value) {
	const size_t shift = position & 7;
	const size_t byte_position = position >> 3;

	buffer[byte_position] |= value >> shift;
	if(shift) buffer[byte_position+1] |= value << (8 - shift);

	return position + 8;
}

/*!
	Encodes a byte into Apple 4-and-4 format and appends it to a buffer.

	@param buffer The buffer to write into.
	@param position The position to write at.
	@param value The byte to encode and write.
	@return The position immediately after the encoded byte.
*/
static size_t write_4_and_4(uint8_t *buffer, size_t position, int value) {
	position = write_byte(buffer, position, (value >> 1) | 0xaa);
	position = write_byte(buffer, position, value | 0xaa);
	return position;
}

/*!
	Appends a 6-and-2-style sync word to a buffer.

	@param buffer The buffer to write into.
	@param position The position to write at.
	@return The position immediately after the sync word.
*/
static size_t write_sync(uint8_t *buffer, size_t position) {
	position = write_byte(buffer, position, 0xff);
	return position + 2; // Skip two bits, i.e. leave them as 0s.
}

/*!
	Converts a 256-byte source buffer into the 343 byte values that
	contain the Apple 6-and-2 encoding of that buffer.

	@param dest The at-least-343 byte buffer to which the encoded sector is written.
	@param src The 256-byte source data.
*/
static void encode_6_and_2(uint8_t *dest, const uint8_t *src) {
	const uint8_t six_and_two_mapping[] = {
		0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
		0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
		0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
		0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
		0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
		0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
		0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
		0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
	};

	// Fill in byte values: the first 86 bytes contain shuffled
	// and combined copies of the bottom two bits of the sector
	// contents; the 256 bytes afterwards are the remaining
	// six bits.
	const uint8_t bit_reverse[] = {0, 2, 1, 3};
	for(size_t c = 0; c < 84; ++c) {
		dest[c] =
			bit_reverse[src[c]&3] |
			(bit_reverse[src[c + 86]&3] << 2) |
			(bit_reverse[src[c + 172]&3] << 4);
	}
	dest[84] =
		(bit_reverse[src[84]&3] << 0) |
		(bit_reverse[src[170]&3] << 2);
	dest[85] =
		(bit_reverse[src[85]&3] << 0) |
		(bit_reverse[src[171]&3] << 2);

	for(size_t c = 0; c < 256; ++c) {
		dest[86 + c] = src[c] >> 2;
	}

	// Exclusive OR each byte with the one before it.
	dest[342] = dest[341];
	size_t location = 342;
	while(location > 1) {
		--location;
		dest[location] ^= dest[location-1];
	}

	// Map six-bit values up to full bytes.
	for(size_t c = 0; c < 343; ++c) {
		dest[c] = six_and_two_mapping[dest[c]];
	}
}

/*!
	Converts a DSK-style track to a WOZ-style track.

	@param dest The 6646-byte buffer that will contain the WOZ track. Both track contents and the
		proper suffix will be written.
	@param src The 4096-byte buffer that contains the DSK track — 16 instances of 256 bytes, each
		a fully-decoded sector.
	@param track_number The track number to encode into this track.
	@param is_prodos @c true if the DSK image is in Pro-DOS order; @c false if it is in DOS 3.3 order.
*/
static void serialise_track(uint8_t *dest, const uint8_t *src, uint8_t track_number, bool is_prodos) {
	size_t track_position = 0;	// This is the track position **in bits**.
	memset(dest, 0, 6646);

	// Write gap 1.
	for(size_t c = 0; c < 16; ++c) {
		track_position = write_sync(dest, track_position);
	}

	// Step through the sectors in physical order.
	for(size_t sector = 0; sector < 16; ++sector) {
		/*
			Write the sector header.
		*/

		// Prologue.
		track_position = write_byte(dest, track_position, 0xd5);
		track_position = write_byte(dest, track_position, 0xaa);
		track_position = write_byte(dest, track_position, 0x96);

		// Volume, track, setor and checksum, all in 4-and-4 format.
		track_position = write_4_and_4(dest, track_position, 254);
		track_position = write_4_and_4(dest, track_position, track_number);
		track_position = write_4_and_4(dest, track_position, sector);
		track_position = write_4_and_4(dest, track_position, 254 ^ track_number ^ sector);

		// Epilogue.
		track_position = write_byte(dest, track_position, 0xde);
		track_position = write_byte(dest, track_position, 0xaa);
		track_position = write_byte(dest, track_position, 0xeb);


		// Write gap 2.
		for(size_t c = 0; c < 7; ++c) {
			track_position = write_sync(dest, track_position);
		}

		/*
			Write the sector body.
		*/

		// Prologue.
		track_position = write_byte(dest, track_position, 0xd5);
		track_position = write_byte(dest, track_position, 0xaa);
		track_position = write_byte(dest, track_position, 0xad);

		// Map from this physical sector to a logical sector.
		const int logical_sector = (sector == 15) ? 15 : ((sector * (is_prodos ? 8 : 7)) % 15);

		// Sector contents.
		uint8_t contents[343];
		encode_6_and_2(contents, &src[logical_sector * 256]);
		for(size_t c = 0; c < sizeof(contents); ++c) {
			track_position = write_byte(dest, track_position, contents[c]);			
		}

		// Epilogue.
		track_position = write_byte(dest, track_position, 0xde);
		track_position = write_byte(dest, track_position, 0xaa);
		track_position = write_byte(dest, track_position, 0xeb);

		// Write gap 3.
		for(size_t c = 0; c < 16; ++c) {
			track_position = write_sync(dest, track_position);
		}
	}

	// Add the track suffix.
	dest[6646] = ((track_position + 7) >> 3) & 0xff;
	dest[6647] = ((track_position + 7) >> 11) & 0xff;	// Byte count.
	dest[6648] = track_position & 0xff;
	dest[6649] = (track_position >> 8) & 0xff;	// Bit count.
	dest[6650] = dest[6651] = 0x00;				// Splice information.
	dest[6652] = 0xff;
	dest[6653] = 10;
}
