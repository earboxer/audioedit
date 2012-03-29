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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef SUCCESS
#undef SUCCESS
#endif

#ifdef FAILURE
#undef FAILURE
#endif

typedef enum { SUCCESS, FAILURE } Status;

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

typedef enum { FALSE, TRUE } Boolean;

/*
 * Frequently used functions.
 */
#define FREEMEM(A) do {                        \
        if (A) {                                \
                free(A);                        \
                A = NULL;                       \
        }                                       \
} while (0)

#define CLOSEFD(A) do {                        \
        if (A) {                                \
                fclose(A);                      \
                A = NULL;                       \
        }                                       \
} while (0)

#define FREAD_CHECK(D, S, C) do {               \
        check(fread(D, C, 1, S) == 1,           \
              "Cannot read the input file.");   \
} while (0)

#define FWRITE_CHECK(D, S, C) do {              \
        check(fwrite(D, C, 1, S) == 1,          \
              "Cannot write the input file");   \
} while (0)

#define check_ptr(A) do {                       \
        check((A), "Invalid pointer received.");\
} while (0)

#define min(m,n)    ((m) < (n) ? (m) : (n))

#endif                          /* UTILS_H_ */
