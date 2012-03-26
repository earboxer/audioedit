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

#include "parse.h"
#include "dbg.h"
#include "file.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

#define __DEBUG__

extern char    *fin_path[MAX_NUM_INPUTFILES];
extern char    *fout_path;
extern int      begin_flag;
extern int      end_flag;
extern uint32_t begin_num_samples_to_trim;
extern uint32_t end_num_samples_to_trim;
extern int      trim_flag;
extern int      merge_flag;

void            Trim(const char *fin_path);
void            Merge(const char *first_fin_path,
                      const char *second_fin_path, const char *fout_path);

/*
 * This program will trim canonical WAVE files
 */
int
main(int argc, char **argv)
{
    ParseArgumentsOrDie(argc, argv);

    if (merge_flag) {
        Merge(fin_path[0], fin_path[1], fout_path);
    } else if (trim_flag) {
        Trim(fin_path[0]);
    }

    return EXIT_SUCCESS;
}

/*
 * Trims the specified WAV file.
 */
void
Trim(const char *fin_path)
{
    WavHeader      *ptr_original_header = NULL,
        *ptr_new_header = NULL;
    uint64_t        fout_num_samples;

    ptr_original_header = CopyDataFromFileOrDie(fin_path);
    check_ptr(ptr_original_header);

    check(begin_num_samples_to_trim + end_num_samples_to_trim <=
          ptr_original_header->num_samples,
          "The specified number \"%ld\" of samples is not legal.",
          (long) begin_num_samples_to_trim + end_num_samples_to_trim);


    fout_num_samples =
        ptr_original_header->num_samples - begin_num_samples_to_trim -
        end_num_samples_to_trim;

    ptr_new_header =
        ConstructTrimedHeader(ptr_original_header, fout_num_samples);
    check_ptr(ptr_new_header);

    ptr_new_header->content +=
        (ptr_original_header->subchunk2_size -
         ptr_new_header->subchunk2_size) * begin_num_samples_to_trim /
        (begin_num_samples_to_trim + end_num_samples_to_trim);

#ifdef __DEBUG__
    printf("original subchunk2size: %ld\n",
           (long) ptr_original_header->subchunk2_size);
    printf("new subchunk2size: %ld\n",
           (long) ptr_new_header->subchunk2_size);
#endif

    WriteDataOrDie(ptr_new_header, fout_path, kTotalHeaderSize, 0);
    WriteDataOrDie(ptr_new_header->content, fout_path,
                   ptr_new_header->subchunk2_size, 1);

    /*
     * Clean-up
     */
    FREEMEM_(ptr_original_header->content);
    FREEMEM_(ptr_original_header);
    FREEMEM_(ptr_new_header);
    return;

  error:
    FREEMEM_(ptr_original_header->content);
    FREEMEM_(ptr_original_header);
    FREEMEM_(ptr_new_header);
    exit(EXIT_FAILURE);
}

/*
 * Merges two WAV files into one.
 */
void
Merge(const char *first_fin_path, const char *second_fin_path,
      const char *fout_path)
{

    WavHeader      *first_fin_header,
                   *second_fin_header,
                   *fout_header;

    first_fin_header = CopyDataFromFileOrDie(first_fin_path);
    check_ptr(first_fin_header);
    second_fin_header = CopyDataFromFileOrDie(second_fin_path);
    check_ptr(second_fin_header);

    fout_header =
        ConstructMergedHeader(first_fin_header, second_fin_header);
    check_ptr(fout_header);

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
    return;

  error:
    FREEMEM_(first_fin_header->content);
    FREEMEM_(first_fin_header);
    FREEMEM_(second_fin_header->content);
    FREEMEM_(second_fin_header);
    FREEMEM_(fout_header);
}
