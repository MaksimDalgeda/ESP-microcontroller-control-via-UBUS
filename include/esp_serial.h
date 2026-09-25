#ifndef ESP_SERIAL_H
#define ESP_SERIAL_H

#include <stddef.h>

#include <libserialport.h>

#include "device_manager.h"
#include "error.h"

ESP_Error esp_serial_open(const Device *device, struct sp_port **port);

void esp_serial_close(struct sp_port *port);

ESP_Error esp_serial_send(struct sp_port *port, const char *command);

ESP_Error esp_serial_read_line( struct sp_port *port, char *response, size_t response_size);

#endif