#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "util.h"


#define FUNC(type, exportname, funcname, args1, args2,                      \
             preprocess, fmt, args3)                                        \
    static type (*exportname##_libc)(args1) = NULL;                         \
    type exportname(args1) {                                                \
        if (exportname##_libc == NULL) {                                    \
            *(void **)(&exportname##_libc) = dlsym(libc, #exportname);      \
        }                                                                   \
                                                                            \
        preprocess                                                          \
                                                                            \
        type ret = exportname##_libc(args2);                                \
        dprintf_util(OUTPUT, "%s"#fmt"\n",                                  \
                     (#funcname[0] ? #funcname : #exportname), args3);      \
                                                                            \
        return ret;                                                         \
    }                                                                       \

#define LIST(...) __VA_ARGS__


int OUTPUT = -1;
void *libc = NULL;
static int (*dprintf_util)(int fd, const char *format, ...) = NULL;
static int (*open_util)(const char *path, int oflag, mode_t mode) = NULL; 
static int (*fileno_util)(FILE *stream) = NULL;


__attribute__((constructor)) static void load_libc() {
    libc = dlopen("libc.so.6", RTLD_LAZY);

    *(void **)(&dprintf_util) = dlsym(libc, "dprintf");
    *(void **)(&open_util) = dlsym(libc, "open");
    *(void **)(&fileno_util) = dlsym(libc, "fileno");

    if (getenv("MONITOR_OUTPUT")) {
        OUTPUT = open_util(getenv("MONITOR_OUTPUT"), O_WRONLY | O_TRUNC | O_CREAT, 0644);
    } else {
        OUTPUT = fileno_util(stderr);
    }
}


static int (*vfscanf_libc)(FILE *stream, const char *format, va_list ap) = NULL;
int fscanf(FILE *stream, const char *format, ...) {
    if (vfscanf_libc == NULL) {
        *(void **)(&vfscanf_libc) = dlsym(libc, "vfscanf");
    }

    va_list ap;
    va_start(ap, format);

    int ret = vfscanf_libc(stream, format, ap);
    dprintf_util(OUTPUT, "fscanf(\"%s\", \"%s\", ...) = %d\n", stream2name(stream), format, ret);

    va_end(ap);

    return ret;
}

static int (*vfprintf_libc)(FILE *stream, const char *format, va_list ap) = NULL;
int fprintf(FILE *stream, const char *format, ...) {
    if (vfprintf_libc == NULL) {
        *(void **)(&vfprintf_libc) = dlsym(libc, "vfprintf");
    }

    va_list ap;
    va_start(ap, format);

    int ret = vfprintf_libc(stream, format, ap);
    dprintf_util(OUTPUT, "fprintf(\"%s\", \"%s\", ...) = %d\n", stream2name(stream), format, ret);

    va_end(ap);

    return ret;
}

FUNC(int, closedir,,
        LIST(DIR *dirp),
        LIST(dirp), char *name = fd2name(dirfd(dirp));,
        ("%s") = %d,
        LIST(name, ret))

FUNC(DIR*, opendir,,
        LIST(const char *name),
        LIST(name),,
        ("%s") = %s,
        LIST(name, fd2name(dirfd(ret))))

FUNC(struct dirent*, readdir,,
        LIST(DIR *dirp),
        LIST(dirp), char *name = fd2name(dirfd(dirp));,
        ("%s") = %s,
        LIST(name, dirent_handler(ret)))

FUNC(int, creat,,
        LIST(const char *path, mode_t mode),
        LIST(path, mode),,
        ("%s", %05o) = %d,
        LIST(path, mode, ret))

static int (*open_libc)(const char *path, int oflag, ...) = NULL;
int open(const char *path, int oflag, ...) {
    if (open_libc == NULL) {
        *(void **)(&open_libc) = dlsym(libc, "open");
    }

    va_list ap;
    va_start(ap, oflag);

    int ret;
    unsigned int mode;

    if ((oflag & O_CREAT) == O_CREAT) {
        mode = va_arg(ap, unsigned int);

        ret = open_libc(path, oflag, mode);
        dprintf_util(OUTPUT, "open(\"%s\", %x, %o) = %d\n", path, oflag, mode, ret);
    } else {
        ret = open_libc(path, oflag);
        dprintf_util(OUTPUT, "open(\"%s\", %x) = %d\n", path, oflag, ret);
    }

    va_end(ap);

    return ret;
}

FUNC(ssize_t, read,,
        LIST(int fildes, void *buf, size_t nbyte),
        LIST(fildes, buf, nbyte),,
        ("%s", %p, %zu) = %d,
        LIST(fd2name(fildes), buf, nbyte, ret))

FUNC(ssize_t, write,,
        LIST(int fildes, const void *buf, size_t nbyte),
        LIST(fildes, buf, nbyte),,
        ("%s", %p, %zu) = %zd,
        LIST(fd2name(fildes), buf, nbyte, ret))

FUNC(int, dup,,
        LIST(int fildes),
        LIST(fildes),,
        (%d) = %d,
        LIST(fildes, ret))

FUNC(int, dup2,,
        LIST(int fildes, int fildes2),
        LIST(fildes, fildes2),,
        (%d, %d) = %d,
        LIST(fildes, fildes2, ret))

FUNC(int, close,,
        LIST(int fildes),
        LIST(fildes), char *name = fd2name(fildes);,
        ("%s") = %d,
        LIST(name, ret))

FUNC(int, __lxstat, lstat,
        LIST(int ver, const char *path, struct stat *stat_buf),
        LIST(ver, path, stat_buf),,
        ("%s", %p {mode=%05o, size=%d}) = %d,
        LIST(path, stat_buf, stat_buf->st_mode, stat_buf->st_size, ret))

FUNC(int, __xstat, stat,
        LIST(int ver, const char *path, struct stat *stat_buf),
        LIST(ver, path, stat_buf),,
        ("%s", %p {mode=%05o, size=%d}) = %d,
        LIST(path, stat_buf, stat_buf->st_mode, stat_buf->st_size, ret))

FUNC(ssize_t, pwrite,,
        LIST(int fildes, const void *buf, size_t nbyte, off_t offset),
        LIST(fildes, buf, nbyte, offset),,
        ("%s", %p, %zu, %ld) = %zd,
        LIST(fd2name(fildes), buf, nbyte, offset, ret))

FUNC(FILE*, fopen,,
        LIST(const char *pathname, const char *mode),
        LIST(pathname, mode),,
        ("%s", "%s") = %p,
        LIST(pathname, mode, ret))

// FIXME: stderr close, can not show
FUNC(int, fclose,,
        LIST(FILE *stream),
        LIST(stream), char *name = stream2name(stream);,
        ("%s") = %d,
        LIST(name, ret))

FUNC(size_t, fread,,
        LIST(void *ptr, size_t size, size_t nmemb, FILE *stream),
        LIST(ptr, size, nmemb, stream),,
        (%p, %zu, %zu, "%s") = %zu,
        LIST(ptr, size, nmemb, stream2name(stream), ret))

FUNC(size_t, fwrite,,
        LIST(const void *ptr, size_t size, size_t nmemb, FILE *stream),
        LIST(ptr, size, nmemb, stream),,
        (%p, %zu, %zu, "%s") = %zu,
        LIST(ptr, size, nmemb, stream2name(stream), ret))

FUNC(int, fgetc,,
        LIST(FILE *stream),
        LIST(stream),,
        ("%s") = %d,
        LIST(stream2name(stream), ret))

FUNC(char*, fgets,,
        LIST(char *s, int size, FILE *stream),
        LIST(s, size, stream),,
        (%p, %d, "%s") = %p,
        LIST(s, size, stream2name(stream), ret))

FUNC(int, chdir,,
        LIST(const char *path),
        LIST(path),,
        ("%s") = %d,
        LIST(path, ret))

FUNC(int, chown,,
        LIST(const char *path, uid_t owner, gid_t group),
        LIST(path, owner, group),,
        ("%s", %u(%s), %u(%s)) = %d,
        LIST(path, owner, uid2name(owner), group, gid2name(group), ret))

FUNC(int, chmod,,
        LIST(const char *path, mode_t mode),
        LIST(path, mode),,
        ("%s", %o) = %d,
        LIST(path, mode, ret))

FUNC(int, remove,,
        LIST(const char *pathname),
        LIST(pathname),,
        ("%s") = %d,
        LIST(pathname, ret))

FUNC(int, rename,,
        LIST(const char *old, const char *new),
        LIST(old, new),,
        ("%s", "%s") = %d,
        LIST(old, new, ret))

FUNC(int, link,,
        LIST(const char *path1, const char *path2),
        LIST(path1, path2),,
        ("%s", "%s") = %d,
        LIST(path1, path2, ret))

FUNC(int, unlink,,
        LIST(const char *path),
        LIST(path),,
        ("%s") = %d,
        LIST(path, ret))

FUNC(ssize_t, readlink,,
        LIST(const char *restrict path, char *restrict buf, size_t bufsize),
        LIST(path, buf, bufsize),,
        ("%s", %p, %zu) = %zd,
        LIST(path, buf, bufsize, ret))

FUNC(int, symlink,,
        LIST(const char *path1, const char *path2),
        LIST(path1, path2),,
        ("%s", "%s") = %d,
        LIST(path1, path2, ret))

FUNC(int, mkdir,,
        LIST(const char *path, mode_t mode),
        LIST(path, mode),,
        ("%s", %o) = %d,
        LIST(path, mode, ret))

FUNC(int, rmdir,,
        LIST(const char *path),
        LIST(path),,
        ("%s") = %d,
        LIST(path, ret))
