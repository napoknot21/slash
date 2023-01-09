#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "slasherrno.h"
#include "signal.h"

volatile sig_atomic_t interrupt_state = 0;
volatile sig_atomic_t sigterm_received = 0;

void set_signalint () 
{ 
    interrupt_state = 1;
}

void set_sigterm () 
{
    sigterm_received = 1;
}

int get_signalint () 
{
    return interrupt_state;
}

int get_sigterm () 
{
    return sigterm_received;
}

void set_signal_handler()
{
    struct sigaction sg = {0};
    sg.sa_flags = 0;
    sigemptyset(&sg.sa_mask);

    //ignore SIGINT
    sg.sa_handler = &set_signalint;
    sigaction (SIGINT, &sg, NULL);

    //Ignore SIGTERM
    sg.sa_handler = &set_sigterm;
    sigaction (SIGTERM, &sg, NULL);

}

