#include <stdio.h>
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


unsigned int read_connection(struct CONNECTION *list, char *filename, char type);
char is_str_digit(char *str);


int main() {
    char fdpath[512], lnpath[1024], tgpath[256];
    DIR *proc, *fd;
    struct dirent *pid, *fdnum;

    struct CONNECTION tcp[1024], udp[1024];
    memset(tcp, 0, sizeof(tcp));
    memset(udp, 0, sizeof(udp));
    read_connection(tcp, "/proc/net/tcp", 0);
    read_connection(udp, "/proc/net/udp", 1);
    
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
            }

        }
    }
    
    return 0;
}

unsigned int read_connection(struct CONNECTION *list, char *filename, char type) {
    FILE *file = fopen(filename, "r");
    unsigned int c, dummy, local_ip, local_port, rmt_ip, rmt_port, inode;
    char str[256];
    
    fgets(str, sizeof(str), file);

    for (c = 0; fscanf(file, "%u:", &dummy) != EOF; c++) {
        fscanf(file, "%x:%x %x:%x", &local_ip, &local_port, &rmt_ip, &rmt_port);

        for (int i = 0; i < 6; i++) {
            fscanf(file, "%s", str);
        }

        fscanf(file, "%u", &inode);
        
        for (int i = 0, n = (type & 1 ? 3 : 7); i < n; i++) {
            fscanf(file, "%s", str);
        }

        inet_ntop(AF_INET, &local_ip, list[c].local_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &rmt_ip, list[c].rmt_ip, INET_ADDRSTRLEN);
        list[c].local_port = local_port;
        list[c].rmt_port = rmt_port;
        list[c].inode = inode;
    }
    
    return c;
}

char is_str_digit(char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}
