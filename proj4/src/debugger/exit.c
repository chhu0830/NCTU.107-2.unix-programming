#include <stdlib.h>

#include "debugger.h"


BUILDIN_REGESTER(exit, q) {
    free_debugger(dbg);
    exit(0);
}
