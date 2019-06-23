#include <stdio.h>

#include "debugger.h"


BUILDIN_REGESTER(list, l) {
    break_pt_t *current = dbg->bp;

    while (current != NULL) {
        fprintf(stdout, "  %d:  %llx\n", current->id, current->addr);
        current = current->next;
    }
}
