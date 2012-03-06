/*-
 * Copyright (c) 2012 Meitian Huang
 * All rights reserverd.
 *
 * Distributed under FreeBSD License.
 */
#ifndef DBG_H_
#define DBG_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>

// Debug related


#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M)  fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", \
        __FILE__, __LINE__, clean_errno())

#define log_warn(M) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", \
        __FILE__, __LINE__, clean_errno())

#define log_info(M) fprintf(stderr, "[INFO] (%s:%d) " M "\n", \
        __FILE__, __LINE__)

#define check(A, M)     if (!(A)) { log_err(M); errno = 0; goto error; }

#define sentinel(M)     { log_err(M); errno = 0; goto error; }

#define check_mem(A)    check((A), "Out of memory.")

#define check_debug(A, M)   if (!(A)) { debug(M); errno=0; goto error; }

#endif  // DBG_H_
