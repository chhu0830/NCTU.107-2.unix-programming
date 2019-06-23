#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ptrace.h>
#include <sys/user.h>

#include "debugger.h"


BUILDIN_REGESTER(get, g) {
    if (argc < 2) {
        ERRRET("no reg name specified.");
    }

    if (dbg->stat != RUNNING) {
        ERRRET("the program is not being run.");
    }

    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, dbg->pid, 0, &regs)) {
        ERRQUIT(1, "get regs failed.");
    }

    char *regs_list[] = {"r15", "r14", "r13", "r12", "rbp", "rbx", "r11", "r10", "r9", "r8", "rax", "rcx", "rdx", "rsi", "rdi", "orig_rax", "rip", "cs", "eflags", "rsp", "ss", "fs_base", "gs_base", "ds", "es", "fs", "gs", NULL};


    for (int i = 0; regs_list[i]; i++) {
        if (strcmp(regs_list[i], argv[1]) == 0) {
            unsigned long long value = *((unsigned long long*)&regs + i);
            fprintf(stdout, "%s = %lld (%#llx)\n",
                    regs_list[i], value, value);
            return;
        }
    }

    ERRRET("not valid reg name.");
}
