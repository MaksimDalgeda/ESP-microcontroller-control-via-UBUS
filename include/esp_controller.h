#ifndef ESP_CONTROLLER_H
#define ESP_CONTROLLER_H

#include "device_manager.h"
#include "error.h"

ESP_Error esp_controller_test_connection(Device *device);//???

ESP_Error esp_controller_on(Device *device, int pin);
ESP_Error esp_controller_off(Device *device, int pin);
ESP_Error esp_controller_get(Device *device, int pin, const char *model, const char *sensor);

#endif