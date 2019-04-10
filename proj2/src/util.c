#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util.h"


char* fd2name(int fildes) {
  static char path[1024], name[1024];

  sprintf(path, "/proc/self/fd/%d", fildes);
  memset(name, 0, sizeof(name));
  
  readlink(path, name, sizeof(name));

  return name;
}
