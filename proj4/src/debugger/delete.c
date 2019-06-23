#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>

#include "debugger.h"


BUILDIN_REGESTER(delete,) {
    if (argc < 2) {
        ERRRET("no break point specified.");
    }
    
    int n = atoi(argv[1]);
    break_pt_t *current = NULL, *ptr = dbg->bp;

    if (ptr == NULL) {
        ERRRET("breakpoint %d does not exist.", n);
    }
    if (ptr->id == n) {
        dbg->bp = ptr->next;
        current = ptr;
    } else {
        current = ptr->next;
        while (current != NULL) {
            if (current->id == n) {
                ptr->next = current->next;
                break;
            }
            ptr = ptr->next;
            current = ptr->next;
        }
    }

    if (current != NULL) {
        dbg->bp_unpatch(dbg, current);
        free(current);
        ERRRET("breakpoint %d deleted.", n);
    } else {
        ERRRET("breakpoint %d does not exist.", n);
    }
    
}
