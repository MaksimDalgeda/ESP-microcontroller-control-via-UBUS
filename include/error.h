#ifndef ERROR_H
#define ERROR_H

typedef enum
{
    OK,
    ERROR,
    ERR_NULL_POINTER,
    ERR_UBUS_CONNECT,
    ERR_UBUS_INVOKE,
    ERR_UBUS_NOT_INITIALIZED,
    ERR_GET_PORT_LIST,
    PTHREAD_MUTEX_ERROR


} ESP_Error;

const char *error_to_string(ESP_Error err);

#endif