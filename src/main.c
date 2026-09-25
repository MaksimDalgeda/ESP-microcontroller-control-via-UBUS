#include <syslog.h>

#include "device_manager_service.h"
#include "signal_handler.h"
#include "threads.h"

int main(void)
{
    ESP_Error error = OK;

    openlog("esp-controller", LOG_PID | LOG_CONS, LOG_DAEMON);

    syslog(LOG_INFO, "Application started");

    set_signal_action();

    error = device_manager_service_init();

    if (error != OK)
        goto end;

    error = device_manager_service_find_devices();

    if (error != OK)
        goto end;

    error = threads_start();

    if (error != OK)
        goto end;

    error = threads_join();

end:
    device_manager_service_clear_data();

    if (error == OK)
        syslog(LOG_INFO, "Application stopped without error");
    else
        syslog(LOG_ERR,"Application stopped with error - %s (%d)", error_to_string(error),error);
     
    closelog();

    return error;
}