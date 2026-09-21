#include "device_manager.h"

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include <libserialport.h>
#include <pthread.h>

static int inotify_fd = -1;
static int inotify_watch = -1;

static Device *devices = NULL;
static uint32_t device_count = 0;
static pthread_mutex_t devices_mutex = PTHREAD_MUTEX_INITIALIZER;

ESP_Error device_manager_init(void)
{
    inotify_fd = inotify_init1(0);

    if (inotify_fd == -1)
        return ERROR;

    inotify_watch = inotify_add_watch(
        inotify_fd,
        "/dev",
        IN_CREATE |
        IN_DELETE |
        IN_MOVED_TO |
        IN_MOVED_FROM
    );

    if (inotify_watch == -1){
        close(inotify_fd);
        inotify_fd = -1;

        return ERROR;
    }

    return OK;
}

ESP_Error device_manager_find_devices(Device **devices, uint32_t *count)
{
    struct sp_port **ports = NULL;
    enum sp_return result;
    
    if(devices == NULL || count == NULL){
        syslog(LOG_ERR, "Variable(s) not initialized in device_manager");
        return ERR_NULL_POINTER;
    }
    
    *devices = NULL;
    *count = 0;

    result = sp_list_ports(&ports);

    if (result != SP_OK){
        syslog(LOG_ERR, "Unable to get available serial ports");
        return ERR_GET_PORT_LIST;
    }

    uint32_t device_count = 0;


    for (uint32_t i = 0; ports[i] != NULL; i++){
        int vid;
        int pid;

        result = sp_get_port_usb_vid_pid(ports[i], &vid, &pid);

        if (result != SP_OK)
            continue;

        device_count++;
    }

     if (device_count == 0){
        sp_free_port_list(ports);
        return OK;
    }
    
    Device *device_list = calloc(device_count, sizeof(Device));

    if (device_list == NULL){
        sp_free_port_list(ports);
        syslog(LOG_ERR, "Device_list not initialized");
        return ERR_NULL_POINTER;
    }

    uint32_t device_index = 0;

    for (uint32_t i = 0; ports[i] != NULL; i++){
        int vid;
        int pid;

        result = sp_get_port_usb_vid_pid(ports[i], &vid, &pid);

        if (result != SP_OK)
            continue;
        

        const char *port_name = sp_get_port_name(ports[i]);

        if (port_name == NULL)
            continue;
        
        device_list[device_index].port = malloc(strlen(port_name) + 1);

        if (device_list[device_index].port == NULL){
            device_manager_free_devices(device_list, device_index);
            sp_free_port_list(ports);
            return ERR_NULL_POINTER;
        }

        strcpy(device_list[device_index].port, port_name);

        device_list[device_index].vid = vid;
        device_list[device_index].pid = pid;

        device_index++;
    }

    sp_free_port_list(ports);

    *devices = device_list;
    *count = device_index;

    return OK;
}

ESP_Error device_manager_wait_for_change(void)
{
    char buffer[4096];

    if (inotify_fd == -1)
        return ERROR;

    while (1){
        ssize_t length = read(inotify_fd, buffer, sizeof(buffer));

        if (length == -1){
            if (errno == EINTR)
                return OK;

            return ERROR;
        }

        size_t offset = 0;

        while (offset < (size_t)length){
            struct inotify_event *event = (struct inotify_event *)&buffer[offset];

            if (event->len > 0) {
                if (strncmp(event->name, "ttyUSB", 6) == 0 || strncmp(event->name, "ttyACM", 6) == 0)
                    return OK;
            }
            offset += sizeof(struct inotify_event) + event->len;
        }
    }
}

ESP_Error device_manager_update_devices(void)
{
    Device *new_devices = NULL;
    uint32_t new_count = 0;

    ESP_Error error = device_manager_find_devices(&new_devices, &new_count);

    if (error != OK)
        return error;

    pthread_mutex_lock(&devices_mutex);

    Device *old_devices = devices;
    uint32_t old_count = device_count;

    devices = new_devices;
    device_count = new_count;

    pthread_mutex_unlock(&devices_mutex);

    device_manager_free_devices(old_devices, old_count);

    return OK;
}
ESP_Error device_manager_get_devices(Device **devices_out, uint32_t *count_out)
{
    if(devices_out == NULL || count_out == NULL)
        return ERR_NULL_POINTER;

    *devices_out = NULL;
    *count_out = 0;

    if (pthread_mutex_lock(&devices_mutex) != 0){
        return PTHREAD_MUTEX_ERROR;
    }

    uint32_t count = device_count;

    if (count == 0){
        pthread_mutex_unlock(&devices_mutex);
        return OK;
    }

    Device *copy = calloc(count, sizeof(Device));

     if (copy == NULL){
        pthread_mutex_unlock(&devices_mutex);
        return ERR_NULL_POINTER;
    }

    for (uint32_t i = 0; i < count; i++){
        copy[i].vid = devices[i].vid;
        copy[i].pid = devices[i].pid;

        size_t port_length = strlen(devices[i].port) + 1;

        copy[i].port = malloc(port_length);

        if (copy[i].port == NULL){
            device_manager_free_devices(copy, i);

            pthread_mutex_unlock(&devices_mutex);

            return ERR_NULL_POINTER;
        }

        memcpy(copy[i].port, devices[i].port, port_length);
    }

    pthread_mutex_unlock(&devices_mutex);

    *devices_out = copy;
    *count_out = count;

    return OK;
}

void device_manager_free_devices(Device *devices, uint32_t count)
{
    if (devices == NULL)
        return;

    for (uint32_t i = 0; i < count; i++){
        free(devices[i].port);
    }

    free(devices);
}

ESP_Error device_manager_clear_data(void)
{
    if (inotify_watch != -1){
        inotify_rm_watch(inotify_fd, inotify_watch);
        inotify_watch = -1;
    }

    if (inotify_fd != -1){
        close(inotify_fd);
        inotify_fd = -1;
    }

    return OK;
}