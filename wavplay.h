#ifndef __APPLE__
#ifndef _WAVPLAY_H
#define _WAVPLAY_H

/*-
 * wavplay - a C library to play WAV sound via OSS/ALSA
 *
 * Copyright (c) 2011 Zhihao Yuan.
 * All rights reserved.
 *
 * Reference:
 * http://www.freshports.org/audio/waveplay/
 *
 * This file is distributed under the 2-clause BSD License.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if !defined(USE_ALSA)
#define DEV_NAME "/dev/dsp"
#include <fcntl.h>
#include <sys/ioctl.h>
#if defined(__NetBSD__) || defined(__OpenBSD__)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#else
#define DEV_NAME "default"
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#endif

typedef struct _riffchunk {
    char            id[4];
    uint32_t        size;
} riffchunk_t;

typedef struct _wavheader {
    int16_t         format;
    int16_t         nchannels;
    int32_t         framerate;
    int32_t         byterate;
    int16_t         blocksize;
    int16_t         bitdepth;
} wavheader_t;

typedef struct _extdouble {
    int16_t         expon;
    uint32_t        himant;
    uint32_t        lomant;
} __attribute__ ((__packed__)) extdouble_t;

typedef struct _aifheader {
    int16_t         nchannels;
    int32_t         nframes;
    int16_t         bitdepth;
    extdouble_t     framerate;
    char            comptype[4];
} __attribute__ ((__packed__)) aifheader_t;

typedef struct _sunheader {
    uint32_t        size;
    int32_t         encoding;
    int32_t         framerate;
    int32_t         nchannels;
} __attribute__ ((__packed__)) sunheader_t;

typedef struct _wav_info {
    short           nchannels;
    int             nframes;
    short           sampwidth;
    int             framerate;
    int             devformat;
} wav_info_t;

#ifdef __cplusplus
extern          "C" {
#endif

    int             wav_play(const char *filename);
    int             wav_send(FILE * stream);

    int             snd_init(void);
    int             snd_set(int format, /* OSS/ALSA format constant */
                            int nchannels, int framerate);
    int             snd_send(FILE * stream, size_t size);
    int             snd_drop(void);
    int             snd_end(void);

#ifdef __cplusplus
}
#endif
/*
 * Reference: FreeBSD /usr/include/sys/endian.h 
 */
#ifndef bswap16
#define _bswap64(x) \
__extension__ ({ register __uint64_t __X = (x); \
   __asm ("bswap %0" : "+r" (__X)); \
   __X; })
#define _bswap32(x) \
__extension__ ({ register __uint32_t __X = (x); \
   __asm ("bswap %0" : "+r" (__X)); \
   __X; })
static __inline __uint64_t
bswap64(__uint64_t _x)
{
    return (_bswap64(_x));
}

static __inline __uint32_t
bswap32(__uint32_t _x)
{
    return (_bswap32(_x));
}

static __inline __uint16_t
bswap16(__uint16_t _x)
{
    return (_x << 8 | _x >> 8);
}

#endif

#endif /* _WAVPLAY_h */

#endif
