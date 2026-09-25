#include "ubus_methods.h"

#include <libubus.h>

#include "ubus_devices.h"
#include "ubus_pin.h"
#include "ubus_sensor.h"

static const struct ubus_method esp_methods[] = {
    UBUS_METHOD_NOARG("devices", ubus_devices),
    UBUS_METHOD("on", ubus_on, ubus_pin_policy),
    UBUS_METHOD("off", ubus_off, ubus_pin_policy),
    UBUS_METHOD("get", ubus_get, ubus_get_policy)
};

static struct ubus_object_type esp_object_type = UBUS_OBJECT_TYPE("esp-controller", esp_methods);

static struct ubus_object esp_object = {
    .name = "esp-controller",
    .type = &esp_object_type,
    .methods = esp_methods,
    .n_methods = sizeof(esp_methods) / sizeof(esp_methods[0]),
};

struct ubus_object *ubus_methods_get_object(void)
{
    return &esp_object;
}