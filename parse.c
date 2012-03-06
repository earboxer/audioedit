/*-
 * Copyright (c) 2012 Meitian Huang
 * All rights reserverd.
 *
 * Distributed under FreeBSD License.
 */
#include "dbg.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

char *in = NULL;
char *out = NULL;
int bflag = 0;
uint32_t bSamples = 0;
int eflag = 0;
uint32_t eSamples = 0;
int c;

void print_help() {
    printf("OPTIONS\n");
    printf("\t-help\t\tdisplay the command line options\n");
    printf("\t-version\tdisplay the version number\n");
    printf("\t-tb n\t\ttrim n samples from the beginning for the audio clip\n");
    printf("\t-te m\t\ttrim m samples off the end of the audio clip\n");
    printf("\t-i file\t\tprovide the input file name\n");
    printf("\t-o file\t\tprovide the output file name(overwriting an existing file)\n");
}

void parse(int argc, char **argv) {
    while ((c = getopt(argc, argv, "i:o:b:e:")) != -1) {
        switch (c) {
            case 'i':
                in = optarg;
                break;
            case 'o':
                out = optarg;
                break;
            case 'b':
                bflag = 1;
                bSamples = (int)strtol(optarg, (char **)NULL, 10);
                break;
            case 'e':
                eflag = 1;
                eSamples = (int)strtol(optarg, (char **)NULL, 10);
                break;
            case '?':
                print_help();
            default:
                goto error;
        }
    }
    check(!(bflag && eflag), "You cannot set both -b and -e");
    check(bflag || eflag, "You need to specify the new length");
    return;

error:
    abort();
}
