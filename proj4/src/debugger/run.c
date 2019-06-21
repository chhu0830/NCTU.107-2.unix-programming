#include <stdio.h>

#include "debugger.h"


BUILDIN_REGESTER(run, r) {
    if (dbg->stat == INIT) {
        ERRRET("no executable file specified.  load first.");
    }

    char *arg[argc + 1];

    if (dbg->stat == LOADED) {
        copy_argv("start", argc, arg, argv);
        dbg->exec(dbg, argc, (const char**)arg);
    } else if (dbg->stat == RUNNING) {
        ERRMSG("program %s is already running.", dbg->program);
    }

    copy_argv("cont", argc, arg, argv);
    dbg->exec(dbg, argc, (const char**)arg);
}
