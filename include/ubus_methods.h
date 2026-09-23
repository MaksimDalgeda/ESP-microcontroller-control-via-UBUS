#ifndef UBUS_METHODS_H
#define UBUS_METHODS_H

#include <libubus.h>
#include <syslog.h>

#include "error.h"

struct ubus_object *ubus_methods_get_object(void);

#endif