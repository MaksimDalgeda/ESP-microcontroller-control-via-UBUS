#include "ubus_sensor.h"

#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <syslog.h>

#include "config.h"
#include "device_manager.h"
#include "error.h"
#include "esp_controller.h"
#include "ubus_common.h"

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