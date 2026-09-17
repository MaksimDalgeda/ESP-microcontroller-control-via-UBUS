#ifndef UBUS_H
#define UBUS_H

#include <libubus.h>
#include <syslog.h>

#include "error.h"

typedef struct
{
    struct ubus_context *ctx;
    uint32_t system_id;
    uint32_t lan_id;
    uint32_t wan_id;
    uint32_t network_device_id;

} Ubus_State; 


ESP_Error initialize_ubus(void);
ESP_Error disconnect_ubus(void);


#endif