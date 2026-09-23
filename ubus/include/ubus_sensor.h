#ifndef UBUS_SENSOR_H
#define UBUS_SENSOR_H

#include <libubus.h>
#include <libubox/blobmsg.h>

#define UBUS_GET_POLICY_COUNT 4

extern const struct blobmsg_policy ubus_get_policy[UBUS_GET_POLICY_COUNT];

int ubus_get(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg);

#endif