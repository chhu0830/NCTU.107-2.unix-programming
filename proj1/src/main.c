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
    char local_ip[64], rmt_ip[64], *type;
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
    unsigned int nconn[4], ninfo = 0, type = 0xF;
    struct CONN conns[4][1024];
    char *filter = "";

    int c;
    while ((c = getopt_long(argc, (char*const*)argv, optstring, opts, NULL)) != -1) {
        switch (c) {
            case 't':
                // 0b0011
                type &= 0x3;
                break;
            case 'u':
                // 0b1100
                type &= 0xc;
                break;
            default:
                printf("%s [-t|--tcp] [-u|--udp] [filter-string]\n", argv[0]);
                return 1;
        }
    }
    
    if (optind < argc) {
        filter = (char*)argv[optind];
    }

    ninfo = read_fd(info);

    for (int i = 0, flag = false; i < 4; i++) {
        if (type >> i & 0x1) {
            if ((i & 0x1) == 0) {
                printf("%sList of %s connections:\n", (flag ? "\n" : ""), (i >> 1 ? "UDP" : "TCP"));
                printf("%-8s%-32s%-32s%s\n", "Proto", "Local Address", "Foreign Address", "PID/Program name and arguments");
                flag = true;
            }
            nconn[i] = read_conn(conns[i], i);
            show(info, ninfo, conns[i], nconn[i], filter);
        }
    }

    return 0;
}

unsigned int read_conn(struct CONN *list, int type) {
    unsigned int idx, dummy, local_ip[4], rmt_ip[4], local_port, rmt_port, inode;
    char str[512], filename[256];
    
    sprintf(filename, "/proc/net/%s", TYPE[type]);
    FILE *file = fopen(filename, "r");
    
    // Skip first line
    fgets(str, sizeof(str), file);

    for (idx = 0; fscanf(file, "%u:", &dummy) != EOF; idx++) {
        int n = (type & 1 ? 4 : 1);

        for (int i = 0; i < n; i++) {
            fscanf(file, "%08x", &local_ip[i]);
        }
        fscanf(file, ":%04x", &local_port);

        for (int i = 0; i < n; i++) {
            fscanf(file, "%08x", &rmt_ip[i]);
        }
        fscanf(file, ":%04x", &rmt_port);

        for (int i = 0; i < 6; i++) {
            fscanf(file, "%s", str);
        }

        fscanf(file, "%u", &inode);
        
        // Read until line end
        fgets(str, sizeof(str), file);

        const int AF = (type & 1 ? AF_INET6 : AF_INET);
        const int ADDRSTRLEN = (type & 1 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);

        inet_ntop(AF, &local_ip, list[idx].local_ip, ADDRSTRLEN);
        inet_ntop(AF, &rmt_ip, list[idx].rmt_ip, INET_ADDRSTRLEN);
        list[idx].local_port = local_port;
        list[idx].rmt_port = rmt_port;
        list[idx].inode = inode;
        list[idx].type = (char*)TYPE[type];
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
    char local_addr[128], rmt_addr[128];
    regex_t regex;

    regcomp(&regex, filter, 0);

    for (unsigned int i = 0; i < nconn; i++) {
        // TODO: Prettify
        if (conns[i].local_port) {
            sprintf(local_addr, "%s:%hu", conns[i].local_ip, conns[i].local_port);
        } else {
            sprintf(local_addr, "%s:*", conns[i].local_ip);
        }
        
        if (conns[i].rmt_port) {
            sprintf(rmt_addr, "%s:%hu", conns[i].rmt_ip, conns[i].rmt_port);
        } else {
            sprintf(rmt_addr, "%s:*", conns[i].rmt_ip);
        }

        int idx = -1;
        for (unsigned int j = 0; j < ninfo; j++) {
            if (conns[i].inode == info[j].inode && regexec(&regex, info[j].cmdline, 0, NULL, 0) == 0) {
                idx = j;
                break;
            }
        }

        if (idx >= 0 || !filter[0]) {
            printf("%-8s%-32s%-32s", conns[i].type, local_addr, rmt_addr);
            if (idx >= 0) {
                printf("%d/%s\n", info[idx].pid, info[idx].cmdline);
            } else {
                printf("-\n");
            }
        }
    }

    regfree(&regex);
}
