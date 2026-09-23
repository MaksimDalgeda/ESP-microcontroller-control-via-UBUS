#ifndef UBUS_PIN_H
#define UBUS_PIN_H

#include <libubus.h>
#include <libubox/blobmsg.h>

#define UBUS_PIN_POLICY_COUNT 2

extern const struct blobmsg_policy ubus_pin_policy[UBUS_PIN_POLICY_COUNT];

int ubus_on(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg);

int ubus_off(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg);

#endif