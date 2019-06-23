#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>

#include "debugger.h"


BUILDIN_REGESTER(dump, x) {
    if (argc < 2 && dbg->dump == 0) {
        ERRRET("no address specified.");
    }
    if (dbg->stat != RUNNING) {
        ERRRET("the program is not being run.");
    }

    unsigned long long target = (argc < 2 ? dbg->dump : strtoll(argv[1], NULL, 0));
    for (int i = 0; i < 10; i += 2, target += 16) {
        unsigned char text[32] = {};

        fprintf(stdout, "\t%llx:", target);

        for (int k = 0; k < 2; k++) {
            int offset = k * 8;
            unsigned long long code = ptrace(PTRACE_PEEKTEXT, dbg->pid,
                                             target + offset, 0);
            for (int j = 0; j < 8; j++) {
                unsigned char c = code & 0xff;
                text[j + offset] = (isprint(c) ? c : '.');

                fprintf(stdout, " %02x", c);
                code >>= 8;
            }
        }

        fprintf(stdout, "  |%s|\n", text);
    }
    dbg->dump = target;
}
