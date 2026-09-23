#include "error.h"

const char *error_to_string(ESP_Error err)
{
    switch (err)
    {
        case OK:
            return "OK";

        case NO_DEVICE_UPDATE:
            return "NO_DEVICE_UPDATE";

        case ERROR:
            return "ERROR";

        case ERR_NULL_POINTER:
            return "ERR_NULL_POINTER";

        case ERR_UBUS_CONNECT:
            return "ERR_UBUS_CONNECT";

        case ERR_UBUS_INVOKE:
            return "ERR_UBUS_INVOKE";

        case ERR_UBUS_NOT_INITIALIZED:
            return "ERR_UBUS_NOT_INITIALIZED";

        case ERR_GET_PORT_LIST:
            return "ERR_GET_PORT_LIST";

        case PTHREAD_MUTEX_ERROR:
            return "PTHREAD_MUTEX_ERROR";

        case ERR_PORT:
            return "ERR_PORT";
        
        case ERR_DEVICE_NOT_FOUND:
            return "ERR_DEVICE_NOT_FOUND";

        default:
            return "UNKNOWN_ERROR";
        
    }
}