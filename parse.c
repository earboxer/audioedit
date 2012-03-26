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
#include "parse.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
/*
 * Global variables
 */
char           *fin_path[MAX_NUM_INPUTFILES] = { NULL };

char           *fout_path = NULL;

int             begin_flag = 0;

int             end_flag = 0;

uint32_t        begin_num_samples_to_trim = 0;
uint32_t        end_num_samples_to_trim = 0;

int             trim_flag = 0;

int             merge_flag = 0;

/*
 * Function prototypes
 */
void            usage(void);


/*
 * Parses the command line arguments
 */
void
ParseArgumentsOrDie(int argc, char *argv[])
{
    int             ch;
    char          **fin = NULL;

    while ((ch = getopt(argc, argv, "io:b:e:tmhv")) != -1) {
        switch (ch) {
        case 'i':
            for (fin = fin_path; optind < argc && argv[optind][0] != '-';
                 fin++, optind++) {
                *fin = argv[optind];
            }
            break;
        case 'o':
            fout_path = optarg;
            break;
        case 'b':
            if (argv[optind - 2][0] == '-' && argv[optind - 2][1] == 't') {
                trim_flag = 1;
                begin_num_samples_to_trim =
                    (uint32_t) strtol(optarg, (char **) NULL, 10);
                break;
            }
            goto error;
        case 'e':
            if (argv[optind - 2][0] == '-' && argv[optind - 2][1] == 't') {
                trim_flag = 1;
                end_num_samples_to_trim =
                    (uint32_t) strtol(optarg, (char **) NULL, 10);
                break;
            }
            goto error;
        case 'm':
            merge_flag = 1;
            break;
        case 't':
            /*
             * Do nothing 
             */
            break;
        case 'v':
            puts(VERSION);
#ifdef __clang__
            printf("Built at %s on %s, using clang %d.%d\n", __TIME__,
                   __DATE__, __clang_major__, __clang_minor__);
#elif __GNUC__
            printf("Built at %s on %s, using GCC %d.%d\n", __TIME__,
                   __DATE__, __GNUC__, __GNUC_MINOR__);
#endif
            exit(EXIT_SUCCESS);
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case '?':
            // Fall through
        default:
            goto error;
        }
    }
    argv += optind;

    check((merge_flag ^ trim_flag) == 1,
          "You can only specify merge or trim");
    if (trim_flag) {
        check(begin_num_samples_to_trim != 0
              || end_num_samples_to_trim != 0,
              "You must specify either -b or -e, or both.");
    }
    check(fout_path != NULL
          && fin_path[0] != NULL, "No path is specified.");
    return;

  error:
    usage();
    exit(EXIT_FAILURE);
}


/*
 * Prints out usage information. 
 */
void
usage(void)
{
    fputs("OPTIONS\n"
          "    -h              display the command line options\n"
          "    -v              display the version number\n"
          "    -tb n           trim n samples from the beginning for the audio clip\n"
          "    -te m           trim m samples off the end of the audio clip\n"
          "    -a              append the input files."
          "    -i file [,...]  provide the input file name\n"
          "    -o file         provide the output file name(overwriting an existing file)\n",
          stderr);
}
