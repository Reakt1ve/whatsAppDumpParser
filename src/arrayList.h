#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdlib.h>
#include "bool.h"

struct ArrayList_str {
        int size;
        int len;
        char** string_array;
};

static const int ARRAY_LIST_CHUNK = 10;

_Bool arrayList_str_init (struct ArrayList_str**);
void arrayList_set_len (struct ArrayList_str**, char*);
_Bool arrayList_str_append (struct ArrayList_str**, char*);
_Bool arrayList_str_get (struct ArrayList_str*, int, char**);
_Bool arrayList_str_isEmpty (struct ArrayList_str*);
int arrayList_str_len (struct ArrayList_str*);
int arrayList_str_buffer_size (struct ArrayList_str*);
void arrayList_str_free (struct ArrayList_str**);

#endif
