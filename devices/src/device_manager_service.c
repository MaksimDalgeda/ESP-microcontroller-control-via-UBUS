#include "device_manager_service.h"
#include "device_manager.h"

ESP_Error device_manager_service_init(void)
{
    return device_manager_init();
}

ESP_Error device_manager_service_find_devices(void)
{
    syslog(LOG_INFO, "Looking for devices");

    ESP_Error result = device_manager_update_devices();

    if (result != OK)
        return result;

    syslog(LOG_INFO, "Device(s) found successfully");

    return OK;
}

ESP_Error device_manager_service_update_devices(void)
{
    ESP_Error error;
    error = device_manager_update_devices();

    if(error != OK){
        syslog(LOG_ERR,"Device update failed.");
        return error;
    }

    return OK;
}

ESP_Error device_manager_service_wait_for_device_change(void)
{
    ESP_Error error;
    error = device_manager_wait_for_change();

    if(error != OK && error != NO_DEVICE_UPDATE){
        syslog(LOG_ERR, "Waiting for device change failed.");
        return error;
    }

    return error;
}

ESP_Error device_manager_service_get_devices(void)
{
    return OK;
}

ESP_Error device_manager_service_clear_data(void)
{
    return device_manager_clear_data();
}