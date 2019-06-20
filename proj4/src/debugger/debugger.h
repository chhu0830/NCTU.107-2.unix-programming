#define BUILDIN_REGESTER(fname)                                               \
    void fname(struct DEBUGGER *dbg, int argc, const char **argv);            \
    extern struct BUILDIN_FUNC *list;                                         \
    __attribute__((constructor())) static void regester() {                   \
        static struct BUILDIN_FUNC node = {                                   \
            .name = #fname,                                                   \
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

struct DEBUGGER {
    void (*exec)(struct DEBUGGER *dbg, int argc, const char **argv);
};

struct BUILDIN_FUNC {
    char *name;
    void (*exec)(struct DEBUGGER *dbg, int argc, const char **argv);
    struct BUILDIN_FUNC *next;
};

struct DEBUGGER* init_debugger();
