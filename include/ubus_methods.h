#ifndef UBUS_METHODS_H
#define UBUS_METHODS_H

#include <libubus.h>
#include <syslog.h>

#include "error.h"

struct ubus_object *ubus_methods_get_object(void);

ESP_Error ubus_on(void);
ESP_Error ubus_off(void);
ESP_Error ubus_get(void);

#endif