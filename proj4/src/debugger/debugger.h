#define MAX_PROG_NAME_LEN 128
#define INIT 0
#define LOADED 1
#define RUNNING 2

/*******************
 * Macro Functions *
 *******************/
#define BUILDIN_REGESTER(fname, fnick)                                        \
    void cmd_##fname(debugger_t *dbg, int argc, const char **argv);           \
    extern struct BUILDIN_FUNC *list;                                         \
    __attribute__((constructor())) static void regester() {                   \
        static struct BUILDIN_FUNC node = {                                   \
            .name = #fname,                                                   \
            .nick = #fnick,                                                   \
            .exec = cmd_##fname,                                              \
            .next = NULL                                                      \
        };                                                                    \
                                                                              \
        if (list == NULL) {                                                   \
            list = &node;                                                     \
        } else {                                                              \
            node.next = list;                                                 \
            list = &node;                                                     \
        }                                                                     \
    }                                                                         \
    void cmd_##fname(debugger_t *dbg, int argc, const char **argv)            \

#define ERRMSG(fmt, ...)                                                      \
    fprintf(stderr, "** " fmt "\n", ##__VA_ARGS__);

#define ERRRET(fmt, ...)                                                      \
    fprintf(stderr, "** " fmt "\n", ##__VA_ARGS__);                           \
    return

#define ERRQUIT(errno, fmt, ...)                                              \
    fprintf(stderr, "** " fmt "\n", ##__VA_ARGS__);                           \
    exit(errno)


/**************
 * Structures *
 **************/
typedef struct break_pt_s {
    int id;
    unsigned long long addr, code;
    struct break_pt_s *next;
} break_pt_t;

typedef struct debugger_s {
    char program[MAX_PROG_NAME_LEN];
    int stat, status, pid, bpi;
    unsigned long long base, dump;
    struct elf_handle_s *eh;
    struct elf_phdr_s *ptext;
    struct elf_shdr_s *stext;
    break_pt_t *bp;

    void (*exec)(struct debugger_s *dbg, int argc, const char **argv);
    unsigned long long (*bp_patch)(struct debugger_s *dbg, unsigned long long target);
    void (*bp_unpatch)(struct debugger_s *dbg, break_pt_t *break_pt);
    break_pt_t* (*bp_find_by_addr)(struct debugger_s *dbg, unsigned long long addr);
    break_pt_t* (*bp_check)(struct debugger_s *dbg);
    void (*reset_rip)(struct debugger_s *dbg);
    void (*disasm)(struct debugger_s *dbg, void* code, int length, unsigned long long addr, int n);
} debugger_t;

struct BUILDIN_FUNC {
    char *name, *nick;
    void (*exec)(debugger_t *dbg, int argc, const char **argv);
    struct BUILDIN_FUNC *next;
};


/*************************
 * Function Declarations *
 *************************/
debugger_t* init_debugger();
void free_debugger(debugger_t *dbg);
void copy_argv(char *first, int argc, char **new_argv, const char **old_argv);
