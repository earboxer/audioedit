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

#define __DEBUG__

/*
 * Prototypes
 */
static WavHeader *ConstructTrimedHeader(const WavHeader * header,
                                        const uint32_t new_num_samples,
                                        const uint32_t
                                        num_samples_to_skip);

static WavHeader *ConstructMergedHeader(const WavHeader * first_header,
                                        const WavHeader * second_header);

static WavHeader *CopyDataFromFileOrDie(const char *fin_path);

static Status   WriteDataOrDie(const void *data, const char *fout_path,
                               const uint64_t size, const int is_appended);

static void     RainCheck(const WavHeader * header);

static inline uint32_t CalculateNumSamples(const WavHeader * header);
static inline float CalculateLengthInSecond(const WavHeader * header);
static inline uint32_t CalculateOffset(const WavHeader * header,
                                       const uint32_t num_samples_to_skip);

/*
 * Trims the specified WAV file.
 */
Status
Trim(const char *fin_path, const uint32_t begin_num_samples_to_trim,
     const uint32_t end_num_samples_to_trim, const char *fout_path)
{
    WavHeader      *ptr_original_header = NULL,
        *ptr_new_header = NULL;
    uint64_t        fout_num_samples;
    uint64_t        num_samples_to_trim =
        begin_num_samples_to_trim + end_num_samples_to_trim;

    ptr_original_header = CopyDataFromFileOrDie(fin_path);
    check_ptr(ptr_original_header);

    check(num_samples_to_trim < ptr_original_header->num_samples,
          "The specified number \"%ld\" of samples is not valid.",
          num_samples_to_trim);

    fout_num_samples =
        ptr_original_header->num_samples - num_samples_to_trim;

    ptr_new_header =
        ConstructTrimedHeader(ptr_original_header, fout_num_samples,
                              begin_num_samples_to_trim);
    check_ptr(ptr_new_header);

#ifdef __DEBUG__
    printf("Original file:\t%s\tOriginal number of samples:\t%ld\n",
           fin_path, (long) ptr_original_header->num_samples);
    printf("Output file:\t%s\tOutput number of samples:\t%ld\n",
           fout_path, (long) ptr_new_header->num_samples);
#endif

    if (WriteDataOrDie(ptr_new_header, fout_path, kTotalHeaderSize, 0) !=
        SUCCESS)
        goto error;
    if (WriteDataOrDie(ptr_new_header->content, fout_path,
                       ptr_new_header->subchunk2_size, 1) != SUCCESS)
        goto error;
    /*
     * Clean-up
     */
    FREEMEM_(ptr_original_header->content);
    FREEMEM_(ptr_original_header);
    FREEMEM_(ptr_new_header);
    return SUCCESS;
  error:
    FREEMEM_(ptr_original_header->content);
    FREEMEM_(ptr_original_header);
    FREEMEM_(ptr_new_header);
    return FAILURE;
}

/*
 * Merges two WAV files into one.
 */
Status
Merge(const char *first_fin_path, const char *second_fin_path,
      const char *fout_path)
{

    WavHeader      *first_fin_header = NULL,
        *second_fin_header = NULL,
        *fout_header = NULL;

    first_fin_header = CopyDataFromFileOrDie(first_fin_path);
    check_ptr(first_fin_header);

    second_fin_header = CopyDataFromFileOrDie(second_fin_path);
    check_ptr(second_fin_header);

    fout_header =
        ConstructMergedHeader(first_fin_header, second_fin_header);
    check_ptr(fout_header);

#ifdef __DEBUG__
    printf("Original file:\t%s\tOriginal number of samples:\t%ld\n",
           first_fin_path, (long) first_fin_header->num_samples);
    printf("Original file:\t%s\tOriginal number of samples:\t%ld\n",
           second_fin_path, (long) second_fin_header->num_samples);
    printf("Output file:\t%s\tOutput number of samples:\t%ld\n",
           fout_path, (long) fout_header->num_samples);
#endif

    WriteDataOrDie(fout_header, fout_path, kTotalHeaderSize, 0);
    WriteDataOrDie(first_fin_header->content, fout_path,
                   first_fin_header->subchunk2_size, 1);
    WriteDataOrDie(second_fin_header->content, fout_path,
                   second_fin_header->subchunk2_size, 1);
    /*
     * Clean up
     */
    FREEMEM_(first_fin_header->content);
    FREEMEM_(first_fin_header);
    FREEMEM_(second_fin_header->content);
    FREEMEM_(second_fin_header);
    FREEMEM_(fout_header);
    return SUCCESS;
  error:
    FREEMEM_(first_fin_header->content);
    FREEMEM_(first_fin_header);
    FREEMEM_(second_fin_header->content);
    FREEMEM_(second_fin_header);
    FREEMEM_(fout_header);
    return FAILURE;
}


static WavHeader *
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
    new_header->chunk_size =
        kTotalHeaderSize - sizeof(new_header->subchunk2_size) +
        new_header->subchunk2_size;
    CalculateLengthInSecond(new_header);

    new_header->content = original_header->content +
        CalculateOffset(original_header, begin_num_samples_to_skip);

    return new_header;

  error:
    FREEMEM_(new_header->content);
    FREEMEM_(new_header);

    return NULL;
}

static WavHeader *
ConstructMergedHeader(const WavHeader * first_header,
                      const WavHeader * second_header)
{
    WavHeader      *new_header = (WavHeader *) malloc(sizeof(*new_header));
    check_mem(new_header);

    memcpy(new_header, first_header, sizeof(*new_header));

    new_header->subchunk2_size =
        first_header->subchunk2_size + second_header->subchunk2_size;
    new_header->chunk_size =
        kTotalHeaderSize - sizeof(new_header->subchunk2_size)
        + new_header->subchunk2_size;
    new_header->num_samples =
        first_header->num_samples + second_header->num_samples;
    CalculateLengthInSecond(new_header);

    return new_header;

  error:
    FREEMEM_(new_header->content);
    FREEMEM_(new_header);

    return NULL;
}


static WavHeader *
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

static          Status
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
#define checkId(A, M) check((strncmp((char *) &(A), (M), 4) == 0), M);
    checkId(header->chunk_id, "RIFF");
    checkId(header->format, "WAVE");
    checkId(header->subchunk1_id, "fmt ");
    checkId(header->subchunk2_id, "data");

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
