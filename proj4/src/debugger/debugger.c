#include "debugger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <capstone/capstone.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "elftool.h"


void exec(debugger_t *dbg, int argc, const char **argv);
void cmd(debugger_t *dbg, const int param);
void bp_add(debugger_t *dbg, unsigned long long target);
unsigned long long bp_patch(debugger_t *dbg, unsigned long long target);
void bp_unpatch(debugger_t *dbg, break_pt_t *break_pt);
break_pt_t* bp_find_by_addr(debugger_t *dbg, unsigned long long addr);
break_pt_t* bp_check(debugger_t *dbg);
void reset_rip(debugger_t *dbg);
int disasm(debugger_t *dbg, void* code, int length, unsigned long long addr, int n);


struct BUILDIN_FUNC *list;


debugger_t* init_debugger() {
    debugger_t *dbg = calloc(1, sizeof(debugger_t));

    dbg->exec = exec;
    dbg->cmd = cmd;
    dbg->bp_patch = bp_patch;
    dbg->bp_unpatch = bp_unpatch;
    dbg->bp_find_by_addr = bp_find_by_addr;
    dbg->bp_check = bp_check;
    dbg->reset_rip = reset_rip;
    dbg->disasm = disasm;

    dbg->stat = INIT;
    return dbg;
}

void stop_debugger(debugger_t *dbg) {
    if (dbg->stat == INIT) return;
    if (dbg->stat == RUNNING) {
        if (kill(dbg->pid, SIGKILL) == 0) {
            waitpid(dbg->pid, NULL, 0);
        }
        if (dbg->ptext->vaddr == 0) {
            break_pt_t *current = dbg->bp;
            while (current != NULL) {
                current->addr -= dbg->base;
                current = current->next;
            }
        }
    }
    dbg->base = 0;
    dbg->ldump = 0;
    dbg->ldisasm = 0;
    dbg->stat = LOADED;
}

void reset_debugger(debugger_t *dbg) {
    break_pt_t *current = dbg->bp, *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    elf_close(dbg->eh);

    dbg->bpi = 0;
    dbg->base = 0;
    dbg->ldump = 0;
    dbg->ldisasm = 0;
    dbg->eh = NULL;
    dbg->ptext = NULL;
    dbg->stext = NULL;
    dbg->bp = NULL;
    dbg->stat = INIT;
}

void free_debugger(debugger_t *dbg) {
    stop_debugger(dbg);
    reset_debugger(dbg);
}

void copy_argv(char *first, int argc, char **new_argv, const char **old_argv) {
    for (int i = 1; i < argc; i++) {
        new_argv[i] = (char*)old_argv[i];
    }
    if (first != NULL) new_argv[0] = first;
    new_argv[argc] = NULL;
}

void exec(debugger_t *dbg, int argc, const char **argv) {
    struct BUILDIN_FUNC *current = list;

    while (current != NULL) {
        if (strcmp(current->name, argv[0]) == 0
            || strcmp(current->nick, argv[0]) == 0) break;
        current = current->next;
    }
    
    if (current != NULL) current->exec(dbg, argc, argv);
    else {
        fprintf(stderr, "** undefined command: '%s'.\n", argv[0]);
    }
}

void cmd(debugger_t *dbg, const int param) {
    ptrace(param, dbg->pid, 0, 0);
    if (waitpid(dbg->pid, &dbg->status, 0) < 0) {
        ERRQUIT(1, "waitpid failed.");
    }

    if (WIFEXITED(dbg->status)) {
        stop_debugger(dbg);
        if (dbg->status != 0) {
            ERRRET("child process %d terminated abnormally (code %d)",
                   dbg->pid, dbg->status);
        } else {
            ERRRET("child process %d terminated normally (code %d)",
                   dbg->pid, dbg->status);
        }
    } else if (WIFSTOPPED(dbg->status)) {
        dbg->reset_rip(dbg);
    } else {
        ERRRET("child process %d signaled", dbg->pid);
    }
}

unsigned long long bp_patch(debugger_t *dbg, unsigned long long target) {
    unsigned long long code = ptrace(PTRACE_PEEKTEXT, dbg->pid, target, 0);
    if (ptrace(PTRACE_POKETEXT, dbg->pid, target,
               (code & 0xffffffffffffff00) | 0xcc) != 0) {
        ERRMSG("break failed.");
        code = 0;
    }
    return code;
}

void bp_unpatch(debugger_t *dbg, break_pt_t *break_pt) {
    if (ptrace(PTRACE_POKETEXT, dbg->pid,
               break_pt->addr, break_pt->code) != 0) {
        ERRRET("patch failed.");
    }
}

break_pt_t* bp_find_by_addr(debugger_t *dbg, unsigned long long addr) {
    break_pt_t *current = dbg->bp;
    while (current != NULL) {
        if (current->addr == addr) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

break_pt_t* bp_check(debugger_t *dbg) {
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, dbg->pid, 0, &regs)) {
        ERRQUIT(1, "get regs failed.");
    }

    return dbg->bp_find_by_addr(dbg, regs.rip);
}

void reset_rip(debugger_t *dbg) {
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, dbg->pid, 0, &regs)) {
        ERRQUIT(1, "get regs failed.");
    }

    break_pt_t *current = dbg->bp_find_by_addr(dbg, regs.rip - 1);
    if (current != NULL) {
        regs.rip -= 1;
        if (ptrace(PTRACE_SETREGS, dbg->pid, 0, &regs) != 0) {
            ERRRET("set regs failed.");
        }

        fprintf(stdout, "** breakpoint @ ");
        dbg->disasm(dbg, &current->code, sizeof(current->code), current->addr, 1);
    }
}

int disasm(debugger_t *dbg, void* code, int length, unsigned long long addr, int n) {
    int total = 0;
    static csh cshandle = 0;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &cshandle) != CS_ERR_OK) {
        ERRMSG("capstone failed.");
        return 0;
    }

    cs_insn *insn;
    int count = cs_disasm(cshandle, (uint8_t*)code, length, addr, 0, &insn);

    if (count == 0) {
        ERRMSG("disasm failed.");
        return 0;
    }
    if (count > n) {
        count = n;
    }

    for (int i = 0; i < count; i++) {
        fprintf(stdout, "%16lx:", insn[i].address);
        for (int j = 0; j < insn[i].size; j++) {
            fprintf(stdout, " %02x", insn[i].bytes[j]);
        }
        for (int j = 0; j < 16 - insn[i].size; j++) {
            fprintf(stdout, "   ");
        }
        fprintf(stdout, "%-8s %s\n", insn[i].mnemonic, insn[i].op_str);
        total += insn[i].size;
    }

    cs_free(insn, count);
    cs_close(&cshandle);

    return total;
}
