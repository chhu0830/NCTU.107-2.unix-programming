#include <stdio.h>

#include <sys/ptrace.h>
#include <capstone/capstone.h>

#include "debugger.h"
#include "elftool.h"


BUILDIN_REGESTER(disasm, d) {
    if (argc < 2) {
        ERRRET("no addr is given.");
    }
    if (dbg->stat != LOADED && dbg->stat != RUNNING) {
        ERRRET("no executable file specified.  load first.");
    }


    int lcode = 160;
    unsigned char code[256];
    unsigned long long addr = strtoll(argv[1], NULL, 0);

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
    

    static csh cshandle = 0;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &cshandle) != CS_ERR_OK) {
        ERRRET("capstone failed.");
    }

    cs_insn *insn;
    int count = cs_disasm(cshandle, (uint8_t*)code, lcode, addr, 0, &insn);

    if (count == 0) {
        ERRRET("disasm failed.");
    }
    if (count > 10) {
        count = 10;
    }

    for (int i = 0; i < count; i++) {
        fprintf(stdout, "\t%lx:", insn[i].address);
        for (int j = 0; j < insn[i].size; j++) {
            fprintf(stdout, " %02x", insn[i].bytes[j]);
        }
        fprintf(stdout, "%8s %s\n", insn[i].mnemonic, insn[i].op_str);
    }

    cs_close(&cshandle);
}
