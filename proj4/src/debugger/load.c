#include <stdio.h>
#include <string.h>

#include "debugger.h"
#include "elftool.h"


BUILDIN_REGESTER(load,) {
    if (argc < 2) {
        ERRRET("no executable file specified.");
    }

    if (dbg->stat == LOADED || dbg->stat == RUNNING) {
        free_debugger(dbg);
    }

    elf_init();
    if ((dbg->eh = elf_open(argv[1])) == NULL) {
        ERRRET("no such file or directory.");
    }
    if (elf_load_all(dbg->eh) < 0) {
        free_debugger(dbg);
        ERRRET("unable to load '%s'.", argv[1]);
    }

    elf_strtab_t *tab = NULL;
    for (tab = dbg->eh->strtab; tab != NULL; tab = tab->next) {
		if(tab->id == dbg->eh->shstrndx) break;
	}
	if (tab == NULL) {
        reset_debugger(dbg);
        ERRRET("section header string table not found.");
	}

    for (int i = 0; i < dbg->eh->phnum; i++) {
        if (dbg->eh->phdr[i].type == PT_LOAD
                && dbg->eh->phdr[i].flags & PF_R
                && dbg->eh->phdr[i].flags & PF_X) {
            dbg->ptext = &dbg->eh->phdr[i];
            break;
        }
    }

	for (int i = 0; i < dbg->eh->shnum; i++) {
        if (dbg->eh->shdr[i].type == SHT_PROGBITS
                && dbg->eh->shdr[i].flags & SHF_ALLOC
                && dbg->eh->shdr[i].flags & SHF_EXECINSTR
                && dbg->eh->entrypoint >= dbg->eh->shdr[i].addr
                && dbg->eh->entrypoint < dbg->eh->shdr[i].addr + dbg->eh->shdr[i].size) {
            dbg->stext = &dbg->eh->shdr[i];
            break;
        }
    }

    if (dbg->stext != NULL) {
        strcpy(dbg->program, argv[1]);
        dbg->stat = LOADED;

        ERRRET("program '%s' loaded."
               " entry point %#lx, vaddr %#llx, offset %#llx, size %#llx",
               argv[1], dbg->eh->entrypoint,
               dbg->stext->addr, dbg->stext->offset, dbg->stext->size);
    } else {
        reset_debugger(dbg);
        ERRRET("no text segment.");
    }
}
