#ifndef UTILS
#define UTILS
#include <c_types.h>

typedef struct {
	char *key;
	char *value;
} KEYVALUE_PAIR;

bool isUpgradeable(char *);

bool getKeyPair(char *, char*, KEYVALUE_PAIR *);

#endif
