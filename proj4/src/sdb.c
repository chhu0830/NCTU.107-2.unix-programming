#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordexp.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "debugger/debugger.h"

#define PROMPT "sdb> "


int main() {
    char *cmdline = NULL;
    struct DEBUGGER *dbg = init_debugger();

    // TODO: ./sdb <program>

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
