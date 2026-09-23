#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define ESP_VENDOR_ID              ((uint16_t)0x10C4)
#define ESP_PRODUCT_ID             ((uint16_t)0xEA60)

#define ESP_BAUDRATE               9600
#define ESP_DATA_BITS              8
#define ESP_STOP_BITS              1

#define ESP_WRITE_TIMEOUT_MS       1000
#define ESP_READ_TIMEOUT_MS        3000

#define ESP_COMMAND_BUFFER_SIZE    64
#define ESP_RESPONSE_BUFFER_SIZE   256

//pins
#define ESP_PIN_D0    16
#define ESP_PIN_D1    5
#define ESP_PIN_D2    4
#define ESP_PIN_D3    0
#define ESP_PIN_D4    2
#define ESP_PIN_D5    14
#define ESP_PIN_D6    12
#define ESP_PIN_D7    13
#define ESP_PIN_D8    15

#endif