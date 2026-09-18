#include "ubus_service.h"
#include "ubus.h"

ESP_Error ubus_service_init(void)
{
    ESP_Error error;

    error = initialize_ubus();

    if (error != OK)
        return error;

    error = register_ubus_object();

    if (error != OK)
    {
        disconnect_ubus();
        return error;
    }

    return OK;
}

ESP_Error ubus_service_start(void)
{
    return run_ubus();
}

ESP_Error ubus_service_clear_data(void)
{
    return disconnect_ubus();
}