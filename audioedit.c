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

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"
#include "file.h"
#include "parse.h"
#include "play.h"
#include "utils.h"

extern char    *fin_path[MAX_NUM_INPUTFILES];
extern char    *fout_path;
extern int      trim_flag;
extern int      join_flag;
extern int      play_flag;
extern int      merge_flag;
extern uint32_t begin_num_samples_to_trim;
extern uint32_t end_num_samples_to_trim;
extern char    *fplay_path;

/*
 * This program will edit canonical WAVE files according to command line
 * arguments by calling corresponding functions.
 * Sub-routines will return a value indicating the status of operation.
 */
int
main(int argc, char **argv)
{
    check(parse_cmd(argc, argv) == SUCCESS,
          "Cannot parse command line...");

    if (play_flag) {
        check(play(fplay_path) == SUCCESS, "Failed to play.");
    } if (join_flag) {
        check(join(fin_path[0], fin_path[1], fout_path) == SUCCESS,
              "Failed to join");
    } if (merge_flag) {
        check(merge(fin_path[0], fin_path[1], fout_path) == SUCCESS,
              "Failed to merge.");
    } if (trim_flag) {
        check(trim(fin_path[0], begin_num_samples_to_trim,
                   end_num_samples_to_trim, fout_path) == SUCCESS,
              "Failed to trim");
    }

    return EXIT_SUCCESS;

  error:
    return EXIT_FAILURE;
}
