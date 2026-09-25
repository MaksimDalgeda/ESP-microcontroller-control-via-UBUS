#ifndef UBUS_CONTROLLS_H
#define UBUS_CONTROLLS_H

#include <libubus.h>
#include <libubox/blobmsg.h>

#include "config.h"

extern const struct blobmsg_policy ubus_pin_policy[UBUS_PIN_POLICY_COUNT];

extern const struct blobmsg_policy ubus_get_policy[UBUS_GET_POLICY_COUNT];

int ubus_on(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg);

int ubus_off(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg);

int ubus_get(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg);

int ubus_devices(
    struct ubus_context *ctx,
    struct ubus_object *obj,
    struct ubus_request_data *req,
    const char *method,
    struct blob_attr *msg);

#endif