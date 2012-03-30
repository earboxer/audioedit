/*-
 * Copyright (c) 2012 Meitian Huang.
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
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT OWNER AND CONTRIBUTORS "AS IS"
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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "dbg.h"
#include "parse.h"
#include "utils.h"

/*
 * Global variables
 */

char           *fout_path = NULL;

Boolean         trim_flag = FALSE;

Boolean         join_flag = FALSE;

Boolean         play_flag = FALSE;

Boolean         merge_flag = FALSE;

uint32_t        begin_num_samples_to_trim = 0,
    end_num_samples_to_trim = 0;

char           *fin_path[2] = { NULL };

char           *fplay_path = NULL;

/*
 * Function prototypes
 */
static inline void usage(void);
static inline int num_options(void);
/*
 * Parses command line arguments.
 * Returns the number of options.
 */
Status
parse_cmd(int argc, char *argv[])
{
    int             ch;
    char          **fin = NULL;
    int             i = 0;

    while ((ch = getopt(argc, argv, "p:io:b:e:tjmhv")) != -1) {
        switch (ch) {
        case 'p':
            play_flag = TRUE;
            fplay_path = optarg;
            break;
        case 'i':
            /*
             * There may be many input files. getopt() cannot handle this
             * situation. Hence, the command line should be manually processed.
             * Put every separate token into the array until a token begins
             * with '-' is seen.
             */
            for (fin = fin_path;
                 optind < argc && argv[optind][0] != '-' && i <= 2;
                 fin++, optind++, i++) {
                *fin = argv[optind];
            }
            break;
        case 'o':
            fout_path = optarg;
            break;
        case 't':
            /*
             * Well, nothing is done here.
             * This option is kept in order to meet the specification.
             */
            break;
        case 'b':
            /*
             * Only '-tb' is acceptable.
             * Note: '-tb' will be recognised as '-t' plus '-b'. This code
             * exploit this feature.
             */
            if (argv[optind - 2][0] == '-' && argv[optind - 2][1] == 't') {
                trim_flag = TRUE;
                begin_num_samples_to_trim =
                    (uint32_t) strtol(optarg, (char **) NULL, 10);
                break;
            }
            goto error;
        case 'e':
            /*
             * Same as '-b'.
             */
            if (argv[optind - 2][0] == '-' && argv[optind - 2][1] == 't') {
                trim_flag = TRUE;
                end_num_samples_to_trim =
                    (uint32_t) strtol(optarg, (char **) NULL, 10);
                break;
            }
            goto error;
        case 'j':
            join_flag = TRUE;
            break;
        case 'm':
            merge_flag = TRUE;
            break;
        case 'v':
            /*
             * The specification requires '-version' as an option. Yet, it is
             * not practical in this situation as '-version' will be broken
             * into '-v', '-e', '-r', etc.
             */
            puts(VERSION);
            printf("Built at %s on %s", __TIME__, __DATE__);
#ifdef __clang__
            printf(", using clang %d.%d", __clang_major__,
                   __clang_minor__);
#elif __GNUC__
            printf(",using GCC %d.%d", __GNUC__, __GNUC_MINOR__);
#endif
            printf("\n");
            exit(EXIT_SUCCESS); /* Exits the program. */
        case 'h':
            usage();
            exit(EXIT_SUCCESS); /* Exits the program. */
        case '?':
            // Fall through
        default:
            goto error;
        }
    }

    check(num_options() == 1,
          "Please supply the right amount of options.");

    check(begin_num_samples_to_trim >= 0 && end_num_samples_to_trim >= 0,
          "Sorry, I don't know what to do with a negative number of samples");

    if (trim_flag == TRUE)
        check(fin_path[0] != NULL && fin_path[1] == NULL,
              "You need to provide exactly one input files.");
    if (merge_flag == TRUE || join_flag == TRUE)
        check(fin_path[0] != NULL && fin_path[1] != NULL,
              "You need to provide exactly two input files.");
    if (play_flag == TRUE)
        check(fplay_path != NULL, "What do you want to play?");

    return SUCCESS;

  error:
    usage();
    return FAILURE;
}

/*
 * Prints out usage information.
 */
static inline void
usage(void)
{
    fputs("OPTIONS\n"
          "    -h              display the command line options\n"
          "    -v              display the version number\n"
          "    -tb n           trim n samples from the beginning for the audio clip\n"
          "    -te m           trim m samples off the end of the audio clip\n"
          "    -j              join the input files."
          "    -m              merge the input files."
          "    -p FILE         play FILE."
          "    -i file [,...]  provide the input file name\n"
          "    -o file         provide the output file name(overwriting an existing file)\n",
          stderr);
}

/*
 * Counts the number of options in the command line arguments.
 */
static inline int
num_options(void)
{
    int             count = 0;
    if (trim_flag == TRUE)
        count++;
    if (play_flag == TRUE)
        count++;
    if (join_flag == TRUE)
        count++;
    if (merge_flag == TRUE)
        count++;
    return count;
}
