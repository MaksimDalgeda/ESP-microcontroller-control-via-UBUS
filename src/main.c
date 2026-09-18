#include <syslog.h>

#include "service.h"

int main()
{
    ESP_Error error = OK;
    
    openlog("esp-controller", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Application started");


    error = service_find_devices();

    if(error != OK)
        goto end;


    end:
    if(error == OK)
        syslog(LOG_INFO, "Application stopped without error");
    else
        syslog(LOG_INFO, "Application stopped with error -%s (%d)", error_to_string(error), error);
    closelog();
    return error;

}
