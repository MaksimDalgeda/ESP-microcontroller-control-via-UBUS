#include <uci.h>

#include "ubus.h"

static Ubus_State g_ubus;

Error_Code initialize_ubus(void){

    g_ubus.ctx = ubus_connect(NULL);

    if(g_ubus.ctx == NULL)
        return ERR_UBUS_CONNECT;

    return OK;
}

Error_Code disconnect_ubus(void){

    if (g_ubus.ctx != NULL){
        ubus_free(g_ubus.ctx);
        g_ubus.ctx = NULL;
    }

    return OK;
}