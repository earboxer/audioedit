/*-
 * Copyright (c) 2012 Meitian Huang
 * All rights reserverd.
 *
 * Distributed under FreeBSD License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include "parse.h"
#include "dbg.h"
#define __DEBUG__

extern char *in;
extern char *out;
extern int bflag;
extern int eflag;
extern int bSamples;
extern int eSamples;

unsigned int
calcSample(char *c){
    unsigned int subchunk2Size = 0;
    memcpy(&subchunk2Size, c + 40, 4);

    unsigned int bitsPerSample = 0;
    memcpy(&bitsPerSample, c + 34, 2);

    unsigned int numChannels = 0;
    memcpy(&numChannels, c + 22, 2);

    return subchunk2Size * 8 / bitsPerSample / numChannels;
}


    int
main(int argc, char **argv)
{
    parse(argc, argv);
    if (in == NULL || out == NULL){
        fputs("You must say something...", stderr);
        abort();
    }

    FILE *fin;
    FILE *fout;
    long origSize;
    char *buffer;
    size_t result;
    unsigned int origNumberSamples;
    unsigned int newNumberSamples;
    unsigned int newSubChunck2Size;


    fin = fopen(in, "rb");
    check(fin != NULL, "Cannot open the input file");

    // Get the total size of the file.
    fseek(fin, 0, SEEK_END);
    origSize = ftell(fin);
    rewind(fin);

    // Allocate enough memory to store the file.
    buffer = (char *) malloc(sizeof(char) * origSize);
    check_mem(buffer);

    // Read the input file.
    result = fread(buffer, 1, origSize, fin);
    check(result == origSize, "Cannot read the entire file");
    fclose(fin);    // Content is already copied to memory.
    // No need to use the input file.

    // Calculate the original number of samples.
    origNumberSamples = calcSample(buffer);

    if (bSamples > origNumberSamples || eSamples > origNumberSamples){
        fputs("The specified number of samples is not legal.", stderr);
        goto error;
    }

    int startAddr;

    if (bflag) {
        startAddr = 0;
        newNumberSamples = bSamples;
    } else if (eflag) {
        startAddr = origNumberSamples - eSamples;
        newNumberSamples = eSamples;
    } else {
        goto error;
    }
    // Calculate the number of samples we want to get.

    printf("Original Number of Samples:\t%d\n", origNumberSamples);
    printf("Current Number of Samples:\t%d\n", newNumberSamples);

    // Retrieve the original subchunk2size.
    unsigned int subchunk2Size = 0;
    memcpy(&subchunk2Size, buffer + 40, 4);

    // New subchunk2size.
    newSubChunck2Size 
        = subchunk2Size / origNumberSamples * newNumberSamples;

    // Calculate the adjustments.
    unsigned int newChunkSize = 36 + newSubChunck2Size;
    unsigned int newLength = 8 + newChunkSize;

    // Adjust Chunksize.
    memcpy(buffer + 4, &newChunkSize, 4);
    /* Adjust Subchunk2Size */
    memcpy(buffer + 40, &newSubChunck2Size, 4);

    // Open the file to write
    fout = fopen(out, "wb");
    check(fout != NULL, "Cannot open the output file.");

    // Write
    result = fwrite(buffer + startAddr, 1, newLength, fout);
    check(result == newLength, "Cannot write to the file.");

    free(buffer);
    fclose(fout);

    return 0;

error:
    abort();
}
