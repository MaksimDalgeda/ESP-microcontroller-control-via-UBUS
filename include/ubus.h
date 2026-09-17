#ifndef UBUS_H
#define UBUS_H

#include <libubus.h>
#include <syslog.h>

typedef struct
{
    struct ubus_context *ctx;
    uint32_t system_id;
    uint32_t lan_id;
    uint32_t wan_id;
    uint32_t network_device_id;

} Ubus_State; 


Error_Code initialize_ubus(void);
Error_Code disconnect_ubus(void);


#endif