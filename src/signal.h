#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Non-zero after SIGINT. */
volatile sig_atomic_t interrupt_state = 0;

/* Non-zero after SIGTERM */
volatile sig_atomic_t sigterm_received = 0;

void set_signalint ();

void set_sigterm ();

int get_sigterm ();

void set_signal_handler();

#endif