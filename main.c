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
#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

extern char *fin_path;
extern char *fout_path;
extern int begin_flag;
extern int end_flag;
extern uint32_t target_num_samples;
extern int duplicate_flag;

#define __DEBUG__

/*
 * This program will trim canonical WAVE files
 */
int main(int argc, char **argv)
{
        uint32_t fout_num_samples;
        WavHeader *ptr_original_header, *ptr_new_header;
        Data *data;

        ParseArgumentsOrDie(argc, argv);

        data = CopyDataFromFileOrDie(fin_path);

        ptr_original_header = InitialHeader(data->content);

        check(target_num_samples <= ptr_original_header->num_samples,
              "The specified number \"%ld\" of samples is not legal.",
              (long)target_num_samples);

        if (begin_flag) {
                fout_num_samples = target_num_samples;
        } else if (end_flag) {
                fout_num_samples =
                    ptr_original_header->num_samples - target_num_samples;
        } else {
                goto error;
        }

        ptr_new_header =
            ConstructTrimedHeader(ptr_original_header, fout_num_samples);

        /*
         * Adjust header
         */
        SetData(data, (void *)&(ptr_new_header->chunk_size), kChunkSizeSize,
                kChunkSizeOffset);
        SetData(data, (void *)&(ptr_new_header->subchunk2_size),
                kSubchunk2SizeSize, kSubchunk2SizeOffset);

        WriteDataOrDie(data, fout_path,
                       kTotalHeaderSize + ptr_new_header->subchunk2_size, 0);

        WavHeader *test_header =
            ConstructMergedHeader(ptr_original_header, ptr_original_header);
        SetData(data, (void *)&(test_header->chunk_size), kChunkSizeSize,
                kChunkSizeOffset);
        SetData(data, (void *)&(test_header->subchunk2_size),
                kSubchunk2SizeSize, kSubchunk2SizeOffset);

        WriteDataOrDie(data, "test2.wav", ptr_original_header->chunk_size + 8,
                       0);
        WriteDataOrDie(data, "test2.wav", ptr_original_header->subchunk2_size,
                       1);

#ifdef __DEBUG__
        printf("Original Number of Samples:\t%llu\n",
               ptr_original_header->num_samples);
        printf("Original Length in Second:\t%.1f\n",
               ptr_original_header->length_in_second);
        printf("Current Number of Samples:\t%d\n", fout_num_samples);
        printf("Current Length in Second:\t%.1f\n",
               ptr_new_header->length_in_second);
#endif

        /*
         * Cleaning up
         */
        free(data->content);
        free(data);
        free(ptr_original_header);
        free(ptr_new_header);

        return EXIT_SUCCESS;

 error:
        if (data->content)
                free(data->content);
        if (data)
                free(data);
        if (ptr_original_header)
                free(ptr_original_header);
        if (ptr_new_header)
                free(ptr_new_header);
        abort();
}

void Trim(void) {

}
