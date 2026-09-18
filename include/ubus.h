#ifndef UBUS_H
#define UBUS_H

#include <libubus.h>

#include "error.h"

typedef struct
{
    struct ubus_context *ctx;

} Ubus_State; 


ESP_Error initialize_ubus(void);
ESP_Error register_ubus_object(void);
ESP_Error run_ubus(void);
ESP_Error disconnect_ubus(void);


#endif