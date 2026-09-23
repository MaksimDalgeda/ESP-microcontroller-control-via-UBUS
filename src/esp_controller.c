#include "esp_controller.h"

#include <libserialport.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

ESP_Error esp_controller_on(Device *device, int pin)
{
    struct sp_port *port = NULL;
    enum sp_return result;
    char command[64];

    if (device == NULL || device->port == NULL)
        return ERR_NULL_POINTER;

    syslog(LOG_INFO, "Turning ON pin %d on device %s", pin, device->port);

    result = sp_get_port_by_name(device->port, &port);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to find serial port: %s", device->port);
        return ERROR;
    }

    result = sp_open(port, SP_MODE_READ_WRITE);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to open serial port: %s",device->port);
        sp_free_port(port);
        return ERROR;
    }

    result = sp_set_baudrate(port, 9600);
    if (result != SP_OK)
        goto serial_error;

    result = sp_set_bits(port, 8);
    if (result != SP_OK)
        goto serial_error;

    result = sp_set_parity(port, SP_PARITY_NONE);
    if (result != SP_OK)
        goto serial_error;

    result = sp_set_stopbits(port, 1);
    if (result != SP_OK)
        goto serial_error;

    result = sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);
    if (result != SP_OK)
        goto serial_error;

    snprintf(command, sizeof(command), "{\"action\":\"on\",\"pin\":%d}\n", pin);

    syslog(LOG_INFO, "Sending command: %s", command);

    int written = sp_blocking_write(port, command, strlen(command),1000);

    if (written < 0 || (size_t)written != strlen(command)) {
        syslog(LOG_ERR, "Unable to write complete command");
        goto serial_error;
    }

    result = sp_drain(port);

    if (result != SP_OK)
        goto serial_error;

    sp_close(port);
    sp_free_port(port);

    syslog(LOG_INFO, "ON command sent successfully");

    return OK;

    serial_error:

    syslog(LOG_ERR, "Serial communication failed for %s", device->port);

    sp_close(port);
    sp_free_port(port);

    return ERROR;
}