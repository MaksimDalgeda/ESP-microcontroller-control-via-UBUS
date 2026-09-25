#include "esp_controller.h"

#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "esp_serial.h"

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

static ESP_Error esp_controller_set_pin(const Device *device, int pin, const char *action)
{
    struct sp_port *port = NULL;
    char command[ESP_COMMAND_BUFFER_SIZE];
    ESP_Error error;

    if (device == NULL || device->port == NULL || action == NULL)
        return ERR_NULL_POINTER;

    error = esp_controller_validate_pin(pin);
    if (error != OK)
        return error;

    syslog(LOG_INFO, "Turning %s pin %d on device %s", action, pin, device->port);

    error = esp_serial_open(device, &port);
    if (error != OK)
        return error;

    int length = snprintf(command, sizeof(command), "{\"action\":\"%s\",\"pin\":%d}\n", action, pin);

    if (length < 0 || (size_t)length >= sizeof(command)) {
        syslog(LOG_ERR, "ESP command is too long");

        esp_serial_close(port);

        return ERR_PORT;
    }

    error = esp_serial_send(port, command);

    esp_serial_close(port);

    return error;
}

ESP_Error esp_controller_on(Device *device, int pin)
{
    return esp_controller_set_pin(device, pin, "on");
}

ESP_Error esp_controller_off(Device *device, int pin)
{
    return esp_controller_set_pin(device, pin, "off");
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

    syslog(LOG_INFO,"Getting sensor data from %s, sensor=%s, model=%s, pin=%d", device->port, sensor, model, pin);

    error = esp_serial_open(device, &port);
    if (error != OK)
        return error;

    int length = snprintf(command, sizeof(command), "{\"action\":\"get\",\"sensor\":\"%s\",\"pin\":%d,\"model\":\"%s\"}\n", sensor, pin, model);

    if (length < 0 || (size_t)length >= sizeof(command)) {
        syslog(LOG_ERR, "ESP command is too long");

        esp_serial_close(port);

        return ERR_PORT;
    }

    error = esp_serial_send(port, command);

    if (error != OK) {
        esp_serial_close(port);
        return error;
    }

    error = esp_serial_read_line(port, response, response_size);

    esp_serial_close(port);

    return error;
}