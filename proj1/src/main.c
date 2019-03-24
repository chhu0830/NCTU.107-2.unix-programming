#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define false 0
#define true 1


struct CONNECTION {
    char local_ip[32], rmt_ip[32];
    unsigned short int local_port, rmt_port;
    unsigned int inode;
};

struct PROCINFO {
    unsigned short int pid;
    unsigned int inode;
    char cmdline[512];
};


unsigned int read_connection(struct CONNECTION *list, char *filename, int type);
unsigned int read_fd(struct PROCINFO *info);
// TODO: Try not to pass entire `info` into read_cmdline
void read_cmdline(char *pid, struct PROCINFO *info);
char is_str_digit(char *str);


int main() {
    struct PROCINFO info[1024];
    unsigned int nconn[4], ninfo = 0;
    struct CONNECTION connections[4][1024];

    ninfo = read_fd(info);

    nconn[0] = read_connection(connections[0], "/proc/net/tcp", 0);
    nconn[1] = read_connection(connections[1], "/proc/net/udp", 1);

    return 0;
}

unsigned int read_connection(struct CONNECTION *list, char *filename, int type) {
    FILE *file = fopen(filename, "r");
    unsigned int idx, dummy, local_ip, local_port, rmt_ip, rmt_port, inode;
    char str[256];
    
    fgets(str, sizeof(str), file);

    for (idx = 0; fscanf(file, "%u:", &dummy) != EOF; idx++) {
        fscanf(file, "%x:%x %x:%x", &local_ip, &local_port, &rmt_ip, &rmt_port);

        for (int i = 0; i < 6; i++) {
            fscanf(file, "%s", str);
        }

        fscanf(file, "%u", &inode);
        
        for (int i = 0, n = (type & 1 ? 3 : 7); i < n; i++) {
            fscanf(file, "%s", str);
        }

        inet_ntop(AF_INET, &local_ip, list[idx].local_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &rmt_ip, list[idx].rmt_ip, INET_ADDRSTRLEN);
        list[idx].local_port = local_port;
        list[idx].rmt_port = rmt_port;
        list[idx].inode = inode;
    }
    
    return idx;
}

unsigned int read_fd(struct PROCINFO *info) {
    char fdpath[512], lnpath[1024], tgpath[256];
    DIR *proc, *fd;
    struct dirent *pid, *fdnum;
    unsigned int idx = 0;

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

            // Get info if the fd is socket
            unsigned int inode;
            if (sscanf(tgpath, "socket:[%u]", &inode)) {
                info[idx].pid = atoi(pid->d_name);
                info[idx].inode = inode;
                read_cmdline(pid->d_name, &info[idx]);
            }
        }
    }

    return idx;
}

void read_cmdline(char *pid, struct PROCINFO *info) {
    char cmdpath[256];
    sprintf(cmdpath, "/proc/%s/cmdline", pid);

    FILE *cmdfile = fopen(cmdpath, "r");
    
    int n = fread(info->cmdline, 1, sizeof(info->cmdline), cmdfile);
    info->cmdline[n] = '\0';

    while (--n) {
        if (info->cmdline[n] == '\0' || isspace(info->cmdline[n])) {
            info->cmdline[n] = ' ';
        }
    }
}

char is_str_digit(char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}
