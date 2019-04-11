#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util.h"


char* fd2name(int fildes) {
    static char path[256], name[1024];

    sprintf(path, "/proc/self/fd/%d", fildes);
    memset(name, 0, sizeof(name));

    readlink(path, name, sizeof(name));

    return name;
}

char* stream2name(FILE *stream) {
    static char name[1024];
    
    if (stream == stdin) {
        return "<STDIN>";
    } else if (stream == stdout) {
        return "<STDOUT>";
    } else if (stream == stderr) {
        return "<STDERR>";
    } else {
        return fd2name(fileno(stream));
    }
}
