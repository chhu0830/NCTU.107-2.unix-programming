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
    for(tab = dbg->eh->strtab; tab != NULL; tab = tab->next) {
		if(tab->id == dbg->eh->shstrndx) break;
	}
	if(tab == NULL) {
        free_debugger(dbg);
        ERRRET("section header string table not found.");
	}

	for(int i = 0; i < dbg->eh->shnum; i++) {
        // FIXME: Don't use .text to find code segment
        if (strcmp(&tab->data[dbg->eh->shdr[i].name], ".text") == 0) {
            dbg->text = &dbg->eh->shdr[i];
            break;
        }
    }

    if (dbg->text != NULL) {
        strcpy(dbg->program, argv[1]);
        dbg->stat = LOADED;

        ERRRET("program '%s' loaded."
               " entry point %#lx, vaddr %#llx, offset %#llx, size %#llx",
               argv[1], dbg->eh->entrypoint,
               dbg->text->addr, dbg->text->offset, dbg->text->size);
    } else {
        free_debugger(dbg);
        ERRRET("no text segment.");
    }
}
