#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "debugger.h"


BUILDIN_REGESTER(cont, c) {
    if (dbg->stat != RUNNING) {
        ERRRET("the program is not being run.");
    }

    char *arg[argc + 1];
    copy_argv("si", argc, arg, argv);
    dbg->exec(dbg, argc, (const char**)arg);

    dbg->cmd(dbg, PTRACE_CONT);
}
