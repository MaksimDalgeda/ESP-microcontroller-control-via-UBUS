#ifndef DEVICE_MANAGER_SERVICE_H
#define DEVICE_MANAGER_SERVICE_H

#include <stdio.h>
#include <syslog.h>
#include "error.h"

#include "error.h"

ESP_Error device_manager_service_init(void);

ESP_Error device_manager_service_find_devices(void);
ESP_Error device_manager_service_update_devices(void);
ESP_Error device_manager_service_wait_for_device_change(void);
ESP_Error device_manager_service_get_devices(void);

ESP_Error device_manager_service_clear_data(void);

#endif