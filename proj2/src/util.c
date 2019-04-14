#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dlfcn.h>

#include <pwd.h>
#include <grp.h>

#include "util.h"


#define LOAD(name) if (name##_util == NULL) *(void **)(&name##_util) = dlsym(libc_util, #name)


void *libc_util = NULL;
static int (*sprintf_util)(char *str, const char *format, ...) = NULL;
static void (*memset_util)(void *s, int c, size_t n) = NULL;
static ssize_t (*readlink_util)(const char *restrict path, char *restrict buf, size_t bufsize) = NULL;
static int (*fileno_util)(FILE *stream) = NULL;
static struct passwd* (*getpwuid_util)(uid_t uid) = NULL;
static struct group* (*getgrgid_util)(gid_t gid) = NULL;

__attribute__((constructor)) static void load_libc() {
    libc_util = dlopen("libc.so.6", RTLD_LAZY);
}

char* fd2name(int fildes) {
    LOAD(sprintf);
    LOAD(memset);
    LOAD(readlink);

    static char path[256], name[1024];

    sprintf_util(path, "/proc/self/fd/%d", fildes);
    memset_util(name, 0, sizeof(name));

    readlink_util(path, name, sizeof(name));

    return name;
}

char* stream2name(FILE *stream) {
    LOAD(fileno);
    
    int fd = fileno_util(stream);

    if (fd == STDIN_FILENO) {
        return "<STDIN>";
    } else if (fd == STDOUT_FILENO) {
        return "<STDOUT>";
    } else if (fd == STDERR_FILENO) {
        return "<STDERR>";
    } else {
        return fd2name(fileno(stream));
    }
}

char* uid2name(uid_t uid) {
    LOAD(getpwuid);

    struct passwd *pwd;
    
    pwd = getpwuid_util(uid);

    return pwd->pw_name;
}

char* gid2name(gid_t gid) {
    LOAD(getgrgid);

    struct group *grp;
    
    grp = getgrgid_util(gid);
    
    return grp->gr_name;
}

char* dirent_handler(struct dirent *ent) {
    return (ent == NULL ? "NULL" : ent->d_name);
}

char* str_handler(char *str) {
    return (str == NULL ? "NULL" : str);
}
