#define _POSIX_C_SOURCE 200809L //helps VS Code recognize POSIX APIS
#include <signal.h>
#include <stdio.h>

#include "signal_handler.h"

volatile sig_atomic_t stop = 0;

void sigint_handler(int signal){
    if (signal == SIGINT || signal == SIGTERM || signal == SIGQUIT){
        stop = 1;
    }
}

void set_signal_action(void){

    struct sigaction sa = {0};

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}