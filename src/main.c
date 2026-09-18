#include <syslog.h>

#include "service.h"
#include "signal_handler.h"

int main(void)
{
    ESP_Error error = OK;
    
    openlog("esp-controller", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Application started");
    set_signal_action();


    error = service_init();
    if(error != OK)
        goto end;

    error = service_find_devices();

    if(error != OK)
        goto end;

    while(!stop)
    {
        error = service_wait_for_device_change();

        if (error != OK)
            goto end;

        error = service_update_devices();

        if (error != OK)
            goto end;
    }

    end:
    service_clear_data();
    
    if(error == OK)
        syslog(LOG_INFO, "Application stopped without error");
    else
        syslog(LOG_ERR, "Application stopped with error -%s (%d)", error_to_string(error), error);
    closelog();
    return error;
}