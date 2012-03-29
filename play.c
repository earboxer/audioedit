/*-
 * wavplay - a C library to play WAV sound via OSS/ALSA
 *
 * Copyright (c) 2011 Zhihao Yuan.
 * Copyright (c) 2012 Meitian Huang.
 * All rights reserved.
 *
 * This file is distributed under the 2-clause BSD License.
 */

#include <signal.h>

#include "wavplay.h"

static void
drop(int sig)
{
#ifdef SIGQUIT
    if (sig == SIGQUIT)
        snd_drop();
#endif
}

int
play(const char *file_name)
{
    if (snd_init() < 0) {
        fprintf(stderr, "Failed to open device `" DEV_NAME "'\n");
        return 1;
    }
#ifdef SIGQUIT
    signal(SIGQUIT, drop);
#endif
    wav_play(file_name);
    snd_end();
    return 0;
}
