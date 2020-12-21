#include "arrayList.h"

_Bool arrayList_str_init (struct ArrayList_str** list) {
        (*list)->size = ARRAY_LIST_CHUNK;
        (*list)->len = 0;
        (*list)->string_array = (char**) calloc (ARRAY_LIST_CHUNK, sizeof(char*));

        return 1;
}

void arrayList_set_len (struct ArrayList_str** list, char* value) {
        (*list)->len++;
        (*list)->string_array[(*list)->len - 1] = value;
}

_Bool arrayList_str_append (struct ArrayList_str** list, char* value) {
        if ( (*list)->len >= (*list)->size ) {
                (*list)->size += ARRAY_LIST_CHUNK;
                if ( !((*list)->string_array = (char**) realloc ((*list)->string_array, (*list)->size * sizeof(char*))) )
                        return 0;
                arrayList_set_len(list, value);
        } else arrayList_set_len(list, value);

        return 1;
}

_Bool arrayList_str_get (struct ArrayList_str* list, int index, char** element) {
        if ( list->len == 0) return 0;
        if ( index > list->len ) return 0;
        else  *element = list->string_array[index];
        return 1;
}

_Bool arrayList_str_isEmpty (struct ArrayList_str* list) {
        if (list->len == 0) return 1;
        else return 0;
}

int arrayList_str_len (struct ArrayList_str* list) {
        return list->len;
}

int arrayList_str_buffer_size (struct ArrayList_str* list) {
        return list->size;
}

void arrayList_str_free (struct ArrayList_str** list) {
        free((*list)->string_array);
        free(*list);
}
