#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H
#include <signal.h>

extern volatile sig_atomic_t stop;

void sigint_handler(int signal);
void set_signal_action(void);

#endif