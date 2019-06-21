#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debugger.h"


BUILDIN_REGESTER(start,) {
    if (dbg->status == INIT) {
        ERRRET("no executable file specified.  load first.");
    }
    if (dbg->status == RUNNING) {
        ERRRET("program %s is already running.", dbg->program);
    }

    if ((dbg->pid = fork()) < 0) {
        ERRRET("fork failed.");
    } else if (dbg->pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            ERRQUIT(1, "TRACEME failed.");
        }

        char *arg[argc + 1];
        for (int i = 1; i < argc; i++) {
            arg[i] = (char*)argv[i];
        }
        arg[0] = dbg->program;
        arg[argc] = NULL;

        execvp(dbg->program, arg);
        ERRQUIT(1, "execvp failed.");
    } else {
        int status;
        if (waitpid(dbg->pid, &status, 0) < 0) {
            ERRRET("waitpid failed.");
        }
        if (!WIFSTOPPED(status)) {
            ERRRET("child error.");
        }

        ptrace(PTRACE_SETOPTIONS, dbg->pid, 0, PTRACE_O_EXITKILL);

        dbg->status = RUNNING;
        ERRRET("pid %d", dbg->pid);
    }
}
