#include "service.h"

int main()
{
    ESP_Error error = OK;
    
    error = service_find_devices();

    printf("%s\n", error_to_string(error));

    return error;

}