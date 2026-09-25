#include "ubus_pin.h"

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