#ifndef UBUS_COMMON_H
#define UBUS_COMMON_H

#include <stdint.h>

#include <libubus.h>

#include "device_manager.h"
#include "error.h"

int ubus_send_error( struct ubus_context *ctx, struct ubus_request_data *req, ESP_Error error);

ESP_Error ubus_find_device(const char *port_name, Device **device, Device **devices, uint32_t *count);

#endif