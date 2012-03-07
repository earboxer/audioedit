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

char           *fin_path = NULL, *fout_path = NULL;

int             begin_flag = 0;

int             end_flag = 0;

uint32_t        target_num_samples = 0;

void            usage(void);

void
ParseArgumentsOrDie(int argc, char **argv) {
    int             c;

    while ((c = getopt(argc, argv, "i:o:b:e:")) != -1) {
        switch (c) {
        case 'i':
            fin_path = optarg;
            break;
        case 'o':
            fout_path = optarg;
            break;
        case 'b':
            begin_flag = 1;
            target_num_samples = (int) strtol(optarg, (char **)
                                              NULL, 10);
            break;
        case 'e':
            end_flag = 1;
            target_num_samples = (int) strtol(optarg, (char **)
                                              NULL, 10);
            break;
        case '?':
            usage();
        default:
            goto error;
        }
    }
    argc -= optind;
    argv += optind;

    check((begin_flag ^ end_flag) == 1, "-b is incompitible with -e.");
    check(fout_path != NULL && fin_path != NULL, "No path is specified.");
    return;

  error:
    fputs("The input is ill-formed. The program is now exiting.\n",
          stderr);
    exit(EXIT_FAILURE);
}

void
usage(void) {
    fputs("OPTIONS\n"
          "    -help     "
          "display the command line options\n"
          "    -version  "
          "display the version number\n"
          "    -tb n     "
          "trim n samples from the beginning for the audio clip\n"
          "    -te m     "
          "trim m samples off the end of the audio clip\n"
          "    -i file   "
          "provide the input file name\n"
          "    -o file   "
          "provide the output file name(overwriting an existing file)\n",
          stderr);
}
