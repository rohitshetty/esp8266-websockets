#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H
#include "../utils/utils.h"
#include "../rom.h"
#include "mem.h"
#include "stdlib.h"
#include <ip_addr.h>
#include <c_types.h>
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"


bool handshake(char *, struct espconn *);
#endif
