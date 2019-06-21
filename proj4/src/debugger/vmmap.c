#include <stdio.h>
#include <string.h>

#include "debugger.h"
#include "elftool.h"

#define FORMAT "%016llx-%016llx %s %-10llx %s\n"


char* flags2perm(long long flags) {
    static char perm[8];
    sprintf(perm, "r%c%c",
            (flags & SHF_WRITE) == SHF_WRITE ? 'w' : '-',
            (flags & SHF_ALLOC) == SHF_ALLOC ? 'x' : '-');
    return perm;
}

BUILDIN_REGESTER(vmmap, m) {
    if (dbg->stat == LOADED) {
        fprintf(stdout, FORMAT,
                dbg->text->addr,
                dbg->text->addr + dbg->text->size,
                flags2perm(dbg->text->flags),
                dbg->text->offset,
                dbg->program);
    } else if (dbg->stat == RUNNING){
        char filename[128];
        sprintf(filename, "/proc/%d/maps", dbg->pid);

        long long begin, end, offset;
        char perm[8], program[128];
        FILE *maps = fopen(filename, "r");
        while (fscanf(maps, "%llx-%llx %s %llx %*s %*s%*[ \t]",
               &begin, &end, perm, &offset) != EOF) {
            fgets(program, sizeof(program), maps);

            perm[3] = '\0';
            program[strcspn(program, "\r\n")] = '\0';

            fprintf(stdout, FORMAT, begin, end, perm, offset, program);
        }
        fclose(maps);
    } else {
        ERRRET("no executable file specified.  load first.");
    }
}
