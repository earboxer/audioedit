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
