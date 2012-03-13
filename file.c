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
#include "file.h"
#include "dbg.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void            CalculateLengthInSecond(WavHeader * header);

WavHeader      *
InitialHeader(const char *buffer)
{
    WavHeader      *header = (WavHeader *) malloc(sizeof(*header));
    check_mem(header);

    memcpy(&(header->chunk_size), buffer + kChunkSizeOffset,
           kChunkSizeSize);
    memcpy(&(header->num_channels), buffer + kNumChannelsOffset,
           kNumChannelsSize);
    memcpy(&(header->sample_rate), buffer + kSampleRateOffset,
           kSampleRateSize);
    memcpy(&(header->bit_per_sample), buffer + kBitPerSampleOffset,
           kBitPerSampleSize);
    memcpy(&(header->subchunk2_size), buffer + kSubchunk2SizeOffset,
           kSubchunk2SizeSize);

    header->num_samples =
        header->subchunk2_size * 8 / header->num_channels /
        header->bit_per_sample;

    CalculateLengthInSecond(header);
    return header;

  error:
    if (header)
        free(header);
    abort();
}

WavHeader      *
ConstructTrimedHeader(const WavHeader * original_header,
                      uint64_t target_num_samples)
{
    WavHeader      *new_header = (WavHeader *) malloc(sizeof(*new_header));
    check_mem(new_header);

    memcpy(new_header, original_header, sizeof(*new_header));

    /*
     * Adjust the header
     */
    new_header->subchunk2_size =
        original_header->subchunk2_size / original_header->num_samples *
        target_num_samples;
    new_header->chunk_size = 36 + new_header->subchunk2_size;
    new_header->num_samples = target_num_samples;
    CalculateLengthInSecond(new_header);

    return new_header;
  error:
    if (new_header)
        free(new_header);
    abort();
}

WavHeader      *
ConstructMergedHeader(const WavHeader * first_header,
                      const WavHeader * second_header)
{
    WavHeader      *new_header = (WavHeader *) malloc(sizeof(*new_header));
    check_mem(new_header);

    memcpy(new_header, first_header, sizeof(*new_header));

    new_header->subchunk2_size = first_header->subchunk2_size +
        second_header->subchunk2_size;
    new_header->chunk_size = 36 + new_header->subchunk2_size;
    new_header->num_samples = first_header->num_samples +
        second_header->num_samples;
    CalculateLengthInSecond(new_header);

    return new_header;

  error:
    if (new_header)
        free(new_header);
    abort();
}

void
CalculateLengthInSecond(WavHeader * header)
{
    header->length_in_second =
        (float) header->num_samples / header->num_channels /
        header->sample_rate;
}
