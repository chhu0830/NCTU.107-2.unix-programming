#include <sys/types.h>
#include <dirent.h>

char* fd2name(int fildes);
char* stream2name(FILE *stream);
char* uid2name(uid_t uid);
char* gid2name(gid_t gid);

char* dirent_handler(struct dirent *ent);
char* str_handler(char *str);
