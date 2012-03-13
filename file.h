/*-
 * Copyright (c) 2012, Metian Huang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution
 *
 * THIS SOFTWARE IS PROVIDED BY Metian Huang AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AN ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef FILE_H_
#define FILE_H_

#include <stdint.h>

/*
 * Offsets
 */
#define kChunkSizeOffset        4
#define kNumChannelsOffset      22
#define kSampleRateOffset       24
#define kBitPerSampleOffset     34
#define kSubchunk2SizeOffset    40
#define kDataOffset             44

/*
 * Sizes
 */
#define kChunkSizeSize          4
#define kNumChannelsSize        2
#define kSampleRateSize         4
#define kBitPerSampleSize       2
#define kSubchunk2SizeSize      4       /* Not a typo */
#define kTotalHeaderSize        44

typedef struct WavHeader {
    uint32_t        chunk_size;
    uint16_t        num_channels;
    uint32_t        sample_rate;
    uint16_t        bit_per_sample;
    uint32_t        subchunk2_size;
    uint64_t        num_samples;
    float           length_in_second;
} WavHeader;

WavHeader      *InitialHeader(const char *buffer);

WavHeader      *ConstructHeader(const WavHeader * header,
                                uint64_t new_num_samples);

#endif                          /* FILE_H_ */
