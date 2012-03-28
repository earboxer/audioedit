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
#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void     RainCheck(const WavHeader * header);
static inline uint32_t CalculateNumSamples(const WavHeader * header);
static inline float CalculateLengthInSecond(const WavHeader * header);
static inline uint32_t CalculateOffset(const WavHeader * header,
                                       const uint32_t num_samples_to_skip);

WavHeader      *
ConstructTrimedHeader(const WavHeader * original_header,
                      const uint32_t target_num_samples,
                      const uint32_t begin_num_samples_to_skip)
{
    WavHeader      *new_header = (WavHeader *) malloc(sizeof(*new_header));
    check_mem(new_header);

    memcpy(new_header, original_header, sizeof(*new_header));

    /*
     * Adjust the header
     */
    new_header->num_samples = target_num_samples;
    new_header->subchunk2_size =
        original_header->subchunk2_size / original_header->num_samples *
        target_num_samples;
    new_header->chunk_size = 36 + new_header->subchunk2_size;
    CalculateLengthInSecond(new_header);

    new_header->content =
        original_header->content + CalculateOffset(original_header,
                                                   begin_num_samples_to_skip);
    return new_header;

  error:
    FREEMEM_(new_header->content);
    FREEMEM_(new_header);

    return NULL;
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
    FREEMEM_(new_header->content);
    FREEMEM_(new_header);

    return NULL;
}


WavHeader      *
CopyDataFromFileOrDie(const char *fin_path)
{
    WavHeader      *header = NULL;
    FILE           *fin;

    header = (WavHeader *) malloc(sizeof(*header));
    check_mem(header);

    fin = fopen(fin_path, "rb");
    check(fin != NULL, "Cannot open the input file: %s", fin_path);

    FREAD_CHECK(header, fin, kTotalHeaderSize);

    header->content = (char *) malloc(header->subchunk2_size);
    check_mem(header->content);

    FREAD_CHECK(header->content, fin, header->subchunk2_size);

    /*
     * Content is already copied to memory. No need to use the input file.
     */
    CLOSEFD_(fin);

    RainCheck(header);

    header->num_samples = CalculateNumSamples(header);
    header->length_in_second = CalculateLengthInSecond(header);

    return header;

  error:
    CLOSEFD_(fin);
    FREEMEM_(header->content);
    FREEMEM_(header);

    return NULL;
}

Status
WriteDataOrDie(const void *data, const char *fout_path,
               const uint64_t size, int is_appended)
{
    FILE           *fout;

    if (is_appended == 1)
        fout = fopen(fout_path, "ab");
    else
        fout = fopen(fout_path, "wb");

    check(fout != NULL, "Cannot open the output file: %s", fout_path);

    FWRITE_CHECK(data, fout, size);
    CLOSEFD_(fout);

    return SUCCESS;

  error:
    CLOSEFD_(fout);
    fprintf(stderr, "Cannot write data to %s\n", fout_path);
    return FAILURE;
}

static void
RainCheck(const WavHeader * header)
{
    char            id[5];
    id[4] = '\0';
    memcpy(id, &(header->chunk_id), 4);
    check((strcmp(id, "RIFF") == 0), "chunk_id");
    memcpy(id, &(header->format), 4);
    check((strcmp(id, "WAVE") == 0), "format");
    memcpy(id, &(header->subchunk1_id), 4);
    check((strcmp(id, "fmt ") == 0), "subchunk1_id");
    memcpy(id, &(header->subchunk2_id), 4);
    check((strcmp(id, "data") == 0), "subchunk2_id");

    return;

  error:
    fputs(" ********************** ERROR ***********************\n"
          " YOU are screwed. Either you supply an invalid audio \n"
          " or you run this program on a wrong machine. Please  \n"
          " check your input or/and read README.md\n"
          " ****************************************************\n",
          stderr);
    exit(EXIT_FAILURE);
}

static inline   uint32_t
CalculateNumSamples(const WavHeader * header)
{
    return
        header->subchunk2_size / header->num_channels
        / header->bit_per_sample * 8;
}

static inline float
CalculateLengthInSecond(const WavHeader * header)
{
    return (float) header->num_samples / header->num_channels /
        header->sample_rate;
}

static inline   uint32_t
CalculateOffset(const WavHeader * header,
                const uint32_t num_samples_to_skip)
{
    return num_samples_to_skip * header->num_channels *
        header->bit_per_sample / 8;
}
