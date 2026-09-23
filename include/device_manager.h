#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <stdint.h>

#include "error.h"

typedef struct
{
    char *port;
    uint16_t vid;
    uint16_t pid;
} Device;

ESP_Error device_manager_init(void);

ESP_Error device_manager_find_devices(Device **devices, uint32_t *count);
ESP_Error device_manager_update_devices(void);
ESP_Error device_manager_wait_for_change(void); 
ESP_Error device_manager_get_devices(Device **devices_out, uint32_t *count_out);

void device_manager_free_devices(Device *devices, uint32_t count);
ESP_Error device_manager_clear_data(void);

#endif