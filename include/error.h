#ifndef ERROR_H
#define ERROR_H

typedef enum
{
    OK,
    NO_DEVICE_UPDATE,
    ERROR,
    ERR_NULL_POINTER,
    ERR_UBUS_CONNECT,
    ERR_UBUS_INVOKE,
    ERR_UBUS_NOT_INITIALIZED,
    ERR_GET_PORT_LIST,
    PTHREAD_MUTEX_ERROR,
    ERR_DEVICE_NOT_FOUND,
    ERR_PORT

} ESP_Error;

const char *error_to_string(ESP_Error err);

#endif