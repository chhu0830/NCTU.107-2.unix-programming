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

    dbg->bp_add(dbg, target - dbg->base);
}
