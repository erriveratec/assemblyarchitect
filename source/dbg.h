#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(...) do { } while (0)
#else
#define debug(...) do { \
        fprintf(stderr, "DEBUG %s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); \
} while (0)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(...) do { \
    fprintf(stderr, "[ERROR] (%s:%d: errno: %s) ", \
            __FILE__, __LINE__, clean_errno()); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); \
} while (0)

#define log_warn(...) do { \
    fprintf(stderr, "[WARN] (%s:%d: errno: %s) ", \
            __FILE__, __LINE__, clean_errno()); \
    fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); \
} while (0)

#define log_info(...) do { \
        fprintf(stderr, "[INFO] (%s:%d) ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); \
} while (0)

#define check(A, ...) do { if (!(A)) { \
        log_err(__VA_ARGS__); errno=0; goto error; } } while (0)

#define sentinel(...) do { log_err(__VA_ARGS__); \
        errno=0; goto error; } while (0)

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, ...) do { if (!(A)) { debug(__VA_ARGS__); \
        errno=0; goto error; } } while (0)

#endif
