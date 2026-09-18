#include "service.h"
#include "device_manager.h"

ESP_Error service_init(void)
{
    return device_manager_init();
}

ESP_Error service_find_devices(void)
{
    Device *devices = NULL;
    uint32_t count = 0;

    syslog(LOG_INFO, "Looking for devices");

    ESP_Error result = device_manager_find_devices(&devices, &count);

    if (result != OK)
        return result;

    if (count == 0){
        syslog(LOG_INFO, "No devices founded");
        device_manager_free_devices(devices, count);
        return OK;
    }

    for (uint32_t i = 0; i < count; i++){
        printf("Device %u: port=%s VID=%04x PID=%04x\n", i, devices[i].port,devices[i].vid, devices[i].pid);
    }

    syslog(LOG_INFO, "Device(s) founded successfuly");
    
    device_manager_free_devices(devices, count);
    return OK;
}

ESP_Error service_update_devices(void)
{
    ESP_Error error;
    error = device_manager_update_devices();

    if(error != OK)
        syslog(LOG_ERR,"Device update failed.");
    return error;
}

ESP_Error service_wait_for_device_change(void)
{
    ESP_Error error;
    error = device_manager_wait_for_change();

    if(error != OK)
        syslog(LOG_ERR, "Waiting for device change failed.");
    return error;
}

ESP_Error service_get_devices(void)
{
    return OK;
}

ESP_Error service_clear_data(void)
{
    return device_manager_clear_data();
}