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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

#define __DEBUG__

extern char    *fin_path;
extern char    *fout_path;
extern int      begin_flag;
extern int      end_flag;
extern uint32_t target_num_samples;

/*
 * This program will trim canonical WAVE files
 */
int
main(int argc, char **argv) {
    FILE           *fin, *fout;
    int64_t         fin_size;
    char           *buffer;
    size_t          result;
    uint32_t        fout_num_samples;
    WavHeader      *ptr_original_header, *ptr_new_header;

    ParseArgumentsOrDie(argc, argv);

    fin = fopen(fin_path, "rb");
    check(fin != NULL, "Cannot open the input file");

    /*
     * Get the total size of the file. 
     */
    fseek(fin, 0, SEEK_END);
    fin_size = ftell(fin);
    rewind(fin);

    buffer = (char *) malloc(fin_size);
    check_mem(buffer);

    result = fread(buffer, 1, fin_size, fin);
    check(result == fin_size, "Cannot read the entire file");
    /*
     * Content is already copied to memory. No need to use the input file. 
     */
    fclose(fin);

    ptr_original_header = (WavHeader *) malloc(sizeof(WavHeader));
    check_mem(ptr_original_header);

    InitialHeader(buffer, ptr_original_header);

    check(target_num_samples <= ptr_original_header->num_samples,
          "The specified number of samples is not legal.");

    if (begin_flag) {
        fout_num_samples = target_num_samples;
    } else if (end_flag) {
        fout_num_samples =
            ptr_original_header->num_samples - target_num_samples;
    } else {
        goto error;
    }


    ptr_new_header =
        ConstructHeader(ptr_original_header, fout_num_samples);

    /*
     * Adjust header 
     */
    memcpy(buffer + kChunkSizeOffset, &(ptr_new_header->chunk_size), 4);
    memcpy(buffer + kSubchunk2SizeOffset,
           &(ptr_new_header->subchunk2_size), 4);

    fout = fopen(fout_path, "wb");
    check(fout != NULL, "Cannot open the output file.");

    result = fwrite(buffer, 1, 44 + ptr_new_header->subchunk2_size, fout);
    check(result == 44 + ptr_new_header->subchunk2_size,
          "Cannot write the file.");

    free(buffer);
    free(ptr_original_header);
    free(ptr_new_header);
    fclose(fout);

#ifdef __DEBUG__
    printf("Original Number of Samples:\t%llu\n",
           ptr_original_header->num_samples);
    printf("Original Length in Second:\t%u\n",
           ptr_original_header->length_in_second);
    printf("Current Number of Samples:\t%d\n", fout_num_samples);
    printf("Current Length in Second:\t%u\n",
           ptr_new_header->length_in_second);
#endif
    return EXIT_SUCCESS;

  error:
    return EXIT_FAILURE;
}
