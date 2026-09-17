#include "device_manager.h"

#include <libserialport.h>
#include <stdio.h>

ESP_Error device_manager_find_devices(Device **devices, uint32_t *count)
{
    struct sp_port **ports = NULL;

    enum sp_return result = sp_list_ports(&ports);

    if (result != SP_OK)
    {
        return ERROR;
    }

    for (int i = 0; ports[i] != NULL; i++)
    {
        const char *port_name = sp_get_port_name(ports[i]);

        if (port_name == NULL)
        {
            continue;
        }

        printf("Found port: %s\n", port_name);
    }

    sp_free_port_list(ports);

    return OK;
}