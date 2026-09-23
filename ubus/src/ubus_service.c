#include "ubus_service.h"
#include "ubus.h"

ESP_Error ubus_service_init(void)
{
    ESP_Error error;

    syslog(LOG_INFO, "Starting ubus initialization");

    error = initialize_ubus();

    if (error != OK)
        return error;

    error = register_ubus_object();

    if (error != OK)
    {
        disconnect_ubus();
        return error;
    }
    syslog(LOG_INFO, "Ubus initialized successfuly");
    return OK;
}

ESP_Error ubus_service_start(void)
{ 
    ESP_Error error = run_ubus();

    if (error != OK) syslog(LOG_ERR, "UBUS service failed to start - %s (%d)", error_to_string(error), error);

    return error;
}

ESP_Error ubus_service_clear_data(void)
{
    ESP_Error error = disconnect_ubus();

    if (error != OK)syslog(LOG_ERR, "UBUS cleanup failed - %s (%d)", error_to_string(error), error);

    return error;
}