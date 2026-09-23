#include <libubus.h>
#include <libubox/blobmsg_json.h>

#include "ubus_methods.h"
#include "device_manager.h"
#include "esp_controller.h"
#include "config.h"

enum {
    PIN_PORT,
    PIN_PIN,
    __PIN_MAX
};

static const struct blobmsg_policy pin_policy[] = {
    [PIN_PORT] = {
        .name = "port",
        .type = BLOBMSG_TYPE_STRING
    },
    [PIN_PIN] = {
        .name = "pin",
        .type = BLOBMSG_TYPE_INT32
    }
};

static int ubus_devices(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{   
    Device *devices = NULL;
    uint32_t count = 0;

    ESP_Error error = device_manager_get_devices(&devices, &count);

    if (error != OK)
        return UBUS_STATUS_UNKNOWN_ERROR;

    struct blob_buf buf = {0};

    if (blob_buf_init(&buf, 0) != 0) {
        syslog(LOG_ERR, "Failed to initialize blob buffer");
        device_manager_free_devices(devices, count);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    blob_buf_init(&buf, 0);

    void *array = blobmsg_open_array(&buf, NULL);
    
    for (uint32_t i = 0; i < count; i++) {

        void *table = blobmsg_open_table(&buf, NULL);

        blobmsg_add_string(&buf, "port", devices[i].port);
        blobmsg_add_u32(&buf, "vendor_id", devices[i].vid);
        blobmsg_add_u32(&buf, "product_id", devices[i].pid);

        blobmsg_close_table(&buf, table);
    }

    blobmsg_close_array(&buf, array);

    ubus_send_reply(ctx, req, buf.head);

    blob_buf_free(&buf);

    device_manager_free_devices(devices, count);

    return UBUS_STATUS_OK;
}

static ESP_Error ubus_find_device(const char *port_name, Device **device, Device **devices,uint32_t *count)
{
    if (port_name == NULL ||
        device == NULL ||
        devices == NULL ||
        count == NULL)
        return ERR_NULL_POINTER;

    *device = NULL;
    *devices = NULL;
    *count = 0;

    ESP_Error error = device_manager_get_devices(devices, count);

    if (error != OK)
        return error;

    for (uint32_t i = 0; i < *count; i++) {
        if (strcmp((*devices)[i].port, port_name) == 0) {
            *device = &(*devices)[i];
            return OK;
        }
    }

    return ERR_DEVICE_NOT_FOUND;
}

static int ubus_on(struct ubus_context *ctx, struct ubus_object *obj,struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    struct blob_attr *tb[__PIN_MAX] = {0};

    blobmsg_parse( pin_policy, __PIN_MAX, tb, blob_data(msg), blob_len(msg));

    if (tb[PIN_PORT] == NULL || tb[PIN_PIN] == NULL) {
        syslog(LOG_ERR, "ON: missing required arguments");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    const char *port = blobmsg_get_string(tb[PIN_PORT]);
    int pin = blobmsg_get_u32(tb[PIN_PIN]);

    Device *devices = NULL;
    Device *device = NULL;
    uint32_t count = 0;

    ESP_Error error = ubus_find_device(port, &device, &devices,&count);

    if (error == ERR_DEVICE_NOT_FOUND) {
        syslog(LOG_ERR, "Device not found: %s", port);
        return UBUS_STATUS_NOT_FOUND;
    }

    if (error != OK) {
        syslog(LOG_ERR,"Unable to find device: %s (%d)", error_to_string(error), error);
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    error = esp_controller_on(device, pin);

    device_manager_free_devices(devices, count);

    if (error != OK) {
        syslog(LOG_ERR,"Failed to turn ON pin %d on %s: %s (%d)",pin, port, error_to_string(error), error);

        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    return UBUS_STATUS_OK;
}

static int ubus_off(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    struct blob_attr *tb[__PIN_MAX] = {0};

    blobmsg_parse(pin_policy, __PIN_MAX, tb, blob_data(msg), blob_len(msg));

    if (tb[PIN_PORT] == NULL || tb[PIN_PIN] == NULL) {
        syslog(LOG_ERR, "OFF: missing required arguments");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    const char *port = blobmsg_get_string(tb[PIN_PORT]);
    int pin = blobmsg_get_u32(tb[PIN_PIN]);

    Device *devices = NULL;
    Device *device = NULL;
    uint32_t count = 0;

    ESP_Error error = ubus_find_device(port, &device, &devices, &count);

    if (error == ERR_DEVICE_NOT_FOUND) {
        syslog(LOG_ERR, "Device not found: %s", port);

        device_manager_free_devices(devices, count);

        return UBUS_STATUS_NOT_FOUND;
    }

    if (error != OK) {
        syslog(LOG_ERR,"Unable to find device: %s (%d)", error_to_string(error), error);

        device_manager_free_devices(devices, count);

        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    error = esp_controller_off(device, pin);

    device_manager_free_devices(devices, count);

    if (error != OK) {
        syslog(LOG_ERR, "Failed to turn OFF pin %d on %s: %s (%d)",pin, port, error_to_string(error), error);

        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    return UBUS_STATUS_OK;
}

enum {
    GET_PORT,
    GET_PIN,
    GET_MODEL,
    GET_SENSOR,
    __GET_MAX
};

static const struct blobmsg_policy get_policy[] = {
    [GET_PORT] = {
        .name = "port",
        .type = BLOBMSG_TYPE_STRING
    },
    [GET_PIN] = {
        .name = "pin",
        .type = BLOBMSG_TYPE_INT32
    },
    [GET_MODEL] = {
        .name = "model",
        .type = BLOBMSG_TYPE_STRING
    },
    [GET_SENSOR] = {
        .name = "sensor",
        .type = BLOBMSG_TYPE_STRING
    }
};

static int ubus_get(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method,struct blob_attr *msg)
{
    struct blob_attr *tb[__GET_MAX] = {0};

    blobmsg_parse(get_policy, __GET_MAX, tb, blob_data(msg), blob_len(msg));

    if (tb[GET_PORT] == NULL ||
        tb[GET_PIN] == NULL ||
        tb[GET_MODEL] == NULL ||
        tb[GET_SENSOR] == NULL) {

        syslog(LOG_ERR, "GET: missing required arguments");

        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    const char *port = blobmsg_get_string(tb[GET_PORT]);
    int pin = blobmsg_get_u32(tb[GET_PIN]);
    const char *model = blobmsg_get_string(tb[GET_MODEL]);
    const char *sensor = blobmsg_get_string(tb[GET_SENSOR]);

    Device *devices = NULL;
    Device *device = NULL;
    uint32_t count = 0;

    ESP_Error error = ubus_find_device(port, &device, &devices, &count);

    if (error == ERR_DEVICE_NOT_FOUND) {
        syslog(LOG_ERR,"Device not found: %s",port);

        device_manager_free_devices(devices, count);

        return UBUS_STATUS_NOT_FOUND;
    }

    if (error != OK) {
        syslog(LOG_ERR,"Unable to find device: %s (%d)",error_to_string(error), error);

        device_manager_free_devices(devices, count);

        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    char response[ESP_RESPONSE_BUFFER_SIZE] = {0};

    error = esp_controller_get(device, pin, model, sensor, response, sizeof(response));

    device_manager_free_devices(devices, count);

    if (error != OK) {
        syslog(LOG_ERR, "Failed to get sensor data from %s: %s (%d)", port, error_to_string(error), error);

        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    struct blob_buf buf = {0};

    if (blob_buf_init(&buf, 0) != 0) {
        syslog(LOG_ERR, "Failed to initialize blob buffer");
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    if (!blobmsg_add_json_from_string(&buf, response)) {
        syslog(LOG_ERR, "Invalid JSON response from ESP: %s", response);

        blob_buf_free(&buf);

        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    ubus_send_reply(ctx, req, buf.head);

    blob_buf_free(&buf);

    return UBUS_STATUS_OK;
}


static const struct ubus_method esp_methods[] = { 
    UBUS_METHOD_NOARG("devices", ubus_devices),
    UBUS_METHOD("on", ubus_on, pin_policy),
    UBUS_METHOD("off", ubus_off, pin_policy),
    UBUS_METHOD("get", ubus_get, get_policy)
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