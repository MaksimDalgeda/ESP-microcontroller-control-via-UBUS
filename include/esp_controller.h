#ifndef ESP_CONTROLLER_H
#define ESP_CONTROLLER_H

#include <stdio.h>

#include "device_manager.h"
#include "error.h"

ESP_Error esp_controller_on(Device *device, int pin, char *response, size_t response_size);
ESP_Error esp_controller_off( Device *device, int pin, char *response, size_t response_size);
ESP_Error esp_controller_get(Device *device, int pin, const char *model, const char *sensor, char *response, size_t response_size);

#endif