#include "ubus_common.h"

#include <libubox/blobmsg.h>
#include <syslog.h>
#include <string.h>

int ubus_send_error(struct ubus_context *ctx, struct ubus_request_data *req,ESP_Error error)
{
    struct blob_buf buf = {0};

    if (blob_buf_init(&buf, 0) != 0) {
        syslog(LOG_ERR, "Failed to initialize error response buffer");
        return UBUS_STATUS_UNKNOWN_ERROR;
    }

    void *error_table = blobmsg_open_table(&buf, "error");

    blobmsg_add_u32(&buf, "code", (uint32_t)error);

    blobmsg_add_string(&buf, "message", error_to_string(error));

    blobmsg_close_table(&buf, error_table);

    ubus_send_reply(ctx, req, buf.head);

    blob_buf_free(&buf);

    return UBUS_STATUS_OK;
}

ESP_Error ubus_find_device(const char *port_name, Device **device, Device **devices, uint32_t *count)
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

    device_manager_free_devices(*devices, *count);

    *devices = NULL;
    *count = 0;

    return ERR_DEVICE_NOT_FOUND;
}