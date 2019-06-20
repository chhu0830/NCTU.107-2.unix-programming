#include "debugger.h"

#include <stdlib.h>
#include <string.h>


void exec(struct DEBUGGER *dbg, int argc, const char **argv);
struct BUILDIN_FUNC* BUILDIN_FIND(const char *name);


struct BUILDIN_FUNC *list;


struct DEBUGGER* init_debugger() {
    struct DEBUGGER *dbg = malloc(sizeof(struct DEBUGGER));
    dbg->exec = exec;

    return dbg;
}

void exec(struct DEBUGGER *dbg, int argc, const char **argv) {
    struct BUILDIN_FUNC *node = BUILDIN_FIND(argv[0]);
    
    if (node != NULL) node->exec(dbg, argc, argv);
    else {
        //TODO: Unknown command
    }
}

struct BUILDIN_FUNC* BUILDIN_FIND(const char *name) {
    struct BUILDIN_FUNC *current = list;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) return current;
        current = current->next;
    }

    return NULL;
}
