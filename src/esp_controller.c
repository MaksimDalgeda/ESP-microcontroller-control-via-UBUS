#include "esp_controller.h"

#include <libserialport.h>
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

static ESP_Error esp_controller_validate_device(const Device *device)
{
    if (device == NULL)
        return ERR_NULL_POINTER;

    if (device->vid != ESP_VENDOR_ID || device->pid != ESP_PRODUCT_ID) {

        syslog(LOG_WARNING, "Unsupported device: %s VID=0x%04X PID=0x%04X", device->port, (unsigned int)device->vid, (unsigned int)device->pid);

        return ERR_UNSUPPORTED_DEVICE;
    }

    return OK;
}

static ESP_Error esp_controller_validate_pin(int pin)
{
    switch (pin) {
        case ESP_PIN_D0:
        case ESP_PIN_D1:
        case ESP_PIN_D2:
        case ESP_PIN_D3:
        case ESP_PIN_D4:
        case ESP_PIN_D5:
        case ESP_PIN_D6:
        case ESP_PIN_D7:
        case ESP_PIN_D8:
            return OK;

        default:
            syslog(LOG_WARNING, "Unsupported ESP pin: %d", pin);

            return ERR_INVALID_PIN;
    }
}

static ESP_Error esp_controller_open_port(Device *device, struct sp_port **port_out)
{
    struct sp_port *port = NULL;
    enum sp_return result;
    ESP_Error error;

    if (device == NULL || device->port == NULL || port_out == NULL)
        return ERR_NULL_POINTER;

    *port_out = NULL;

    error = esp_controller_validate_device(device);
    if (error != OK)
        return error;

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

    written = sp_blocking_write(port, command, command_length, ESP_WRITE_TIMEOUT_MS);

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

    error = esp_controller_validate_pin(pin);
    if (error != OK)
        return error;
        
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
    
    error = esp_controller_validate_pin(pin);
    if (error != OK)
        return error;

    syslog(LOG_INFO, "Turning OFF pin %d on device %s", pin, device->port);
    
    error = esp_controller_open_port(device, &port);

    if (error != OK)
        return error;

    snprintf( command, sizeof(command), "{\"action\":\"off\",\"pin\":%d}\n",pin);

    error = esp_controller_send_command(port, command);

    esp_controller_close_port(port);

    return error;
}

static ESP_Error esp_controller_read_response(struct sp_port *port, char *response, size_t response_size)
{
    if (port == NULL || response == NULL || response_size == 0)
        return ERR_NULL_POINTER;

    size_t position = 0;

    while (position < response_size - 1) {
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

ESP_Error esp_controller_get(Device *device, int pin, const char *model, const char *sensor, char *response, size_t response_size)
{
    struct sp_port *port = NULL;
    char command[ESP_COMMAND_BUFFER_SIZE];
    ESP_Error error;

    if (device == NULL ||
        device->port == NULL ||
        model == NULL ||
        sensor == NULL ||
        response == NULL ||
        response_size == 0)
        return ERR_NULL_POINTER;

    error = esp_controller_validate_pin(pin);
    if (error != OK)
        return error;

    syslog(LOG_INFO, "Getting sensor data from %s, sensor=%s, model=%s, pin=%d", device->port, sensor, model, pin);

    error = esp_controller_open_port(device, &port);

    if (error != OK)
        return error;

    snprintf(command, sizeof(command),
        "{\"action\":\"get\",\"sensor\":\"%s\",\"pin\":%d,\"model\":\"%s\"}\n",
        sensor, pin, model);

    error = esp_controller_send_command(port, command);

    if (error != OK) {
        esp_controller_close_port(port);
        return error;
    }

    error = esp_controller_read_response(port, response, response_size);

    esp_controller_close_port(port);

    return error;
}