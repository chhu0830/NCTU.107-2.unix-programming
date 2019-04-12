#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pwd.h>
#include <grp.h>

#include "util.h"


char* fd2name(int fildes) {
    static char path[256], name[1024];

    sprintf(path, "/proc/self/fd/%d", fildes);
    memset(name, 0, sizeof(name));

    readlink(path, name, sizeof(name));

    return name;
}

char* stream2name(FILE *stream) {
    int fd = fileno(stream);

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
    struct passwd *pwd;
    
    pwd = getpwuid(uid);

    return pwd->pw_name;
}

char* gid2name(gid_t gid) {
    struct group *grp;
    
    grp = getgrgid(gid);
    
    return grp->gr_name;
}

char* dirent_handler(struct dirent *ent) {
    return (ent == NULL ? "NULL" : ent->d_name);
}

char* str_handler(char *str) {
    return (str == NULL ? "NULL" : str);
}
