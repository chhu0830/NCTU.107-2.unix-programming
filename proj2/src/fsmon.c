#include <stdio.h>

#include <dlfcn.h>
#include <stdarg.h>
#include <sys/types.h>

#include "util.h"


#define FUNC(type, name, args1, args2)                                      \
    type (*name##_libc)(args1) = NULL;                                      \
    void name##_handler(args1);                                             \
    type name(args1) {                                                      \
        if (name##_libc == NULL) name##_libc = dlsym(libc, #name);          \
                                                                            \
        name##_handler(args2);                                              \
                                                                            \
        type ret = name##_libc(args2);                                      \
        fprintf(OUTPUT, "%d\n", ret);                                       \
        return ret;                                                         \
    }                                                                       \
    void name##_handler(args1)

#define LIST(...) __VA_ARGS__


FILE *OUTPUT = NULL;
void *libc = NULL;


__attribute__((constructor)) static void load_libc() {
    libc = dlopen("libc.so.6", RTLD_LAZY);
    OUTPUT = stderr;
}


FUNC(int, open, LIST(const char *path, int oflag), LIST(path, oflag)) {
    fprintf(OUTPUT, "open(\"%s\", %x) = ", path, oflag);
}

FUNC(ssize_t, read, LIST(int fildes, void *buf, size_t nbyte), LIST(fildes, buf, nbyte)) {
    fprintf(OUTPUT, "read(\"%s\", %p, %lu) = ", fd2name(fildes), buf, nbyte);
}

FUNC(int, close, LIST(int fildes), LIST(fildes)) {
    fprintf(OUTPUT, "close(\"%s\") = ", fd2name(fildes));
}
