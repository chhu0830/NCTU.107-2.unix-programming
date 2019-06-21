#include "debugger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>

#include "elftool.h"


void exec(struct DEBUGGER *dbg, int argc, const char **argv);


struct BUILDIN_FUNC *list;


struct DEBUGGER* init_debugger() {
    struct DEBUGGER *dbg = malloc(sizeof(struct DEBUGGER));

    dbg->status = INIT;
    dbg->eh = NULL;
    dbg->text = NULL;

    dbg->exec = exec;

    return dbg;
}

void free_debugger(struct DEBUGGER *dbg) {
    if (dbg->status == INIT) return;
    if (dbg->status == RUNNING) {
        if (kill(dbg->pid, SIGKILL) == 0) {
            waitpid(dbg->pid, NULL, 0);
        }
    }

    elf_close(dbg->eh);

    dbg->status = INIT;
    dbg->eh = NULL;
    dbg->text = NULL;
}

void exec(struct DEBUGGER *dbg, int argc, const char **argv) {
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
