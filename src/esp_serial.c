#include "esp_serial.h"
#include "esp_controller.h"

#include <libserialport.h>
#include <string.h>
#include <syslog.h>

#include "config.h"

static ESP_Error esp_serial_configure_port(struct sp_port *port)
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

ESP_Error esp_serial_open(const Device *device, struct sp_port **port_out)
{
    struct sp_port *port = NULL;
    enum sp_return result;
    ESP_Error error;

    if (device == NULL || device->port == NULL || port_out == NULL)
        return ERR_NULL_POINTER;

    *port_out = NULL;

    if (device->vid != ESP_VENDOR_ID || device->pid != ESP_PRODUCT_ID) {

        syslog(LOG_WARNING, "Unsupported device: %s VID=0x%04X PID=0x%04X", device->port, (unsigned int)device->vid, (unsigned int)device->pid);

        return ERR_UNSUPPORTED_DEVICE;
    }

    result = sp_get_port_by_name(device->port, &port);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to find serial port: %s", device->port);
        return ERR_PORT;
    }

    result = sp_open(port, SP_MODE_READ_WRITE);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to open serial port: %s", device->port);
        sp_free_port(port);
        return ERR_PORT;
    }

    error = esp_serial_configure_port(port);

    if (error != OK) {
        syslog(LOG_ERR, "Unable to configure serial port: %s", device->port);

        sp_close(port);
        sp_free_port(port);

        return error;
    }

    *port_out = port;

    return OK;
}

void esp_serial_close(struct sp_port *port)
{
    if (port == NULL)
        return;

    sp_close(port);
    sp_free_port(port);
}

ESP_Error esp_serial_send(struct sp_port *port,const char *command)
{
    int written;
    enum sp_return result;
    size_t command_length;

    if (port == NULL || command == NULL)
        return ERR_NULL_POINTER;

    command_length = strlen(command);

    written = sp_blocking_write(port, command, command_length, ESP_WRITE_TIMEOUT_MS);

    if (written < 0 || (size_t)written != command_length) {
        syslog(LOG_ERR, "Unable to write complete command");
        return ERR_PORT;
    }

    result = sp_drain(port);

    if (result != SP_OK) {
        syslog(LOG_ERR, "Unable to drain serial port");
        return ERR_PORT;
    }

    return OK;
}

ESP_Error esp_serial_read_line(struct sp_port *port, char *response, size_t response_size)
{
    size_t position = 0;

    if (port == NULL || response == NULL || response_size == 0)
        return ERR_NULL_POINTER;

    while (position < response_size - 1) 
    {
        char character;

        enum sp_return result = sp_blocking_read_next(port, &character, 1, ESP_READ_TIMEOUT_MS);

        if (result < 0) {
            syslog(LOG_ERR, "Unable to read from serial port");
            return ERR_PORT;
        }

        if (result == 0) {
            syslog(LOG_ERR, "Serial read timeout");
            return ERR_PORT;
        }

        if (character == '\n')
            break;

        if (character == '\r')
            continue;

        response[position++] = character;
    }

    response[position] = '\0';

    if (position == response_size - 1) {
        syslog(LOG_ERR, "ESP response is too long");
        return ERR_PORT;
    }

    syslog(LOG_INFO, "Received response: %s", response);

    return OK;
}