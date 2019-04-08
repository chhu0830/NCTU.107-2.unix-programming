#include <stdio.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <stdarg.h>


#define FUNC(type, name, args1, args2) \
    type (*name##_libc)(args1) = NULL; \
    void name##_handler(type ret, args1); \
    type name(args1) { \
        if (name##_libc == NULL) name##_libc = dlsym(libc, #name); \
        type ret = name##_libc(args2); \
        name##_handler(ret, args2); \
        return ret; \
    } \
    void name##_handler(type ret, args1)

#define LIST(...) __VA_ARGS__


void *libc = NULL;


__attribute__((constructor)) static void load_libc() {
    libc = dlopen("libc.so.6", RTLD_LAZY);
}
