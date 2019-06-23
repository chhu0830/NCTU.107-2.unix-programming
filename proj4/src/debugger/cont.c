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

    ptrace(PTRACE_CONT, dbg->pid, 0, 0);
    if (waitpid(dbg->pid, &dbg->status, 0) < 0) {
        ERRQUIT(1, "waitpid failed.");
    }

    if (WIFEXITED(dbg->status)) {
        // FIXME: return to load state
        free_debugger(dbg);
        ERRRET("child process %d terminated normally (code %d)",
               dbg->pid, dbg->status);
    } else if (WIFSTOPPED(dbg->status)) {
        dbg->reset_rip(dbg);
    } else {
        // FIXME: continue execute?
    }
}
