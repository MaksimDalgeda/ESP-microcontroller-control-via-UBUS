#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define ESP_VENDOR_ID              ((uint16_t)0x10C4)
#define ESP_PRODUCT_ID             ((uint16_t)0xEA60)

#define ESP_BAUDRATE               9600
#define ESP_DATA_BITS              8
#define ESP_STOP_BITS              1
#define ESP_SERIAL_TIMEOUT_MS      1000

#define ESP_COMMAND_BUFFER_SIZE    64

#endif