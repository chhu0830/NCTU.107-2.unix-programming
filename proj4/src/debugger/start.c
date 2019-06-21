#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debugger.h"


BUILDIN_REGESTER(start,) {
    if (dbg->stat == INIT) {
        ERRRET("no executable file specified.  load first.");
    }
    if (dbg->stat == RUNNING) {
        ERRRET("program %s is already running.", dbg->program);
    }

    if ((dbg->pid = fork()) < 0) {
        ERRRET("fork failed.");
    } else if (dbg->pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            ERRQUIT(1, "TRACEME failed.");
        }

        char *arg[argc + 1];
        copy_argv(dbg->program, argc, arg, argv);

        execvp(dbg->program, arg);
        ERRQUIT(1, "execvp failed.");
    } else {
        if (waitpid(dbg->pid, &dbg->status, 0) < 0) {
            ERRRET("waitpid failed.");
        }
        if (!WIFSTOPPED(dbg->status)) {
            ERRRET("child error.");
        }

        // FIXME: calculate dbg base
        ptrace(PTRACE_SETOPTIONS, dbg->pid, 0, PTRACE_O_EXITKILL);

        break_pt_t *current = dbg->bp;
        while (current != NULL) {
            current->code = dbg->bp_patch(dbg, current->addr);
            current = current->next;
        }

        dbg->stat = RUNNING;
        ERRRET("pid %d", dbg->pid);
    }
}
