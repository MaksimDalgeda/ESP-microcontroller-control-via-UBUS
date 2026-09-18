#ifndef UBUS_SERVICE_H
#define UBUS_SERVICE_H

#include "error.h"

ESP_Error ubus_service_init(void);
ESP_Error ubus_service_start(void);
ESP_Error ubus_service_clear_data(void);

#endif