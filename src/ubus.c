#include "ubus.h"
#include "ubus_methods.h"
static Ubus_State g_ubus;

ESP_Error initialize_ubus(void){

    if(uloop_init() != 0){
        syslog(LOG_ERR, "UBUS service failed to initialize ubus");
        return ERR_UBUS_INVOKE;
    }
    g_ubus.ctx = ubus_connect(NULL);

    if (g_ubus.ctx == NULL){
        uloop_done();
        return ERR_UBUS_CONNECT;
    }

    return OK;
}

ESP_Error register_ubus_object(void)
{
    if(g_ubus.ctx == NULL)
        return ERR_UBUS_NOT_INITIALIZED;

    struct ubus_object *object = ubus_methods_get_object();

    if(object == NULL){
        syslog(LOG_ERR, "UBUS service failed to get object");
        return ERR_NULL_POINTER;
    }

    int result = ubus_add_object(g_ubus.ctx, object);
  
    if(result != 0){
        syslog(LOG_ERR, "Ubus service failes to register ubus object: %s (%d)", ubus_strerror(result), result);
        return ERR_UBUS_INVOKE;
    }
    return OK;
}

ESP_Error run_ubus(void)
{
    if (g_ubus.ctx == NULL)
        return ERR_UBUS_NOT_INITIALIZED;

    ubus_add_uloop(g_ubus.ctx);

    uloop_run();

    return OK;
}

ESP_Error disconnect_ubus(void){

    if (g_ubus.ctx != NULL){
        ubus_free(g_ubus.ctx);
        g_ubus.ctx = NULL;
    }

    uloop_done();

    return OK;
}