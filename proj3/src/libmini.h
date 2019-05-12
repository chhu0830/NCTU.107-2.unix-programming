#ifndef __LIBMINI_H__
#define __LIBMINI_H__

/***********************
 * Constant Definition *
 ***********************/
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define PERRMSG_MIN 0
#define PERRMSG_MAX 34
#define PERRMSG_UNKNWON "Unknown"

/* from /usr/include/asm-generic/errno-base.h */
#define EPERM        1  /* Operation not permitted */
#define ENOENT       2  /* No such file or directory */
#define ESRCH        3  /* No such process */
#define EINTR        4  /* Interrupted system call */
#define EIO          5  /* I/O error */
#define ENXIO        6  /* No such device or address */
#define E2BIG        7  /* Argument list too long */
#define ENOEXEC      8  /* Exec format error */
#define EBADF        9  /* Bad file number */
#define ECHILD      10  /* No child processes */
#define EAGAIN      11  /* Try again */
#define ENOMEM      12  /* Out of memory */
#define EACCES      13  /* Permission denied */
#define EFAULT      14  /* Bad address */
#define ENOTBLK     15  /* Block device required */
#define EBUSY       16  /* Device or resource busy */
#define EEXIST      17  /* File exists */
#define EXDEV       18  /* Cross-device link */
#define ENODEV      19  /* No such device */
#define ENOTDIR     20  /* Not a directory */
#define EISDIR      21  /* Is a directory */
#define EINVAL      22  /* Invalid argument */
#define ENFILE      23  /* File table overflow */
#define EMFILE      24  /* Too many open files */
#define ENOTTY      25  /* Not a typewriter */
#define ETXTBSY     26  /* Text file busy */
#define EFBIG       27  /* File too large */
#define ENOSPC      28  /* No space left on device */
#define ESPIPE      29  /* Illegal seek */
#define EROFS       30  /* Read-only file system */
#define EMLINK      31  /* Too many links */
#define EPIPE       32  /* Broken pipe */
#define EDOM        33  /* Math argument out of domain of func */
#define ERANGE      34  /* Math result not representable */

#define NSIG        29  /* number of signals used */
/* from /usr/include/x86_64-linux-gnu/asm/signal.h */
#define SIGHUP       1
#define SIGINT       2
#define SIGQUIT      3
#define SIGILL       4
#define SIGTRAP      5
#define SIGABRT      6
#define SIGIOT       6
#define SIGBUS       7
#define SIGFPE       8
#define SIGKILL      9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGSTKFLT   16
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGPOLL     SIGIO

/* from /usr/include/x86_64-linux-gnu/bits/sigaction.h */
#define SA_NOCLDSTOP  1     /* Don't send SIGCHLD when children stop.  */
#define SA_NOCLDWAIT  2     /* Don't create zombie on child death.  */
#define SA_SIGINFO    4     /* Invoke signal-catching function with
                                       three arguments instead of one.  */
#define SA_ONSTACK   0x08000000    /* Use signal stack by using `sa_restorer'. */
#define SA_RESTART   0x10000000    /* Restart syscall on signal return.  */
#define SA_INTERRUPT 0x20000000    /* Historical no-op.  */
#define SA_NODEFER   0x40000000    /* Don't automatically block the signal when
                                       its handler is being executed.  */
#define SA_RESETHAND 0x80000000    /* Reset to SIG_DFL on entry to handler.  */

#define SA_RESTORER  0x04000000   

#define SIG_BLOCK     0      /* Block signals.  */
#define SIG_UNBLOCK   1      /* Unblock signals.  */
#define SIG_SETMASK   2      /* Set the set of blocked signals.  */

#define SIG_DFL ((__sighandler_t)0)  /* default signal handling */
#define SIG_IGN ((__sighandler_t)1)   /* ignore signal */
#define SIG_ERR ((__sighandler_t)-1) /* error return from signal */
/*******************
 * Type Definition *
 *******************/
#define NULL ((void*) 0)

typedef unsigned long long size_t;
typedef long long ssize_t;
typedef unsigned long sigset_t;
typedef void __signalfn_t(int);
typedef __signalfn_t *__sighandler_t;
typedef void __restorefn_t(void);
typedef __restorefn_t *__sigrestore_t;
typedef struct jmp_buf_s {
    long long reg[8];
    sigset_t mask;
} jmp_buf[1]; 

struct timespec {
    long    tv_sec;     /* seconds */
    long    tv_nsec;    /* nanoseconds */
};

struct sigaction {
    __sighandler_t sa_handler;
    unsigned long sa_flags;
    __sigrestore_t sa_restorer;
    sigset_t sa_mask;
};

extern long errno;

/************************
 * Function Declaration *
 ************************/
long sys_write(unsigned int fd, const char *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

void sigrestore();
long sys_rt_sigaction(int sig, const struct sigaction *act, struct sigaction *oact, size_t sigsetsize);
int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact);
__sighandler_t signal(int signum, __sighandler_t handler);

long sys_rt_sigprocmask(int how, const sigset_t *nset, sigset_t *oset, size_t sigsetsize);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

long sys_pause();
int pause();

long sys_nanosleep(struct timespec *rqtp, struct timespec *rmtp);
unsigned int sleep(unsigned int seconds);

long sys_alarm(unsigned int seconds);
unsigned int alarm(unsigned int seconds);

long sys_exit(int error_code) __attribute__ ((noreturn));
void exit(int error_code);

long sys_rt_sigpending(sigset_t *set, size_t sigsetsize);
int sigpending(sigset_t *set);

size_t strlen(const char *s);
void perror(const char *prefix);

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

/* Compute mask for signal SIG.  */
# define sigmask(sig) ((int)(1u << ((sig) - 1)))

/* Make sure there is nothing inside the signal set. */
#  define sigemptyset(set) (*(set) = 0)

/* Initialize the signal set to hold all signals. */
#  define sigfillset(set) (*set) = sigmask (NSIG) - 1

/* Add SIG to the contents of SET. */
#  define sigaddset(set, sig) *(set) |= sigmask (sig)

/* Delete SIG from signal set SET. */
#  define sigdelset(set, sig) *(set) &= ~sigmask (sig)

/* Is SIG a member of the signal set SET? */
#  define sigismember(set, sig) ((*(set) & sigmask (sig)) != 0)

#endif /* __LIBMINI_H__ */
