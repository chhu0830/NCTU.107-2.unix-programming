#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>

#include "debugger.h"
#include "elftool.h"


BUILDIN_REGESTER(break, b) {
    if (argc < 2) {
        ERRRET("no address specified.");
    }

    if (dbg->stat != LOADED && dbg->stat != RUNNING) {
        ERRRET("no executable file specified.  load first.");
    }

    unsigned long long target = strtoll(argv[1], NULL, 0);
    unsigned long long begin = dbg->base + dbg->text->addr;
    unsigned long long end = dbg->base + dbg->text->addr + dbg->text->size;

    if (target < begin || target > end) {
        ERRRET("address out of range.");
    }
    if (dbg->bp_find_by_addr(dbg, target) != NULL) {
        ERRRET("breakpoint already set @ %llx.", target);
    }

    break_pt_t *bp = malloc(sizeof(break_pt_t));
    bp->id = dbg->bpi++;
    bp->addr = target;
    bp->code = (dbg->stat == RUNNING ? dbg->bp_patch(dbg, target) : 0);
    bp->next = NULL;

    if (dbg->bp == NULL) {
        dbg->bp = bp;
    } else {
        break_pt_t *current = dbg->bp;
        while (current->next != NULL) current = current->next;
        current->next = bp;
    }
}
