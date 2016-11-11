#include<string.h>
#include "utils.h"
#include "osapi.h"
#include<mem.h>

bool isUpgradeable(char *header) {
	KEYVALUE_PAIR keyvaluepair;
	bool status;
	char key[20];
	char value[100];
	keyvaluepair.key = key;
	keyvaluepair.value = value;

	if (getKeyPair(header, "Upgrade: ", &keyvaluepair)) {
		if ( strcmp(keyvaluepair.value, "Upgrade"))
			return true;
	} else {
		return false;
	}
}

bool getKeyPair(char *src, char *key, KEYVALUE_PAIR * keyvaluepair) {
		//Not safe implementation.
		char *header_cpy;
		char *value;
		header_cpy = src;

		value = strstr(header_cpy, key); //extract the line
		if (value) {
			value = strtok(value, "\r\n"); // tokenize by /r/n
			value = strstr(value, ":"); // find the key
			value++;
			if(*value == ' ')
				value++;
		} else {
			keyvaluepair->key = NULL;
			keyvaluepair->value = NULL;
			return false;
		}
		strcpy(keyvaluepair->key, key);
		strcpy(keyvaluepair->value,value);
		return true;
}
