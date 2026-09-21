#ifndef UBUS_METHODS_H
#define UBUS_METHODS_H

#include <libubus.h>

#include "error.h"

static int ubus_devices(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg);
ESP_Error ubus_on(void);
ESP_Error ubus_off(void);
ESP_Error ubus_get(void);

#endif