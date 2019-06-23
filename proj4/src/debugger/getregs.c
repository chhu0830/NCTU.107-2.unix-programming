#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>
#include <sys/user.h>

#include "debugger.h"


BUILDIN_REGESTER(getregs,) {
    if (dbg->stat != RUNNING) {
        ERRRET("the program is not being run.");
    }

    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, dbg->pid, 0, &regs)) {
        ERRQUIT(1, "get regs failed.");
    }

    fprintf(stdout, "RAX %-18llxRBX %-18llxRCX %-18llxRDX %-18llx\n",
            regs.rax, regs.rbx, regs.rcx, regs.rdx);
    fprintf(stdout, "R8  %-18llxR9  %-18llxR10 %-18llxR11 %-18llx\n",
            regs.r8, regs.r9, regs.r10, regs.r11);
    fprintf(stdout, "R12 %-18llxR13 %-18llxR14 %-18llxR15 %-18llx\n",
            regs.r12, regs.r13, regs.r14, regs.r15);
    fprintf(stdout, "RDI %-18llxRSI %-18llxRBP %-18llxRSP %-18llx\n",
            regs.rdi, regs.rsi, regs.rbp, regs.rsp);
    fprintf(stdout, "RIP %-18llxFLAGS %016llx\n",
            regs.rip, regs.eflags);
}
