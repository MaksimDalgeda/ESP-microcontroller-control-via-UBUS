#include <pthread.h>
#include <syslog.h>
#include <unistd.h>

#include "threads.h"
#include "device_manager_service.h"
#include "ubus_service.h"
#include "signal_handler.h"

static pthread_t device_manager_thread;
static pthread_t ubus_thread;

static ESP_Error device_manager_thread_error = OK;
static ESP_Error ubus_thread_error = OK;


static void *device_manager_thread_function(void *arg)
{
    (void)arg;

    ESP_Error error;

    while (!stop)
    {
        error = device_manager_service_wait_for_device_change();

        if (stop)
            break;

        if (error == NO_DEVICE_UPDATE)
            continue;

        if (error != OK){
            syslog(LOG_ERR, "Device manager wait failed - %s (%d)", error_to_string(error), error);

            device_manager_thread_error = error;
            stop = 1;

            break;
        }

        error = device_manager_service_update_devices();

        if (error != OK){
            syslog(LOG_ERR, "Device manager update failed - %s (%d)", error_to_string(error), error);

            device_manager_thread_error = error;
            stop = 1;

            break;
        }
    }

    return NULL;
}


static void *ubus_thread_function(void *arg)
{
    (void)arg;

    while (!stop)
    {
        ESP_Error error;

        error = ubus_service_init();

        if (error != OK)
        {
            syslog(LOG_ERR, "UBUS initialization failed - %s (%d), retrying in 5 seconds", error_to_string(error),error);

            for (int i = 0; i < 5 && !stop; i++) //need to test sleep
                sleep(1);

            continue;
        }

        error = ubus_service_start();

        ubus_service_clear_data();

        if (stop)
            break;

        if (error != OK)syslog(LOG_ERR, "UBUS service stopped with error - %s (%d), restarting in 5 seconds", error_to_string(error),error);
        
        else syslog(LOG_WARNING, "UBUS service stopped unexpectedly, restarting in 5 seconds");
        

        for (int i = 0; i < 5 && !stop; i++)
            sleep(1);
    }

    return NULL;
}


ESP_Error threads_start(void)
{
    int result;

    device_manager_thread_error = OK;
    ubus_thread_error = OK;

    result = pthread_create(&device_manager_thread, NULL, device_manager_thread_function,NULL);

    if (result != 0){
        syslog(LOG_ERR, "Failed to create device manager thread: %d",result);

        return ERROR;
    }

    result = pthread_create(&ubus_thread, NULL, ubus_thread_function, NULL);

    if (result != 0){
        syslog(LOG_ERR, "Failed to create UBUS thread: %d", result);

        stop = 1;

        pthread_join(device_manager_thread, NULL);

        return ERROR;
    }

    syslog(LOG_INFO, "Threads started successfully");

    return OK;
}


ESP_Error threads_join(void)
{
    int result;

    result = pthread_join(device_manager_thread, NULL);

    if (result != 0){ syslog(LOG_ERR, "Failed to join device manager thread: %d", result);

        return ERROR;
    }

    result = pthread_join(ubus_thread, NULL);

    if (result != 0){
        syslog(LOG_ERR, "Failed to join UBUS thread: %d", result);

        return ERROR;
    }

    if (device_manager_thread_error != OK)
        return device_manager_thread_error;

    if (ubus_thread_error != OK)
        return ubus_thread_error;

    return OK;
}