#ifndef SERVICE_H
#define SERVICE_H

#include <stdio.h>

#include "error.h"

ESP_Error service_init(void);
ESP_Error service_find_devices(void);

#endif