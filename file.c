/*-
 * Copyright (c) 2012 Meitian Huang.
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
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT OWNER AND CONTRIBUTORS "AS IS"
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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbg.h"
#include "file.h"
#include "utils.h"

#define __DEBUG__

/*
 * Prototypes
 */
static struct wav_header *trim_header(const struct wav_header *header,
                                      const uint32_t new_num_samples,
                                      const uint32_t num_samples_to_skip);

static struct wav_header *join_header(const struct wav_header *first_header,
                                      const struct wav_header *second_header);

static struct wav_header *read_data(const char *fin_path);

static Status   write_data(const void *data, const char *fout_path,
                           const uint64_t size, const Boolean append);

static Boolean  rain_check(const struct wav_header *header);

static inline uint32_t num_samples(const struct wav_header *header);
static inline float length_in_second(const struct wav_header *header);
static inline uint32_t skip(const struct wav_header *header,
                            const uint32_t num_samples_to_skip);

/*
 * Trims the specified WAVE file.
 */
Status
trim(const char *fin_path, const uint32_t begin_num_samples_to_trim,
     const uint32_t end_num_samples_to_trim, const char *fout_path)
{
    struct wav_header *original_header = NULL,
        *new_header = NULL;
    uint32_t        fout_num_samples;
    uint32_t        num_samples_to_trim =
        begin_num_samples_to_trim + end_num_samples_to_trim;

    /*
     * Reads header & data from fin.
     */
    original_header = read_data(fin_path);
    check_ptr(original_header);

    /*
     * Checks if the desired number of samples to trim is valid.
     */
    check(num_samples_to_trim < original_header->num_samples,
          "The specified number \"%d\" of samples is not valid.",
          num_samples_to_trim);

    fout_num_samples = original_header->num_samples - num_samples_to_trim;

    new_header = trim_header(original_header, fout_num_samples,
                             begin_num_samples_to_trim);
    check_ptr(new_header);

#ifdef __DEBUG__
    printf("Original file:\t%s\tOriginal number of samples:\t%ld\n",
           fin_path, (long) original_header->num_samples);
    printf("Output file:\t%s\tOutput number of samples:\t%ld\n",
           fout_path, (long) new_header->num_samples);
#endif

    /*
     * Writes header & data.
     */
    if (write_data(new_header, fout_path, kTotalHeaderSize, FALSE) !=
        SUCCESS)
        goto error;
    if (write_data
        (new_header->content, fout_path, new_header->subchunk2_size,
         TRUE) != SUCCESS)
        goto error;
    /*
     * Clean-up
     */
    FREEMEM(original_header->content);
    FREEMEM(original_header);
    FREEMEM(new_header);
    return SUCCESS;

  error:
    FREEMEM(original_header->content);
    FREEMEM(original_header);
    FREEMEM(new_header);
    return FAILURE;
}

/*
 * Joins two WAVE files into one.
 */
Status
join(const char *first_fin_path, const char *second_fin_path,
     const char *fout_path)
{

    struct wav_header *first_fin_header = NULL,
        *second_fin_header = NULL,
        *fout_header = NULL;

    /*
     * Read headers & data.
     */
    first_fin_header = read_data(first_fin_path);
    check_ptr(first_fin_header);

    second_fin_header = read_data(second_fin_path);
    check_ptr(second_fin_header);

    fout_header = join_header(first_fin_header, second_fin_header);
    check_ptr(fout_header);

#ifdef __DEBUG__
    printf("Original file:\t%s\tOriginal number of samples:\t%ld\n",
           first_fin_path, (long) first_fin_header->num_samples);
    printf("Original file:\t%s\tOriginal number of samples:\t%ld\n",
           second_fin_path, (long) second_fin_header->num_samples);
    printf("Output file:\t%s\tOutput number of samples:\t%ld\n",
           fout_path, (long) fout_header->num_samples);
#endif

    write_data(fout_header, fout_path, kTotalHeaderSize, FALSE);
    write_data(first_fin_header->content, fout_path,
               first_fin_header->subchunk2_size, TRUE);
    write_data(second_fin_header->content, fout_path,
               second_fin_header->subchunk2_size, TRUE);
    /*
     * Clean up
     */
    FREEMEM(first_fin_header->content);
    FREEMEM(first_fin_header);
    FREEMEM(second_fin_header->content);
    FREEMEM(second_fin_header);
    FREEMEM(fout_header);
    return SUCCESS;

  error:
    FREEMEM(first_fin_header->content);
    FREEMEM(first_fin_header);
    FREEMEM(second_fin_header->content);
    FREEMEM(second_fin_header);
    FREEMEM(fout_header);
    return FAILURE;
}

/*
 * Merges two WAVE files into one.
 * In order to be merged, the input files should
 * 1. have identical number of channels and bit-per-sample.
 * 2. has a bit-per-sample of 8 or 16(see the explanation later).
 */
Status
merge(const char *first_fin_path, const char *second_fin_path,
      const char *fout_path)
{
    struct wav_header *first_fin_header = NULL,
        *second_fin_header = NULL,
        *longer_fin_header = NULL,
        *shorter_fin_header = NULL;
    uint32_t        num_samples_to_merge;

    /*
     * Reads headers & data.
     */
    first_fin_header = read_data(first_fin_path);
    check_ptr(first_fin_header);

    second_fin_header = read_data(second_fin_path);
    check_ptr(second_fin_header);

    /*
     * Checks if input files are compatible.
     */
    check(first_fin_header->bit_per_sample ==
          second_fin_header->bit_per_sample,
          "The input files have different bit per sample.");
    check(first_fin_header->num_channels ==
          second_fin_header->num_channels,
          "The input files have different numbers of channels.");

    /*
     * Makes longer_fin_header the fin header with more number of samples.
     * Makes shorter_fin_header the fin header with less number of samples.
     */
    if (first_fin_header->num_samples > second_fin_header->num_samples) {
        longer_fin_header = first_fin_header;
        shorter_fin_header = second_fin_header;
    } else {
        longer_fin_header = second_fin_header;
        shorter_fin_header = first_fin_header;
    }

    /*
     * Only needs to merge the number of samples that shorter_fin_header has.
     */
    num_samples_to_merge = shorter_fin_header->num_samples;

    /**************************************************************************
     * I know doing this will break the compatibility with compilers that only*
     * support C89. Yet, it is the only way that the least amount of confusion*
     * is introduced.                                                         *
     **************************************************************************/
    int             i;
    uint16_t        byte_per_sample =
        shorter_fin_header->bit_per_sample / 8;
    uint32_t        offset = 0;                   /* Offset in data */

    /**************************************************************************
     * ```8-bit samples are stored as unsigned bytes, ranging from 0 to 255.
     * 16-bit samples are stored as 2's-complement signed integers, ranging
     * from -32768 to 32767.'''
     * See: https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
     *
     * Apply the principle described in
     * http://www.vttoth.com/CMS/index.php/technical-notes/68
     * we can now merge the files.
     *************************************************************************/
    if (longer_fin_header->bit_per_sample == 8) {
            /*
             * Well, let's read `usample_a' as `unsigned sample a'.
             */
        uint8_t         usample_a,
                        usample_b,
                        uresult_sample;
        for (i = 0; i < num_samples_to_merge; i++) {
            offset = byte_per_sample * i;
            memcpy(&usample_a, longer_fin_header->content + offset,
                   byte_per_sample);
            memcpy(&usample_b, shorter_fin_header->content + offset,
                   byte_per_sample);
            uresult_sample = usample_a + usample_b -
                usample_a * usample_b / (uint8_t) pow(2, 8);
            memcpy(longer_fin_header->content + offset,
                   &uresult_sample, byte_per_sample);
        }
    } else if (longer_fin_header->bit_per_sample == 16) {
        int16_t         sample_a,
                        sample_b,
                        result_sample;
        for (i = 0; i < num_samples_to_merge; i++) {
            offset = byte_per_sample * i;
            memcpy(&sample_a, longer_fin_header->content + offset,
                   byte_per_sample);
            memcpy(&sample_b, shorter_fin_header->content + offset,
                   byte_per_sample);
            result_sample = sample_a + sample_b -
                sample_a * sample_b / (int16_t) pow(2, 16);
            memcpy(longer_fin_header->content + offset,
                   &result_sample, byte_per_sample);
        }
    } else {
        fputs("The `merge' operation on this type of WAVE file is not implemented", stderr);
        goto error;
    }

    write_data(longer_fin_header, fout_path, kTotalHeaderSize, FALSE);
    write_data(longer_fin_header->content, fout_path,
               longer_fin_header->subchunk2_size, TRUE);

    FREEMEM(first_fin_header->content);
    FREEMEM(first_fin_header);
    FREEMEM(second_fin_header->content);
    FREEMEM(second_fin_header);
    return SUCCESS;

  error:
    FREEMEM(first_fin_header->content);
    FREEMEM(first_fin_header);
    FREEMEM(second_fin_header->content);
    FREEMEM(second_fin_header);
    return FAILURE;
}

static struct wav_header *
trim_header(const struct wav_header *original_header,
            const uint32_t target_num_samples,
            const uint32_t begin_num_samples_to_skip)
{
    struct wav_header *new_header =
        (struct wav_header *) malloc(sizeof(*new_header));
    check_mem(new_header);

    memcpy(new_header, original_header, sizeof(*new_header));

    /*
     * Adjust the header
     */
    new_header->num_samples = target_num_samples;
    new_header->subchunk2_size =
        original_header->subchunk2_size / original_header->num_samples *
        target_num_samples;
    /*
     * Assume the total size of header is 44.
     */
    new_header->chunk_size = 36 + new_header->subchunk2_size;

#ifdef __DEBUG__
    new_header->length_in_second = length_in_second(new_header);
#endif

    new_header->content = original_header->content +
        skip(original_header, begin_num_samples_to_skip);

    return new_header;

  error:
    FREEMEM(new_header->content);
    FREEMEM(new_header);

    return NULL;
}

static struct wav_header *
join_header(const struct wav_header *first_header,
            const struct wav_header *second_header)
{
    struct wav_header *new_header =
        (struct wav_header *) malloc(sizeof(*new_header));
    check_mem(new_header);

    memcpy(new_header, first_header, sizeof(*new_header));

    /*
     * Adjust the headers.
     * Simply add up every proper field.
     */
    new_header->subchunk2_size =
        first_header->subchunk2_size + second_header->subchunk2_size;
     /*
     * Assume the total size of header is 44.
     */
    new_header->chunk_size = 36 + new_header->subchunk2_size;
    new_header->num_samples =
        first_header->num_samples + second_header->num_samples;

#ifdef __DEBUG__
    new_header->length_in_second = length_in_second(new_header);
#endif

    new_header->content = NULL;

    return new_header;

  error:
    FREEMEM(new_header);

    return NULL;
}

/*
 * A wrapper of I/O functions.
 */
static struct wav_header *
read_data(const char *fin_path)
{
    struct wav_header *header = NULL;
    FILE           *fin = NULL;

    header = (struct wav_header *) malloc(sizeof(*header));
    check_mem(header);

    fin = fopen(fin_path, "rb");
    /*
     * MUST check the return value.
     */
    check(fin != NULL, "Cannot open the input file: %s", fin_path);

    FREAD_CHECK(header, fin, kTotalHeaderSize);

    header->content = (char *) malloc(header->subchunk2_size);
    check_mem(header->content);

    FREAD_CHECK(header->content, fin, header->subchunk2_size);

    /*
     * Content is already copied to memory. No need to use the input file.
     */
    CLOSEFD(fin);

    check(rain_check(header) == TRUE, "Invalid header.");

    header->num_samples = num_samples(header);
    header->length_in_second = length_in_second(header);

    return header;

  error:
    CLOSEFD(fin);
    FREEMEM(header->content);
    FREEMEM(header);

    return NULL;
}

static          Status
write_data(const void *data, const char *fout_path,
           const uint64_t size, const Boolean append)
{
    FILE           *fout;

    if (append == TRUE)
        fout = fopen(fout_path, "ab");
    else
        fout = fopen(fout_path, "wb");

    check(fout != NULL, "Cannot open the output file: %s", fout_path);

    FWRITE_CHECK(data, fout, size);
    CLOSEFD(fout);

    return SUCCESS;

  error:
    CLOSEFD(fout);
    fprintf(stderr, "Cannot write data to %s\n", fout_path);
    return FAILURE;
}

static          Boolean
rain_check(const struct wav_header *header)
{
#define checkId(A, M) check((strncmp((char *) &(A), (M), 4) == 0), M);
    checkId(header->chunk_id, "RIFF");
    checkId(header->format, "WAVE");
    checkId(header->subchunk1_id, "fmt ");
    checkId(header->subchunk2_id, "data");
#undef checkId

    return TRUE;

  error:
    fputs(" ********************** ERROR ***********************\n"
          " YOU are screwed. Either you supply an invalid audio \n"
          " or you run this program on a wrong machine. Please  \n"
          " check your input or/and read README.md\n"
          " ****************************************************\n",
          stderr);
    return FALSE;
}

static inline   uint32_t
num_samples(const struct wav_header *header)
{
    return header->subchunk2_size / header->num_channels /
        header->bit_per_sample * 8;
}

static inline float
length_in_second(const struct wav_header *header)
{
    return (float) header->num_samples / header->num_channels /
        header->sample_rate;
}

static inline   uint32_t
skip(const struct wav_header *header, const uint32_t num_samples_to_skip)
{
    return num_samples_to_skip * header->num_channels *
        header->bit_per_sample / 8;
}
