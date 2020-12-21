#ifndef EXTEND_H
#define EXTEND_H

#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "bool.h"

struct StringPosition {
	int begin;
	int end;
};

_Bool replace_ch (char**, char, char);
_Bool delsub (char*, char*);
_Bool substr (char*, struct StringPosition*, const char*);

#endif
