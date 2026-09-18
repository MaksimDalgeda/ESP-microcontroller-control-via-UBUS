#ifndef SERVICE_H
#define SERVICE_H

#include <stdio.h>
#include <syslog.h>
#include "error.h"

ESP_Error service_init(void);

ESP_Error service_find_devices(void);
ESP_Error service_update_devices(void);
ESP_Error service_wait_for_device_change(void);
ESP_Error service_get_devices(void);

ESP_Error service_clear_data(void);

#endif