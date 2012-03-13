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

void Trim();
void Merge(const char *first_fin_path, const char *second_fin_path,
           const char *fout_path);

/*
 * This program will trim canonical WAVE files
 */
int main(int argc, char **argv)
{
        ParseArgumentsOrDie(argc, argv);

        Trim();
        Merge(fin_path, fin_path, "12.wav");

        return EXIT_SUCCESS;
}

/*
 * Trims the specified WAV file.
 */
void Trim(void)
{
        Data *data;
        WavHeader *ptr_original_header, *ptr_new_header;
        uint32_t fout_num_samples;

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

        free(data->content);
        free(data);
        free(ptr_original_header);
        free(ptr_new_header);
        return;

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

/*
 * Merges two WAV files into one.
 */
void Merge(const char *first_fin_path, const char *second_fin_path,
           const char *fout_path)
{
        Data *first_fin_data, *second_fin_data;
        WavHeader *first_fin_header, *second_fin_header, *fout_header;

        first_fin_data = CopyDataFromFileOrDie(first_fin_path);
        second_fin_data = CopyDataFromFileOrDie(second_fin_path);

        first_fin_header = InitialHeader(first_fin_data->content);
        second_fin_header = InitialHeader(second_fin_data->content);

        fout_header =
            ConstructMergedHeader(first_fin_header, second_fin_header);
        SetData(first_fin_data, (void *)&(fout_header->chunk_size),
                kChunkSizeSize, kChunkSizeOffset);
        SetData(first_fin_data, (void *)&(fout_header->subchunk2_size),
                kSubchunk2SizeSize, kSubchunk2SizeOffset);

        WriteDataOrDie(first_fin_data, fout_path,
                       first_fin_header->chunk_size + 8, 0);
        WriteDataOrDie(second_fin_data, fout_path,
                       second_fin_header->subchunk2_size, 1);

        /* Clean up */
        free(first_fin_data->content);
        free(first_fin_data);
        free(second_fin_data->content);
        free(second_fin_data);

        free(first_fin_header);
        free(second_fin_header);
        free(fout_header);
}
