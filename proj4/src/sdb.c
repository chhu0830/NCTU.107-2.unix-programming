#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordexp.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "debugger/debugger.h"

#define PROMPT "sdb> "


int main(int argc, const char **argv) {
    char *cmdline = NULL;
    debugger_t *dbg = init_debugger();

    if (argc >= 2) {
        char *arg[argc + 1];
        copy_argv("load", argc, arg, argv);
        dbg->exec(dbg, argc, (const char**)arg);
    }

    while ((cmdline = readline(PROMPT))) {
        if (cmdline[0] == '\0') continue;
        add_history(cmdline);

        wordexp_t p;
        wordexp(cmdline, &p, 0);

        dbg->exec(dbg, p.we_wordc, (const char **)p.we_wordv);

        wordfree(&p);
        free(cmdline);
    }

    free_debugger(dbg);
    free(dbg);

    return 0;
}
