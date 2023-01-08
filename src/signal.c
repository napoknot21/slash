#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "slasherrno.h"
#include "signal.h"

/* Non-zero after SIGINT. */
volatile sig_atomic_t interrupt_state = 0;

/* Non-zero after SIGTERM */
volatile sig_atomic_t sigterm_received = 0;

/* When non-zero, we throw_to_top_level (). */
int interrupt_immediately = 0;

/* When non-zero, we call the terminating signal handler immediately. */
int terminate_immediately = 0;

static const struct signal signal_table[] = {
#ifdef SIGHUP
    {SIGHUP, "SIGHUP", "Terminal hung up"},
#endif
#ifdef SIGINT
    {SIGINT, "SIGINT", "Quit request from job control (^C)"},
#endif
#ifdef SIGQUIT
    {SIGQUIT, "SIGQUIT", "Quit request from job control with core dump (^\\)"},
#endif
#ifdef SIGILL
    {SIGILL, "SIGILL", "Illegal instruction"},
#endif
#ifdef SIGTRAP
    {SIGTRAP, "SIGTRAP", "Trace or breakpoint trap"},
#endif
#ifdef SIGABRT
    {SIGABRT, "SIGABRT", "Abort"},
#endif
#ifdef SIGBUS
    {SIGBUS, "SIGBUS", "Misaligned address error"},
#endif
#ifdef SIGFPE
    {SIGFPE, "SIGFPE", "Floating point exception"},
#endif
#ifdef SIGKILL
    {SIGKILL, "SIGKILL", "Forced quit"},
#endif
#ifdef SIGUSR1
    {SIGUSR1, "SIGUSR1", "User defined signal 1"},
#endif
#ifdef SIGUSR2
    {SIGUSR2, "SIGUSR2", "User defined signal 2"},
#endif
#ifdef SIGSEGV
    {SIGSEGV, "SIGSEGV", "Address boundary error"},
#endif
#ifdef SIGPIPE
    {SIGPIPE, "SIGPIPE", "Broken pipe"},
#endif
#ifdef SIGALRM
    {SIGALRM, "SIGALRM", "Timer expired"},
#endif
#ifdef SIGTERM
    {SIGTERM, "SIGTERM", "Polite quit request"},
#endif
#ifdef SIGCHLD
    {SIGCHLD, "SIGCHLD", "Child process status changed"},
#endif
#ifdef SIGCONT
    {SIGCONT, "SIGCONT", "Continue previously stopped process"},
#endif
#ifdef SIGSTOP
    {SIGSTOP, "SIGSTOP", "Forced stop"},
#endif
#ifdef SIGTSTP
    {SIGTSTP, "SIGTSTP", "Stop request from job control (^Z)"},
#endif
#ifdef SIGTTIN
    {SIGTTIN, "SIGTTIN", "Stop from terminal input"},
#endif
#ifdef SIGTTOU
    {SIGTTOU, "SIGTTOU", "Stop from terminal output"},
#endif
#ifdef SIGURG
    {SIGURG, "SIGURG", "Urgent socket condition"},
#endif
#ifdef SIGXCPU
    {SIGXCPU, "SIGXCPU", "CPU time limit exceeded"},
#endif
#ifdef SIGXFSZ
    {SIGXFSZ, "SIGXFSZ", "File size limit exceeded"},
#endif
#ifdef SIGVTALRM
    {SIGVTALRM, "SIGVTALRM", "Virtual timer expired"},
#endif
#ifdef SIGPROF
    {SIGPROF, "SIGPROF", "Profiling timer expired"},
#endif
#ifdef SIGWINCH
    {SIGWINCH, "SIGWINCH", "Window size change"},
#endif
#ifdef SIGWIND
    {SIGWIND, "SIGWIND", "Window size change"},
#endif
#ifdef SIGIO
    {SIGIO, "SIGIO", "I/O on asynchronous file descriptor is possible"},
#endif
#ifdef SIGPWR
    {SIGPWR, "SIGPWR", "Power failure"},
#endif
#ifdef SIGSYS
    {SIGSYS, "SIGSYS", "Bad system call"},
#endif
#ifdef SIGINFO
    {SIGINFO, "SIGINFO", "Information request"},
#endif
#ifdef SIGSTKFLT
    {SIGSTKFLT, "SISTKFLT", "Stack fault"},
#endif
#ifdef SIGEMT
    {SIGEMT, "SIGEMT", "Emulator trap"},
#endif
#ifdef SIGIOT
    {SIGIOT, "SIGIOT", "Abort (Alias for SIGABRT)"},
#endif
#ifdef SIGUNUSED
    {SIGUNUSED, "SIGUNUSED", "Unused signal"},
#endif
};

#define SIG_TABLE_SIZE (sizeof (signal_table) / sizeof (struct signal))

#define XSIGNAL(x) (signal_table[x].signal)
#define XNAME(x) (signal_table[x].name)
#define XDESC(x) (signal_table[x].desc)


int char_to_signal (const char *str)
{
    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        if (strcmp(XNAME(i), str) == 0) return XSIGNAL(i);
    }
    return -1;
}


const char *signal_get_name (int signal) 
{
    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        if (XSIGNAL(i) == signal) return XNAME(i);
    }
    return NULL;
}


const char *signal_get_descr (int signal)
{
    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        if (XSIGNAL(i) == signal) return XDESC(i);
    }
    return NULL;
}

//static const pid_t main_pid = getpid();


static void signal_handler(int signal) 
{

    //const int tmp_errno = errno; //to preserve the errno

    switch (signal) {
        
        case SIGWINCH :
            //TODO finish handlers
            break;
        
        case SIGHUP :
            break;

        case SIGTERM :
            break;

        case SIGINT :
            break;

        case SIGCHLD :
            break;

        case SIGALRM :
            break;

    }
    
}


void reset_signal_handler (void) 
{
    struct sigaction sg;
    sigemptyset(&sg.sa_mask);
    sg.sa_flags = 0;
    sg.sa_handler = SIG_DFL;

    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        struct signal s = signal_table[i];
        if (s.signal == SIGHUP) {
            struct sigaction old_sg;
            sigaction(SIGHUP, NULL, &old_sg);
            if (old_sg.sa_handler == SIG_IGN) continue;
        }
        sigaction (s.signal, &sg, NULL);
    }
}


void set_signal_handler () 
{
    struct sigaction sg;
    sg.sa_flags = 0;
    sigemptyset(&sg.sa_mask);

    //ignore SIGINT
    sg.sa_sigaction = NULL;
    sg.sa_handler = SIG_IGN;
    sigaction (SIGINT, &sg, NULL);

    //Ignore SIGTERM
    sg.sa_handler = SIG_DFL;
    sigaction (SIGTERM, &sg, NULL);

}


void signal_handle (int signal) 
{
    struct sigaction sg;

    if ((signal == SIGINT)  || (signal == SIGQUIT) || (signal == SIGTSTP) || (signal == SIGTTIN ) ||
        (signal == SIGTTOU) || (signal == SIGCHLD))
        return;

    sg.sa_flags = 0;
    sigemptyset (&sg.sa_mask);
    sg.sa_flags = SA_SIGINFO;
    sg.sa_sigaction = &signal_handler;
    sigaction (signal, &sg, NULL);
}

//static pid_t main_pid = getpid(); //Store the "main" pid

/*
void handler_sigkill (int signal) {
    sigaction(SIGKILL, &handler_sigkill, NULL);
}

int sig_stop (int out, int err, pid_t pid) 
{
    struct sigaction sg;
    sg.sa_handler = &handler_sigkill;
    sg.sa_flags = SA_RESTART;
    
    int st = sigaction(SIGSTOP, &sg, NULL);
    
    if (st != 0) {
        write (err, "SIGKILL problem !\n", 18);
        return -1;
    }
    
    write(out, "[SIG]\n", 6);
    return 0;
}
*/


/*
int sig_kill (pid_t pid)
{
    //struct sigaction 
}
*/