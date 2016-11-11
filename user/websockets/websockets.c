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
