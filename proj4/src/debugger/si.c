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

    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, dbg->pid, 0, &regs)) {
        ERRQUIT(1, "get regs failed.");
    }

    break_pt_t *current = dbg->bp_find_by_addr(dbg, regs.rip - 1);

    if (current != NULL) {
        dbg->bp_recover(dbg, current);
    }

    ptrace(PTRACE_SINGLESTEP, dbg->pid, 0, 0);
    if (waitpid(dbg->pid, &dbg->status, 0) < 0) {
        ERRQUIT(1, "waitpid failed.");
    }

    if (WIFEXITED(dbg->status)) {
        // FIXME: return to load state
        free_debugger(dbg);
        ERRRET("child process %d terminated normally (code %d)",
               dbg->pid, dbg->status);
    }
    
    if (current != NULL) {
        dbg->bp_patch(dbg, current->addr);
    }
}
