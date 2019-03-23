#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define false 0
#define true 1

short int is_str_digit(char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

int main() {
    char fdpath[512], lnpath[1024], tgpath[256];
    DIR *proc, *fd;
    struct dirent *pid, *fdnum;
    
    proc = opendir("/proc");
    
    while ((pid = readdir(proc)) != NULL) {
        // Only /proc/[pid]/
        if (!is_str_digit(pid->d_name)) {
            continue;
        }

        sprintf(fdpath, "/proc/%s/fd", pid->d_name);

        // Skip if fail to open dir due to permission deny or other reason
        if ((fd = opendir(fdpath)) == NULL) {
            continue;
        }
        
        while ((fdnum = readdir(fd)) != NULL) {
            // Skip . and ..
            if (!is_str_digit(fdnum->d_name)) {
                continue;
            }

            sprintf(lnpath, "%s/%s", fdpath, fdnum->d_name);

            // Follow the link
            memset(tgpath, 0, sizeof(tgpath));
            readlink(lnpath, tgpath, sizeof(tgpath));

            // Get inode if the fd is socket
            int inode;
            if (sscanf(tgpath, "socket:[%d]", &inode)) {
                printf("%s %s %d\n", lnpath, tgpath, inode);
            }

        }
    }
    
    return 0;
}
