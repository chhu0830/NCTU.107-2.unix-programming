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

    dbg->cmd(dbg, PTRACE_SINGLESTEP);

    if (current != NULL) {
        dbg->bp_patch(dbg, current->addr);
    }

}
