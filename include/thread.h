#ifndef THREADS_H
#define THREADS_H

#include "error.h"

ESP_Error threads_start(void);
ESP_Error threads_join(void);

#endif