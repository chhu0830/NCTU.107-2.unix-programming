#include <stdio.h>
#include <stdint.h>

#include <dlfcn.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "util.h"


#define FMT(TYPE) _Generic((TYPE),                                          \
                    int: "%d",                                              \
                    char*: "%s",                                            \
                    size_t: "%zu",                                          \
                    ssize_t: "%zd",                                         \
                    FILE*: "%p",                                            \
                    DIR*: "%p",                                             \
                    struct dirent*: "%s"                                    \
                )

#define TRANS(RET) _Generic((RET),                                          \
                    default: RET,                                           \
                    char*: str_handler(RET),                                \
                    struct dirent*: dirent_handler(RET)                     \
                )


#define FUNC(type, name, args1, args2, fmt, args3)                          \
    type (*name##_libc)(args1) = NULL;                                      \
    type name(args1) {                                                      \
        if (name##_libc == NULL) {                                          \
            *(void **)(&name##_libc) = dlsym(libc, #name);                  \
        }                                                                   \
                                                                            \
        fprintf(OUTPUT, #name#fmt" = ", args3);                             \
                                                                            \
        type ret = name##_libc(args2);                                      \
        fprintf(OUTPUT, FMT(ret), TRANS(ret));                              \
        fprintf(OUTPUT, "\n");                                              \
                                                                            \
        return ret;                                                         \
    }                                                                       \

#define LIST(...) __VA_ARGS__


FILE *OUTPUT = NULL;
void *libc = NULL;


__attribute__((constructor)) static void load_libc() {
    libc = dlopen("libc.so.6", RTLD_LAZY);
    OUTPUT = stderr;
}


// FIXME: dir2name
FUNC(int, closedir,
        LIST(DIR *dirp),
        LIST(dirp),
        (%p),
        LIST((void*)dirp))

// FIXME: dir2name
FUNC(DIR*, opendir,
        LIST(const char *name),
        LIST(name),
        ("%s"),
        LIST(name))

// FIXME: dir2name
FUNC(struct dirent*, readdir,
        LIST(DIR *dirp),
        LIST(dirp),
        (%p),
        LIST((void*)dirp))

FUNC(int, creat,
        LIST(const char *path, mode_t mode),
        LIST(path, mode),
        ("%s", %o),
        LIST(path, mode))

// FIXME: flag2string?, int open(const char *path, int oflag, ...)
FUNC(int, open,
        LIST(const char *path, int oflag),
        LIST(path, oflag),
        ("%s", %x),
        LIST(path, oflag))

// FIXME: no read?
FUNC(ssize_t, read,
        LIST(int fildes, void *buf, size_t nbyte),
        LIST(fildes, buf, nbyte),
        ("%s", %p, %zu),
        LIST(fd2name(fildes), buf, nbyte));

FUNC(ssize_t, write,
        LIST(int fildes, const void *buf, size_t nbyte),
        LIST(fildes, buf, nbyte),
        ("%s", %p, %zu),
        LIST(fd2name(fildes), buf, nbyte))

FUNC(int, dup,
        LIST(int fildes),
        LIST(fildes),
        (%d),
        LIST(fildes))

FUNC(int, dup2,
        LIST(int fildes, int fildes2),
        LIST(fildes, fildes2),
        (%d, %d),
        LIST(fildes, fildes2))

FUNC(int, close,
        LIST(int fildes),
        LIST(fildes),
        ("%s"),
        LIST(fd2name(fildes)))

// FIXME: change name to lstat?
FUNC(int, __lxstat,
        LIST(int ver, const char *path, struct stat *stat_buf),
        LIST(ver, path, stat_buf),
        ("%s", %p {mode=%o, size=%d}),
        LIST(path, stat_buf, stat_buf->st_mode, stat_buf->st_size))

// FIXME: change name to stat?
FUNC(int, __xstat,
        LIST(int ver, const char *path, struct stat *stat_buf),
        LIST(ver, path, stat_buf),
        ("%s", %p {mode=%o, size=%d}),
        LIST(path, stat_buf, stat_buf->st_mode, stat_buf->st_size))

FUNC(ssize_t, pwrite,
        LIST(int fildes, const void *buf, size_t nbyte, off_t offset),
        LIST(fildes, buf, nbyte, offset),
        ("%s", %p, %zu, %ld),
        LIST(fd2name(fildes), buf, nbyte, offset))

FUNC(FILE*, fopen,
        LIST(const char *pathname, const char *mode),
        LIST(pathname, mode),
        ("%s", "%s"),
        LIST(pathname, mode))

FUNC(int, fclose,
        LIST(FILE *stream),
        LIST(stream),
        ("%s"),
        stream2name(stream))

FUNC(size_t, fread,
        LIST(void *ptr, size_t size, size_t nmemb, FILE *stream),
        LIST(ptr, size, nmemb, stream),
        (%p, %zu, %zu, "%s"),
        LIST(ptr, size, nmemb, stream2name(stream)))

FUNC(size_t, fwrite,
        LIST(const void *ptr, size_t size, size_t nmemb, FILE *stream),
        LIST(ptr, size, nmemb, stream),
        (%p, %zu, %zu, "%s"),
        LIST(ptr, size, nmemb, stream2name(stream)))

FUNC(char*, fgets,
        LIST(char *s, int size, FILE *stream),
        LIST(s, size, stream),
        (%p, %d, "%s"),
        LIST(s, size, stream2name(stream)))

FUNC(int, chdir,
        LIST(const char *path),
        LIST(path),
        ("%s"),
        LIST(path))

FUNC(int, chown,
        LIST(const char *path, uid_t owner, gid_t group),
        LIST(path, owner, group),
        ("%s", %u(%s), %u(%s)),
        LIST(path, owner, uid2name(owner), group, gid2name(group)))

FUNC(int, chmod,
        LIST(const char *path, mode_t mode),
        LIST(path, mode),
        ("%s", %o),
        LIST(path, mode))

FUNC(int, remove,
        LIST(const char *pathname),
        LIST(pathname),
        ("%s"),
        LIST(pathname))

FUNC(int, rename,
        LIST(const char *old, const char *new),
        LIST(old, new),
        ("%s", "%s"),
        LIST(old, new))

FUNC(int, link,
        LIST(const char *path1, const char *path2),
        LIST(path1, path2),
        ("%s", "%s"),
        LIST(path1, path2))

FUNC(int, unlink,
        LIST(const char *path),
        LIST(path),
        ("%s"),
        LIST(path))

FUNC(ssize_t, readlink,
        LIST(const char *restrict path, char *restrict buf, size_t bufsize),
        LIST(path, buf, bufsize),
        ("%s", %p, %zu),
        LIST(path, buf, bufsize))

FUNC(int, symlink,
        LIST(const char *path1, const char *path2),
        LIST(path1, path2),
        ("%s", "%s"),
        LIST(path1, path2))

FUNC(int, mkdir,
        LIST(const char *path, mode_t mode),
        LIST(path, mode),
        ("%s", %o),
        LIST(path, mode))

FUNC(int, rmdir,
        LIST(const char *path),
        LIST(path),
        ("%s"),
        LIST(path))
