#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "signal.h"

#define SIG_TABLE_SIZE 36

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


int char_to_signal (const char *str)
{
    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        struct signal s = signal_table[i];
        if (strcmp(s.name, str) == 0) return s.signal;
    }
    return -1;
}


const char *signal_get_name (int signal) 
{
    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        struct signal s = signal_table[i];
        if (s.signal == signal) return s.name;
    }
    return NULL;
}


const char *signal_get_descr (int signal)
{
    for (int i = 0; i < SIG_TABLE_SIZE; i++) {
        struct signal s = signal_table[i];
        if (s.signal == signal) return s.desc;
    }
    return NULL;
}

/*
static void signal_handler(int signal) 
{

    //const int tmp_errno = errno; //to preserve the errno

    switch (signal) {
        
        case SIGWINCH :
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
*/

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