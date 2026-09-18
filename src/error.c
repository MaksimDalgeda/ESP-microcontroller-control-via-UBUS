#include "error.h"

const char *error_to_string(ESP_Error err)
{
    switch (err)
    {
        case OK:
            return "OK";

        case ERROR:
            return "ERROR";

        case ERR_UBUS_CONNECT:
            return "ERR_UBUS_CONNECT";

        case ERR_UBUS_INVOKE:
            return "ERR_UBUS_INVOKE";

        case ERR_UBUS_NOT_INITIALIZED:
            return "ERR_UBUS_NOT_INITIALIZED";

        default:
            return "UNKNOWN_ERROR";
        
    }
}