#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <regex.h>
#include <getopt.h>

#define false 0
#define true 1

struct CONN {
    char local_ip[32], rmt_ip[32], *type;
    unsigned short int local_port, rmt_port;
    unsigned int inode;
};

struct PROCINFO {
    unsigned int inode, pid;
    char cmdline[512];
};


unsigned int read_conn(struct CONN *list, int type);
unsigned int read_fd(struct PROCINFO *info);
// TODO: Try not to pass entire `info` into read_cmdline
void read_cmdline(char *pid, struct PROCINFO *info);
char is_str_digit(char *str);
void show(struct PROCINFO *info, unsigned int ninfo, struct CONN *conns, unsigned int nconn, char *filter);


const char TYPE[][8] = {"tcp", "tcp6", "udp", "udp6"};

const struct option opts[] = {
    {"tcp", 0, NULL, 't'},
    {"udp", 0, NULL, 'u'}
};

const char *optstring = "tu";


int main(int argc, const char *argv[]) {
    struct PROCINFO info[1024];
    unsigned int nconn[4], ninfo = 0, type = 0x1111;
    struct CONN conns[4][1024];
    char *filter = ".";

    int c;
    while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
        switch (c) {
            case 't':
                type &= 0x0011;
                break;
            case 'u':
                type &= 0x1100;
                break;
            default:
                printf("%s [-t|--tcp] [-u|--udp] [filter-string]\n", argv[0]);
                return 1;
        }
    }
    
    if (optind < argc) {
        filter = argv[optind];
    }

    ninfo = read_fd(info);

    for (int i = 0; i < 4; i++) {
        if (type >> i & 0x1) {
            nconn[i] = read_conn(conns[i], i);
            show(info, ninfo, conns[i], nconn[i], filter);
        }
    }

    return 0;
}

unsigned int read_conn(struct CONN *list, int type) {
    unsigned int idx, dummy, local_ip, local_port, rmt_ip, rmt_port, inode;
    char str[512], filename[256];
    
    sprintf(filename, "/proc/net/%s", TYPE[type]);
    FILE *file = fopen(filename, "r");
    
    // Skip first line
    fgets(str, sizeof(str), file);

    for (idx = 0; fscanf(file, "%u:", &dummy) != EOF; idx++) {
        fscanf(file, "%x:%x %x:%x", &local_ip, &local_port, &rmt_ip, &rmt_port);

        for (int i = 0; i < 6; i++) {
            fscanf(file, "%s", str);
        }

        fscanf(file, "%u", &inode);
        
        // Read until line end
        fgets(str, sizeof(str), file);

        inet_ntop(AF_INET, &local_ip, list[idx].local_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &rmt_ip, list[idx].rmt_ip, INET_ADDRSTRLEN);
        list[idx].local_port = local_port;
        list[idx].rmt_port = rmt_port;
        list[idx].inode = inode;
        list[idx].type = TYPE[type];
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
                idx++;
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

void show(struct PROCINFO *info, unsigned int ninfo, struct CONN *conns, unsigned int nconn, char *filter) {
    regex_t regex;

    regcomp(&regex, filter, 0);

    for (unsigned int i = 0; i < nconn; i++) {
        for (unsigned int j = 0; j < ninfo; j++) {
            if (conns[i].inode == info[j].inode && regexec(&regex, info[j].cmdline, 0, NULL, 0) == 0) {
                printf("%s\t%s:%u\t%s:%u", conns[i].type, conns[i].local_ip, conns[i].local_port, conns[i].rmt_ip, conns[i].rmt_port);
                printf("\t%d/%s\n", info[j].pid, info[j].cmdline);
            }
        }
    }
}
