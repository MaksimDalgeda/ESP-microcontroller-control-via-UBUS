#include "ubus_devices.h"

#include <libubox/blobmsg.h>
#include <syslog.h>

#include "device_manager.h"
#include "error.h"

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