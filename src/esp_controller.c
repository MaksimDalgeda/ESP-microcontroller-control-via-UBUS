#include "esp_controller.h"

#include <libserialport.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "config.h"

static ESP_Error esp_controller_configure_port(struct sp_port *port)
{
    enum sp_return result;

    result = sp_set_baudrate(port, ESP_BAUDRATE);
    if (result != SP_OK)
        return ERR_PORT;

    result = sp_set_bits(port, ESP_DATA_BITS);
    if (result != SP_OK)
        return ERR_PORT;

    result = sp_set_parity(port, SP_PARITY_NONE);
    if (result != SP_OK)
        return ERR_PORT;

    result = sp_set_stopbits(port, ESP_STOP_BITS);
    if (result != SP_OK)
        return ERR_PORT;

    result = sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);
    if (result != SP_OK)
        return ERR_PORT;

    return OK;
}

static ESP_Error esp_controller_open_port(Device *device, struct sp_port **port_out)
{
    struct sp_port *port = NULL;
    enum sp_return result;
    ESP_Error error;

    if (device == NULL || device->port == NULL || port_out == NULL)
        return ERR_NULL_POINTER;

    *port_out = NULL;

    result = sp_get_port_by_name(device->port, &port);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to find serial port: %s", device->port);
        return ERR_PORT;
    }

    result = sp_open(port, SP_MODE_READ_WRITE);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to open serial port: %s",device->port);

        sp_free_port(port);

        return ERR_PORT;
    }

    error = esp_controller_configure_port(port);

    if (error != OK) {
        syslog(LOG_ERR, "Unable to configure serial port: %s", device->port);

        sp_close(port);
        sp_free_port(port);

        return error;
    }

    *port_out = port;

    return OK;
}

static void esp_controller_close_port(struct sp_port *port)
{
    if (port == NULL)
        return;

    sp_close(port);
    sp_free_port(port);
}

static ESP_Error esp_controller_send_command(struct sp_port *port, const char *command)
{
    int written;
    enum sp_return result;
    size_t command_length;

    if (port == NULL || command == NULL)
        return ERR_NULL_POINTER;

    command_length = strlen(command);

    written = sp_blocking_write(port, command, command_length, ESP_SERIAL_TIMEOUT_MS);

    if (written < 0 || (size_t)written != command_length) {
        syslog(LOG_ERR, "Unable to write complete command");
        return ERR_PORT;
    }

    result = sp_drain(port);

    if (result != SP_OK) {
        syslog(LOG_ERR,"Unable to drain serial port");
        return ERR_PORT;
    }

    return OK;
}

ESP_Error esp_controller_on(Device *device, int pin)
{
    struct sp_port *port = NULL;
    char command[ESP_COMMAND_BUFFER_SIZE];
    ESP_Error error;

    if (device == NULL || device->port == NULL)
        return ERR_NULL_POINTER;

    syslog(LOG_INFO, "Turning ON pin %d on device %s", pin, device->port);

    error = esp_controller_open_port(device, &port);

    if (error != OK)
        return error;

    snprintf(command, sizeof(command), "{\"action\":\"on\",\"pin\":%d}\n", pin);

    error = esp_controller_send_command(port, command);

    esp_controller_close_port(port);

    if (error != OK)
        return error;

    return OK;
}

ESP_Error esp_controller_off(Device *device, int pin)
{
    struct sp_port *port = NULL;
    char command[ESP_COMMAND_BUFFER_SIZE];
    ESP_Error error;

    if (device == NULL || device->port == NULL)
        return ERR_NULL_POINTER;

    syslog(LOG_INFO, "Turning OFF pin %d on device %s", pin, device->port);
    
    error = esp_controller_open_port(device, &port);

    if (error != OK)
        return error;

    snprintf( command, sizeof(command), "{\"action\":\"off\",\"pin\":%d}\n",pin);

    error = esp_controller_send_command(port, command);

    esp_controller_close_port(port);

    return error;
}