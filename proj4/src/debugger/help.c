#include <stdio.h>

#include "debugger.h"


BUILDIN_REGESTER(help, h) {
    fprintf(stdout,
            "  break {instruction-address}: add a break point\n"
            "  cont: continue execution\n"
            "  delete {break-point-id}: remove a break point\n"
            "  disasm addr: disassemble instructions in a file or a memory region\n"
            "  dump addr [length]: dump memory content\n"
            "  exit: terminate the debugger\n"
            "  get reg: get a single value from a register\n"
            "  getregs: show registers\n"
            "  help: show this message\n"
            "  list: list break points\n"
            "  load {path/to/a/program}: load a program\n"
            "  run: run the program\n"
            "  vmmap: show memory layout\n"
            "  set reg val: get a single value to a register\n"
            "  si: step into instruction\n"
            "  start: start the program and stop at the first instruction\n");
}
