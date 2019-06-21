#define MAX_PROG_NAME_LEN 128
#define INIT 0
#define LOADED 1
#define RUNNING 2

/*******************
 * Macro Functions *
 *******************/
#define BUILDIN_REGESTER(fname, fnick)                                        \
    void fname(struct DEBUGGER *dbg, int argc, const char **argv);            \
    extern struct BUILDIN_FUNC *list;                                         \
    __attribute__((constructor())) static void regester() {                   \
        static struct BUILDIN_FUNC node = {                                   \
            .name = #fname,                                                   \
            .nick = #fnick,                                                   \
            .exec = fname,                                                    \
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
    void fname(struct DEBUGGER *dbg, int argc, const char **argv)             \

#define ERRRET(fmt, ...)                                                      \
    fprintf(stderr, "** " fmt "\n", ##__VA_ARGS__);                           \
    return

#define ERRQUIT(errno, fmt, ...)                                              \
    fprintf(stderr, "** " fmt "\n", ##__VA_ARGS__);                           \
    exit(errno)


/**************
 * Structures *
 **************/
struct elf_handle_s;

struct DEBUGGER {
    char program[MAX_PROG_NAME_LEN];
    int status, pid;
    struct elf_handle_s *eh;
    struct elf_shdr_s *text;

    void (*exec)(struct DEBUGGER *dbg, int argc, const char **argv);
};

struct BUILDIN_FUNC {
    char *name, *nick;
    void (*exec)(struct DEBUGGER *dbg, int argc, const char **argv);
    struct BUILDIN_FUNC *next;
};


/*************************
 * Function Declarations *
 *************************/
struct DEBUGGER* init_debugger();
void free_debugger(struct DEBUGGER *dbg);
