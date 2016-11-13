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

typedef struct {
	long int framecounter;
	bool fin;
	uint8 rsvs; //first 3 rsv bits
	uint8 opcode;
	long int payloadlen;
	bool masked;
	uint8 maskkey[4];
} WebsocketFrame;

bool handshake(char *, struct espconn *);
bool getMessageParameters(char *, WebsocketFrame*);
bool getMessage (char *,WebsocketFrame* ,char *);
bool sendPong(struct espconn*);
bool sendPing(struct espconn*);
bool closeWebsocket(struct espconn*);
bool sendMessage(char*, struct espconn*);
bool sendMessagePacket(char *, WebsocketFrame *, struct espconn *);


#endif
