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

unsigned int
calcSample(FILE *f){
    unsigned int result;

    unsigned int subchunk2Size = 0;
    fseek(f, 40, SEEK_SET);
    fread(&subchunk2Size, 1, 4, f);

    unsigned int bitsPerSample = 0;
    fseek(f, 34, SEEK_SET);
    fread(&bitsPerSample, 1, 2, f);

    unsigned int numChannels = 0;
    fseek(f, 22, SEEK_SET);
    fread(&numChannels, 1, 2, f);

    result = subchunk2Size * 8 / bitsPerSample / numChannels;

    return result;
}


    int
main(int argc, char **argv)
{
    parse(argc, argv);
    if (in == NULL || out == NULL)
    {
        puts("You must say something...");
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

    // Calculate the original number of samples.
    origNumberSamples = calcSample(fin);

    // Calculate the number of samples we want to get. 
    newNumberSamples = origNumberSamples / 7 * 4;

    printf("Original Number of Samples:\t%d\n", origNumberSamples);
    printf("Current Number of Samples:\t%d\n", newNumberSamples);

    // Retrieve the original subchunk2size. 
    unsigned int subchunk2Size = 0;
    fseek(fin, 40, SEEK_SET);
    result = fread(&subchunk2Size, 1, 4, fin);
    check(result == 4, "Cannot read the file.");

    // New subchunk2size. 
    newSubChunck2Size = subchunk2Size / origNumberSamples * newNumberSamples;

    // Calculate the adjustments. 
    unsigned int newChunkSize = 36 + newSubChunck2Size;
    unsigned int newLength = 8 + newChunkSize;

    // Write the information to disk.
    fout = fopen("test.wav", "wb");
    check(fout != NULL, "Cannot open the output file.");

    // Write without adjustment. 
    result = fwrite(buffer, 1, newLength, fout);
    check(result == newLength, "Cannot write to the file.");

    // Adjust ChunkSize. 
    fseek(fout, 4, SEEK_SET);
    result = fwrite(&newChunkSize, 1, sizeof(unsigned int), fout);
    check(result == sizeof(unsigned int), "Cannot write to the file.");

    // Adjust Subchunk2Size.
    fseek(fout, 40, SEEK_SET);
    result = fwrite(&newSubChunck2Size, 1, sizeof(unsigned int), fout);
    check(result == sizeof(unsigned int), "Cannot write to the file.");
    
    free(buffer);
    fclose(fin);
    fclose(fout);
    return 0;

error:
    abort();
}
