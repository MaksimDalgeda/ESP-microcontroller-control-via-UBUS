#include "service.h"

#include "device_manager.h"

ESP_Error service_find_devices(void)
{
    Device *devices = NULL;
    uint32_t count = 0;

    ESP_Error result = device_manager_find_devices(&devices, &count);

    if (result != OK)
        return result;


    //device_manager_free_devices(devices, count);

    return OK;
}