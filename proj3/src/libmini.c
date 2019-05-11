#include "libmini.h"

#define COMB(N, ...) COMB##N(__VA_ARGS__)
#define COMB0()
#define COMB1(TYPE, NAME) TYPE NAME
#define COMB2(TYPE, NAME, ...) TYPE NAME, COMB1(__VA_ARGS__)
#define COMB3(TYPE, NAME, ...) TYPE NAME, COMB2(__VA_ARGS__)
#define COMB4(TYPE, NAME, ...) TYPE NAME, COMB3(__VA_ARGS__)
#define COMB5(TYPE, NAME, ...) TYPE NAME, COMB4(__VA_ARGS__)
#define COMB6(TYPE, NAME, ...) TYPE NAME, COMB5(__VA_ARGS__)

#define SEP(N, ...) SEP##N(__VA_ARGS__)
#define SEP0()
#define SEP1(TYPE, NAME, ...) NAME
#define SEP2(TYPE, NAME, ...) NAME, SEP1(__VA_ARGS__)
#define SEP3(TYPE, NAME, ...) NAME, SEP2(__VA_ARGS__)
#define SEP4(TYPE, NAME, ...) NAME, SEP3(__VA_ARGS__)
#define SEP5(TYPE, NAME, ...) NAME, SEP4(__VA_ARGS__)
#define SEP6(TYPE, NAME, ...) NAME, SEP5(__VA_ARGS__)

#define WRAPPER(RET_TYPE, NAME, RET_VALUE, NUM, ...) \
    RET_TYPE NAME(COMB(NUM, __VA_ARGS__)) { \
        long ret = sys_##NAME(SEP(NUM, __VA_ARGS__)); \
\
        errno = 0; \
\
        if (ret < 0) { \
            errno = -ret; \
            return RET_VALUE; \
        } \
        return ((RET_TYPE) ret); \
    }

#define LIST(...) __VA_ARGS__


long errno;


WRAPPER(ssize_t, write, -1, 3, int, fd, const void*, buf, size_t, count)
WRAPPER(int, pause, -1, 0)
WRAPPER(unsigned int, alarm, -1, 1, unsigned int, secondes)

void exit(int error_code) {
    sys_exit(error_code);
}

size_t strlen(const char *s) {
    size_t count = 0;
    while (*s++) count++;
    return count;
}

static const char *errmsg[] = {
    "Success",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "I/O error",
    "No such device or address",
    "Argument list too long",
    "Exec format error",
    "Bad file number",
    "No child processes",
    "Try again",
    "Out of memory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Device or resource busy",
    "File exists",
    "Cross-device link",
    "No such device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "File table overflow",
    "Too many open files",
    "Not a typewriter",
    "Text file busy",
    "File too large",
    "No space left on device",
    "Illegal seek",
    "Read-only file system",
    "Too many links",
    "Broken pipe",
    "Math argument out of domain of func",
    "Math result not representable"
};

void perror(const char *prefix) {
    long backup = errno;
    
    if (prefix) {
        write(STDERR_FILENO, prefix, strlen(prefix));
        write(STDERR_FILENO, ": ", 2);
    }

    if (backup < PERRMSG_MIN || backup > PERRMSG_MAX) {
        write(STDERR_FILENO, PERRMSG_UNKNWON, strlen(PERRMSG_UNKNWON));
    } else {
        write(STDERR_FILENO, errmsg[backup], strlen(errmsg[backup]));
    }

    write(STDERR_FILENO, "\n", 1);

    return;
}
