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

#ifndef FILE_H_
#define FILE_H_

#include <stdint.h>

#include "utils.h"

#define kTotalHeaderSize        44

/*
 * Why not typedef? Because it is evil.
 * For more information, see "Linux kernel coding style" or do a "man style" on
 * FreeBSD.
 */
struct wav_header {
    /*
     * Wave file header fields.
     * Total: 44 bytes.
     * These fields will be directly read from the file.
     */

    /******************** WARNING *************************
     * Memory alignment may cause huge problems. This     *
     * approach is s**t in terms of portability. I did    *
     * not notice this problem until I finished the code. *
     * My original design (read the field one by one) is  *
     * much robust. However, I presume this will NOT be a *
     * problem on MODERN computers.                       *
     ******************************************************/
    uint32_t        chunk_id;
    uint32_t        chunk_size;
    uint32_t        format;
    uint32_t        subchunk1_id;
    uint32_t        subchunk1_size;
    uint16_t        audio_format;
    uint16_t        num_channels;
    uint32_t        sample_rate;
    uint32_t        byte_rate;
    uint16_t        block_align;
    uint16_t        bit_per_sample;
    uint32_t        subchunk2_id;
    uint32_t        subchunk2_size;
    /*
     * Extra fields.
     * Because Subchunk2Size == NumSamples * NumChannels * BitPerSample / 8,
     * where NumChannels >= 1, BitPerSample >= 8.
     * Hence, Subchunk2Size >= NumSamples, which means uint32_t is enough for
     * the number of samples.
     */
    uint32_t        num_samples;
    float           length_in_second;
    char           *content;
} __attribute__ ((__packed__));

/*
 * Exported functions.
 */
Status          trim(const char *fin_path,
                     const uint32_t begin_num_samples_to_trim,
                     const uint32_t end_num_samples_to_trim,
                     const char *fout_path);
Status          join(const char *first_fin_path,
                     const char *second_fin_path, const char *fout_path);
Status          merge(const char *first_fin_path,
                      const char *second_fin_path, const char *fout_path);

#endif                          /* FILE_H_ */
