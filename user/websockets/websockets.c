#include "websockets.h"
bool handshake(char *header, struct espconn *connection) {
	KEYVALUE_PAIR keyvaluepair;
	char key[20];
	char value[100];
	uint8 buffer[20];
	SHA1_CTX sha1_context;
	size_t length;
	int i;
	unsigned char * base64;
   	unsigned char * somemem;
	char response[500] = "HTTP/1.1 101 Switching Protocols \r\nUpgrade: websocket \r\nConnection: Upgrade \r\nSec-WebSocket-Protocol: chat \r\nSec-WebSocket-Accept: ";




	keyvaluepair.key = key;
	keyvaluepair.value = value;
	if (getKeyPair(header, "Sec-WebSocket-Key: ", &keyvaluepair)) {
		strcat(keyvaluepair.value, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
		SHA1Init(&sha1_context);
		SHA1Update(&sha1_context, keyvaluepair.value, strlen(keyvaluepair.value));
		SHA1Final(buffer, &sha1_context);

		// get some mem from nodemcu heap
	    somemem =(char *) os_malloc(8192);
	    // fake rom malloc init
	    mem_init(somemem);


		keyvaluepair.value = base64_encode(buffer, 20, &length);
		strcat(response, keyvaluepair.value);
		strcat(response, "\r\n");

		espconn_send(connection, response, os_strlen(response));

		os_free(somemem);
		os_printf("Handshake done \n");
		return true;
	}
	return false;
}

bool getMessageParameters(char * frame , WebsocketFrame *websocketframe) {
	uint8 opcode;
	long int paylen;
	bool masked;

	websocketframe->framecounter = 0;
	//extract frame details
	websocketframe->fin = frame[0]>>7 & 0x01;
	websocketframe->rsvs = frame[0]>>4 & 0x07;
	websocketframe->opcode = frame[0] & 0x0f;
	websocketframe->masked = frame[1]>>7 & 0x01;

	if (!websocketframe->masked || !websocketframe->fin || websocketframe->rsvs) {
		return false;
	}

	if ((frame[1] & 0x7f) < 126 ) {
		websocketframe->payloadlen = frame[1] & 0x7f;
		websocketframe->framecounter = 2;
	} else if ((frame[1] & 0x7f) == 126) {
		// currently supports 64k frame
		websocketframe->payloadlen = frame[2];
		websocketframe->payloadlen = websocketframe->payloadlen << 8;
		websocketframe->payloadlen |= frame [3];
		websocketframe->framecounter = 4;
	} else {
		return false;
	}

	websocketframe->maskkey[0] = frame[websocketframe->framecounter];
	websocketframe->maskkey[1] = frame[websocketframe->framecounter + 1];
	websocketframe->maskkey[2] = frame[websocketframe->framecounter + 2];
	websocketframe->maskkey[3] = frame[websocketframe->framecounter + 3];
	websocketframe->framecounter += 4;

	return true;
}


bool getMessage(char* frame, WebsocketFrame *websocketframe, char *target) {

	long int i;

	for (i=0; i< websocketframe->payloadlen; i++) {
		target[i] = frame[websocketframe->framecounter + i] ^ websocketframe->maskkey[i%4];
		// os_printf("%c", frame[websocketframe.framecounter + i] ^ websocketframe.maskkey[i%4]);
	}

	return false;
}


bool sendMessagePacket(char * message, WebsocketFrame * websocketframe, struct espconn * connection) {
	int counter;
	int err;
	uint8 *packet;
	int length = 0;
	websocketframe->framecounter = 0;
	length = (websocketframe->payloadlen < 126)? 1: 3;

	length = length + 1 +  websocketframe->payloadlen;
	/*
	1 -> first byte with fin and reserved bits and opcode
	payload length
	data length
	*/
	packet =(uint8*) os_zalloc(sizeof(uint8)*(length+1));
	packet[websocketframe->framecounter] = 0 | (websocketframe->fin <<7);
 	packet[websocketframe->framecounter] |= (websocketframe->opcode);
	websocketframe->framecounter++;

	if(websocketframe->payloadlen <126) {
		packet[websocketframe->framecounter++] = (uint8) websocketframe->payloadlen;
	} else {
		packet[websocketframe->framecounter++] = 126;
		packet[websocketframe->framecounter++] = (uint8) (websocketframe->payloadlen | 0xff);
		packet[websocketframe->framecounter++] = (uint8)((websocketframe->payloadlen>>8) | 0xff);
	}

	for(counter=0; counter < websocketframe->payloadlen; counter++) {
		packet[websocketframe->framecounter++] = message[counter];
	}

	err = espconn_send(connection, (char *)packet, os_strlen(packet));
	if (err) {
		os_printf("Error %d", err);
		return false;
	}
	os_free(packet);
	return true;
}

bool sendMessage(char *message, struct espconn *connection) {
	uint8 failcounter = 0;

	WebsocketFrame websocket;
	websocket.framecounter = 0;
	websocket.fin = 1;
	websocket.rsvs = 0;
	websocket.opcode = 0x01;
	websocket.payloadlen = strlen(message);
	websocket.masked = 0;

	while(!sendMessagePacket(message, &websocket, connection)) {
		if(failcounter > 1) {
			return false;
		}
		failcounter++;
	}

	return true;
}

bool sendPong(struct espconn* connection) {
	uint8 failcounter = 0;
	WebsocketFrame websocket;
	websocket.framecounter = 0;
	websocket.fin = 1;
	websocket.rsvs = 0;
	websocket.payloadlen = 0;
	websocket.opcode = 0x0A;
	websocket.masked = 0;
	sendMessagePacket("", &websocket, connection);

	return true;
};

bool sendPing(struct espconn* connection) {
	uint8 failcounter = 0;
	WebsocketFrame websocket;
	websocket.framecounter = 0;
	websocket.fin = 1;
	websocket.rsvs = 0;
	websocket.payloadlen = 1;
	websocket.opcode = 0x09;
	websocket.masked = 0;
	while(!sendMessagePacket("H", &websocket, connection)) {
		if(failcounter > 5) {
			return false;
		}
		failcounter++;

	}

	return true;
};

bool closeWebsocket(struct espconn* connection) {
	uint8 failcounter = 0;
	WebsocketFrame websocket;
	websocket.framecounter = 0;
	websocket.fin = 1;
	websocket.rsvs = 0;
	websocket.payloadlen = 0;
	websocket.opcode = 0x08;
	websocket.masked = 0;
	while(!sendMessagePacket("", &websocket, connection)) {
		if(failcounter > 5) {
			return false;
		}
		failcounter++;

	}

	return true;
};
