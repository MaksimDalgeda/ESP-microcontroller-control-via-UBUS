    #include <libubus.h>
    #include <libubox/blobmsg_json.h>

    #include "ubus_methods.h"
    #include "device_manager.h"

    static int ubus_devices(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
    {   
        Device *devices = NULL;
        uint32_t count = 0;

        ESP_Error error = device_manager_get_devices(&devices, &count);

        if(error != OK)
            return UBUS_STATUS_UNKNOWN_ERROR;
        
        struct blob_buf buf;

        blob_buf_init(&buf, 0);

        void *array = blobmsg_open_array(&buf, NULL);

        for(int i = 0; i < count; i++){

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

    static const struct ubus_method esp_methods[] = { //add new methods(off, on, get)
        UBUS_METHOD_NOARG("devices", ubus_devices)
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