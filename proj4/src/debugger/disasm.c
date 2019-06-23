#include <stdio.h>

#include <capstone/capstone.h>
#include <sys/ptrace.h>

#include "debugger.h"
#include "elftool.h"


BUILDIN_REGESTER(disasm, d) {
    if (argc < 2 && dbg->ldisasm == 0) {
        ERRRET("no addr is given.");
    }
    if (dbg->stat != LOADED && dbg->stat != RUNNING) {
        ERRRET("no executable file specified.  load first.");
    }


    int lcode = 160;
    unsigned char code[256];
    unsigned long long addr = (argc < 2 ? dbg->ldisasm : strtoll(argv[1], NULL, 0));

    if (dbg->stat == LOADED) {
        int offset = addr - dbg->ptext->vaddr;

        if (offset < dbg->stext->offset) {
            ERRRET("address out of range.");
        }

        if (offset + lcode > dbg->stext->offset + dbg->stext->size) {
            lcode = dbg->stext->offset + dbg->stext->size - offset;
        }

        FILE *file = fopen(dbg->program, "r");
        fseek(file, offset, SEEK_SET);
        fread(code, 1, lcode, file);
        fclose(file);
    } else if (dbg->stat == RUNNING) {
        unsigned long long *ptr;
        for (int i = 0; i < lcode; i += 8) {
            ptr = (unsigned long long*)(code + i);
            *ptr = ptrace(PTRACE_PEEKTEXT, dbg->pid, addr + i, 0);
        }

        for (int i = 0; i < lcode; i++) {
            if (code[i] == 0xcc) {
                break_pt_t *bp = dbg->bp_find_by_addr(dbg, addr + i);
                code[i] = bp->code & 0xff;
            }
        }
    }
    
    dbg->ldisasm = addr + dbg->disasm(dbg, code, lcode, addr, 10);
}
