#include <stdio.h>
#include <string.h>

#include "debugger.h"
#include "elftool.h"

#define FORMAT "%016llx-%016llx %s %llx %s\n"


char* flags2perm(long long flags) {
    static char perm[8];
    sprintf(perm, "r%c%c",
            (flags & SHF_WRITE) == SHF_WRITE ? 'w' : '-',
            (flags & SHF_ALLOC) == SHF_ALLOC ? 'x' : '-');
    return perm;
}

BUILDIN_REGESTER(vmmap,) {
    if (dbg->status == LOADED) {
        fprintf(stdout, FORMAT,
                dbg->text->addr,
                dbg->text->addr + dbg->text->size,
                flags2perm(dbg->text->flags),
                dbg->text->offset,
                dbg->program);
    } else if (dbg->status == RUNNING){
        char filename[128];
        sprintf(filename, "/proc/%d/maps", dbg->pid);

        long long start, end, offset;
        char perm[8], program[128], dummy[128];
        FILE *maps = fopen(filename, "r");
        while (fscanf(maps, "%llx-%llx %s %llx %s %s %s",
               &start, &end, perm, &offset, dummy, dummy, program) != EOF) {
            perm[3] = '\0';
            fprintf(stdout, FORMAT, start, end, perm, offset, program);
        }
    } else {
        ERRRET("no executable file specified.  load first.");
    }
}
