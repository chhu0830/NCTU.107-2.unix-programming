#include "debugger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ptrace.h>
#include <sys/wait.h>

#include "elftool.h"


void exec(debugger_t *dbg, int argc, const char **argv);
void bp_add(debugger_t *dbg, unsigned long long target);
unsigned long long bp_patch(debugger_t *dbg, unsigned long long target);
break_pt_t* bp_find_by_addr(debugger_t *dbg, unsigned long long addr);


struct BUILDIN_FUNC *list;


debugger_t* init_debugger() {
    debugger_t *dbg = malloc(sizeof(debugger_t));

    dbg->stat = INIT;
    dbg->bpi = 0;
    dbg->base = 0;
    dbg->eh = NULL;
    dbg->text = NULL;
    dbg->bp = NULL;

    dbg->exec = exec;
    dbg->bp_add = bp_add;
    dbg->bp_patch = bp_patch;
    dbg->bp_find_by_addr = bp_find_by_addr;

    return dbg;
}

void free_debugger(debugger_t *dbg) {
    if (dbg->stat == INIT) return;
    if (dbg->stat == RUNNING) {
        if (kill(dbg->pid, SIGKILL) == 0) {
            waitpid(dbg->pid, NULL, 0);
        }
    }

    break_pt_t *current = dbg->bp, *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    elf_close(dbg->eh);

    dbg->stat = INIT;
    dbg->bpi = 0;
    dbg->base = 0;
    dbg->eh = NULL;
    dbg->text = NULL;
    dbg->bp = NULL;
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

void bp_add(debugger_t *dbg, unsigned long long target) {
    break_pt_t *bp = malloc(sizeof(break_pt_t));
    bp->id = dbg->bpi++;
    bp->addr = target;
    bp->code = (dbg->stat == RUNNING ? dbg->bp_patch(dbg, target) : 0);
    bp->next = NULL;

    if (dbg->bp == NULL) {
        dbg->bp = bp;
    } else {
        bp->next = dbg->bp;
        dbg->bp = bp;
    }
}

unsigned long long bp_patch(debugger_t *dbg, unsigned long long target) {
    unsigned long long code = ptrace(PTRACE_PEEKTEXT, dbg->pid, target + dbg->base, 0);
    if (ptrace(PTRACE_POKETEXT, dbg->pid, target,
               (code & 0xffffffffffffff00) | 0xcc) != 0) {
        ERRMSG("break failed.");
        code = 0;
    }
    return code;
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
