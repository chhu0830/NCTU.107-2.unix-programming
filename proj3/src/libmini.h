#ifndef __LIBMINI_H__
#define __LIBMINI_H__

#define DEF(RET_TYPE, NAME, ...) \
    long sys_##NAME(__VA_ARGS__); \
    RET_TYPE NAME(__VA_ARGS__);

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define NULL ((void*) 0)

/* from /usr/include/asm-generic/errno-base.h */
#define EPERM        1  /* Operation not permitted */
#define ENOENT       2  /* No such file or directory */
#define ESRCH        3  /* No such process */
#define EINTR        4  /* Interrupted system call */
#define EIO          5  /* I/O error */
#define ENXIO        6  /* No such device or address */
#define E2BIG        7  /* Argument list too long */
#define ENOEXEC      8  /* Exec format error */
#define EBADF        9  /* Bad file number */
#define ECHILD      10  /* No child processes */
#define EAGAIN      11  /* Try again */
#define ENOMEM      12  /* Out of memory */
#define EACCES      13  /* Permission denied */
#define EFAULT      14  /* Bad address */
#define ENOTBLK     15  /* Block device required */
#define EBUSY       16  /* Device or resource busy */
#define EEXIST      17  /* File exists */
#define EXDEV       18  /* Cross-device link */
#define ENODEV      19  /* No such device */
#define ENOTDIR     20  /* Not a directory */
#define EISDIR      21  /* Is a directory */
#define EINVAL      22  /* Invalid argument */
#define ENFILE      23  /* File table overflow */
#define EMFILE      24  /* Too many open files */
#define ENOTTY      25  /* Not a typewriter */
#define ETXTBSY     26  /* Text file busy */
#define EFBIG       27  /* File too large */
#define ENOSPC      28  /* No space left on device */
#define ESPIPE      29  /* Illegal seek */
#define EROFS       30  /* Read-only file system */
#define EMLINK      31  /* Too many links */
#define EPIPE       32  /* Broken pipe */
#define EDOM        33  /* Math argument out of domain of func */
#define ERANGE      34  /* Math result not representable */

typedef unsigned long long size_t;
typedef long long ssize_t;

extern long errno;


#define PERRMSG_MIN 0
#define PERRMSG_MAX 34
#define PERRMSG_UNKNWON "UnKnown"

DEF(ssize_t, write, int, const void*, size_t)
DEF(int, pause)
DEF(unsigned int, alarm, unsigned int)

long sys_exit(int) __attribute__ ((noreturn));
void exit(int);

size_t strlen(const char *s);
void perror(const char *prefix);

#endif /* __LIBMINI_H__ */
