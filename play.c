#include "wavplay.h"
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <signal.h>

static void
Drop(int sig)
{
#ifdef SIGQUIT
    if (sig == SIGQUIT)
        snd_drop();
#endif
}

int
Play(const char *file_name)
{
    if (snd_init() < 0) {
        fprintf(stderr, "Failed to open device `" DEV_NAME "'\n");
        return 1;
    }
#ifdef SIGQUIT
    signal(SIGQUIT, Drop);
#endif
    wav_play(file_name);
    snd_end();
    return 0;
}
