#include "ubus_controlls.h"

#include <libubox/blobmsg.h>
#include <stdbool.h>
#include <syslog.h>

#include "device_manager.h"
#include "error.h"
#include "esp_controller.h"
#include "ubus_common.h"
#include "config.h"

enum {
    PIN_PORT,
    PIN_PIN
};

const struct blobmsg_policy ubus_pin_policy[UBUS_PIN_POLICY_COUNT] = {
    [PIN_PORT] = {
        .name = "port",
        .type = BLOBMSG_TYPE_STRING
    },
    [PIN_PIN] = {
        .name = "pin",
        .type = BLOBMSG_TYPE_INT32
    }
};

enum {
    GET_PORT,
    GET_PIN,
    GET_MODEL,
    GET_SENSOR
};

const struct blobmsg_policy ubus_get_policy[UBUS_GET_POLICY_COUNT] = {
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


int ubus_devices(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    (void)obj;
    (void)method;
    (void)msg;

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

    void *array = blobmsg_open_array(&buf, "devices");

    for (uint32_t i = 0; i < count; i++) {
        void *table = blobmsg_open_table(&buf, NULL);

        blobmsg_add_string(&buf, "port", devices[i].port);

        char vendor_id[7];
        char product_id[7];

        snprintf(vendor_id, sizeof(vendor_id), "0x%04X", devices[i].vid);

        snprintf(product_id, sizeof(product_id), "0x%04X", devices[i].pid);

        blobmsg_add_string(&buf, "vendor_id", vendor_id);

        blobmsg_add_string(&buf, "product_id", product_id);

        blobmsg_close_table(&buf, table);
    }

    blobmsg_close_array(&buf, array);

    ubus_send_reply(ctx, req, buf.head);

    blob_buf_free(&buf);

    device_manager_free_devices(devices, count);

    return UBUS_STATUS_OK;
}

static int ubus_handle_pin(struct ubus_context *ctx, struct ubus_request_data *req, struct blob_attr *msg, bool turn_on)
{
    struct blob_attr *tb[UBUS_PIN_POLICY_COUNT] = {0};

    blobmsg_parse(ubus_pin_policy, UBUS_PIN_POLICY_COUNT, tb, blob_data(msg), blob_len(msg));

    if (tb[PIN_PORT] == NULL ||
        tb[PIN_PIN] == NULL) {

        syslog(LOG_ERR, "%s: missing required arguments", turn_on ? "ON" : "OFF");

        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    const char *port = blobmsg_get_string(tb[PIN_PORT]);
    int pin = blobmsg_get_u32(tb[PIN_PIN]);

    Device *device = NULL;
    Device *devices = NULL;
    uint32_t count = 0;

    ESP_Error error = ubus_find_device(port, &device, &devices, &count);

    if (error != OK) {
        if (error == ERR_DEVICE_NOT_FOUND)
            syslog(LOG_ERR, "Device not found: %s", port);
        else 
            syslog(LOG_ERR, "Unable to find device: %s (%d)", error_to_string(error), error);

        device_manager_free_devices(devices, count);

        return ubus_send_error(ctx, req, error);
    }

    char response[ESP_RESPONSE_BUFFER_SIZE];

    if (turn_on) 
        error = esp_controller_on(device, pin, response, sizeof(response));
    else 
        error = esp_controller_off(device, pin, response, sizeof(response));
    

    device_manager_free_devices(devices, count);

    if (error != OK) {
        syslog(LOG_ERR,"Failed to turn %s pin %d on %s: %s (%d)", turn_on ? "ON" : "OFF", pin, port, error_to_string(error), error);
        return ubus_send_error(ctx, req, error);
    }

    struct blob_buf buf = {0};

    if (blob_buf_init(&buf, 0) != 0) {
        syslog(LOG_ERR, "Unable to initialize blob buffer");
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

int ubus_on(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    (void)obj;
    (void)method;

    return ubus_handle_pin(ctx, req, msg, true);
}

int ubus_off(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    (void)obj;
    (void)method;

    return ubus_handle_pin( ctx, req, msg, false);
}

int ubus_get(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    (void)obj;
    (void)method;

    struct blob_attr *tb[UBUS_GET_POLICY_COUNT] = {0};

    blobmsg_parse(ubus_get_policy, UBUS_GET_POLICY_COUNT, tb, blob_data(msg), blob_len(msg));

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
    const char *sensor =blobmsg_get_string(tb[GET_SENSOR]);

    Device *device = NULL;
    Device *devices = NULL;
    uint32_t count = 0;

    ESP_Error error = ubus_find_device(port, &device, &devices, &count);

    if (error != OK) {
        syslog(LOG_ERR, "Unable to find device: %s (%d)", error_to_string(error),error);
        device_manager_free_devices(devices, count);
        return ubus_send_error(ctx, req, error);
    }

    char response[ESP_RESPONSE_BUFFER_SIZE] = {0};

    error = esp_controller_get(device, pin, model,sensor, response, sizeof(response));

    device_manager_free_devices(devices, count);

    if (error != OK) {
        syslog(LOG_ERR, "Failed to get sensor data from %s: %s (%d)", port, error_to_string(error), error);
        return ubus_send_error(ctx, req, error);
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