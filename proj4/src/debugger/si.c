#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "debugger.h"


BUILDIN_REGESTER(si,) {
    if (dbg->stat != RUNNING) {
        ERRRET("the program is not being run.");
    }

    break_pt_t *current = dbg->bp_check(dbg);
    if (current != NULL) {
        dbg->bp_unpatch(dbg, current);
    }

    ptrace(PTRACE_SINGLESTEP, dbg->pid, 0, 0);
    if (waitpid(dbg->pid, &dbg->status, 0) < 0) {
        ERRQUIT(1, "waitpid failed.");
    }

    if (current != NULL) {
        dbg->bp_patch(dbg, current->addr);
    }

    if (WIFEXITED(dbg->status)) {
        // FIXME: return to load state and check return status
        free_debugger(dbg);
        ERRRET("child process %d terminated normally (code %d)",
               dbg->pid, dbg->status);
    } else if (WIFSTOPPED(dbg->status)) {
        dbg->reset_rip(dbg);
    }
}
