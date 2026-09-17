#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <stdint.h>

#include "error.h"

typedef struct
{
    char *port;
    int vid;
    int pid;
} Device;

ESP_Error device_manager_find_devices(Device **devices, uint32_t *count);

void device_manager_free_devices(Device *devices, uint32_t count);

#endif