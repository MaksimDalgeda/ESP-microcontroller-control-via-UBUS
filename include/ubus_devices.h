#ifndef UBUS_DEVICES_H
#define UBUS_DEVICES_H

#include <libubus.h>

int ubus_devices( struct ubus_context *ctx,struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg);

#endif