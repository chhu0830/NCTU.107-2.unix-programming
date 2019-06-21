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

    unsigned long long code = ptrace(PTRACE_PEEKTEXT, dbg->pid, regs.rip-1, 0);
    break_pt_t *current = dbg->bp_find_by_addr(dbg, regs.rip - 1);

    if (current != NULL) {
        if (ptrace(PTRACE_POKETEXT, dbg->pid,
                   current->addr, current->code) != 0) {
            ERRRET("patch failed.");
        }

        regs.rip -= 1;
        if (ptrace(PTRACE_SETREGS, dbg->pid, 0, &regs) != 0) {
            ERRRET("patch failed.");
        }
    }

    ptrace(PTRACE_SINGLESTEP, dbg->pid, 0, 0);
    if (waitpid(dbg->pid, &dbg->status, 0) < 0) {
        ERRQUIT(1, "waitpid failed.");
    }

    if (WIFEXITED(dbg->status)) {
        dbg->stat = LOADED;
        ERRRET("child process %d terminated normally (code %d)",
               dbg->pid, dbg->status);
    }
    
    if (current != NULL) {
        if (ptrace(PTRACE_POKETEXT, dbg->pid, current->addr, code) != 0) {
            ERRRET("break failed.");
        }
    }

}
