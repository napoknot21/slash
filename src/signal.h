#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

struct signal {
    // signal id
    int signal;
    // signal name
    const char *name;
    // signal description
    const char *desc;
};

/**
 * @brief Check and search the signal id in the signal table by name
 * @param str string signal name 
 * @return the signal's id number or -1 if not found
 */
int char_to_signal (const char *str);

/**
 * @brief Check and search the signal name in the signal table by signal id
 * @param signal the signal's id number
 * @return the signal's name or NULL if not found
*/
const char *signal_get_name (int signal);

/**
 * @brief Check and search the signal description in the signal table by signal
 * @param signal the signal's id number
 * @return the signal's description or NULL if not found
 */
const char *signal_get_descr (int signal);

//void signal *make_signal (int, const char *name, const char *desc);
//void free_signal (struct signal *sg);

/**
 * @brief The 'main' signal handler function
 * @param sig the signal's id number
 */
static void signal_handler(int sig);

//temp functions
void handler_sigstp (int signal);

void handler_sigkill (int signal);


#endif
