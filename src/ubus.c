#include "ubus.h"
#include "ubus_methods.h"
#include "signal_handler.h"

static Ubus_State g_ubus;
static struct uloop_timeout stop_check_timeout;

static void ubus_check_stop(struct uloop_timeout *timeout)
{
    if (stop){
        uloop_end();
        return;
    }

    uloop_timeout_set(timeout, 500);
}

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

    uloop_init();

    ubus_add_uloop(g_ubus.ctx);

    stop_check_timeout.cb = ubus_check_stop;

    uloop_timeout_set(&stop_check_timeout, 500);

    uloop_run();

    syslog(LOG_INFO, "UBUS event loop stopped");

    return OK;
}

ESP_Error disconnect_ubus(void)
{
    uloop_done();

    if (g_ubus.ctx != NULL){
        ubus_free(g_ubus.ctx);
        g_ubus.ctx = NULL;
    }

    return OK;
}