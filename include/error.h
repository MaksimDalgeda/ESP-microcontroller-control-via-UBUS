#ifndef ERROR_H
#define ERROR_H

typedef enum
{
    OK,
    ERROR,
    ERR_UBUS_CONNECT,
    ERR_UBUS_INVOKE,
    ERR_UBUS_NOT_INITIALIZED

} ESP_Error;

const char *error_to_string(ESP_Error err);

#endif