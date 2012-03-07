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
#include "parse.h"
#include "dbg.h"

extern char *in;
extern char *out;
extern int bflag;
extern int eflag;
extern uint32_t bSamples;
extern uint32_t eSamples;

uint32_t calcSample(char *c) {
    uint32_t subchunk2Size;
    memcpy(&subchunk2Size, c + 40, 4);

    uint16_t bitsPerSample;
    memcpy(&bitsPerSample, c + 34, 2);

    uint16_t numChannels;
    memcpy(&numChannels, c + 22, 2);

    return subchunk2Size * 8 / bitsPerSample / numChannels;
}

/*
 * This program will trim canonical WAVE files
 */
int main(int argc, char **argv) {
    parse(argc, argv);
    if (in == NULL || out == NULL) {
        fputs("You must say something...\n", stderr);
        abort();
    }

    FILE *fin;
    FILE *fout;
    int64_t origSize;
    char *buffer;
    size_t result;
    uint32_t origNumberSamples;
    uint32_t newNumberSamples;
    uint32_t newSubChunck2Size;


    fin = fopen(in, "rb");
    check(fin != NULL, "Cannot open the input file");

    // Get the total size of the file.
    fseek(fin, 0, SEEK_END);
    origSize = ftell(fin);
    rewind(fin);

    // Allocate enough memory to store the file.
    buffer = (char *) malloc(origSize);
    check_mem(buffer);

    // Read the input file.
    result = fread(buffer, 1, origSize, fin);
    check(result == origSize, "Cannot read the entire file");
    fclose(fin);    // Content is already copied to memory.
    // No need to use the input file.

    // Calculate the original number of samples.
    origNumberSamples = calcSample(buffer);

    if (bSamples > origNumberSamples || eSamples > origNumberSamples) {
        fputs("The specified number of samples is not legal.\n", stderr);
        goto error;
    }


    if (bflag) {
        newNumberSamples = bSamples;
    } else if (eflag) {
        newNumberSamples = origNumberSamples - eSamples;
    } else {
        goto error;
    }
    // Calculate the number of samples we want to get.

#ifdef __DEBUG__
    printf("Original Number of Samples:\t%d\n", origNumberSamples);
    printf("Current Number of Samples:\t%d\n", newNumberSamples);
#endif

    // Retrieve the original subchunk2size.
    uint32_t subchunk2Size;
    memcpy(&subchunk2Size, buffer + 40, 4);

    // New subchunk2size.
    newSubChunck2Size = subchunk2Size / origNumberSamples * newNumberSamples;

    // Calculate the adjustments.
    uint32_t newChunkSize = 36 + newSubChunck2Size;

    // Adjust Chunksize.
    memcpy(buffer + 4, &newChunkSize, 4);
    // Adjust Subchunk2Size
    memcpy(buffer + 40, &newSubChunck2Size, 4);

    // Open the file to write
    fout = fopen(out, "wb");
    check(fout != NULL, "Cannot open the output file.");

    // Write
    result = fwrite(buffer, 1, 44 + newSubChunck2Size, fout);

    check(result == 44 + newSubChunck2Size, "Cannot write to the file.");

    free(buffer);
    fclose(fout);

    return 0;

error:
    abort();
}
