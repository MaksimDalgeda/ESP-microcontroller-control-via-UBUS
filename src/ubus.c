#include <uci.h>

#include "ubus.h"

static Ubus_State g_ubus;

ESP_Error initialize_ubus(void){

    g_ubus.ctx = ubus_connect(NULL);

    if(g_ubus.ctx == NULL)
        return ERR_UBUS_CONNECT;

    return OK;
}

ESP_Error disconnect_ubus(void){

    if (g_ubus.ctx != NULL){
        ubus_free(g_ubus.ctx);
        g_ubus.ctx = NULL;
    }

    return OK;
}