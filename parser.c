#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include <locale.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define RBMS 4000 // Read buff max size
#define RMTS 50 // Regex max template size

#define TRUE 1
#define FALSE 0

const size_t m_dir_path_size = 100;
const size_t m_prefix_size = 30;
const size_t m_file_name_size = 100;
const size_t m_format_size = 5;

struct Path {
	char* dir_path;
	char* prefix;
	char* file_name;
	char* format;
};

_Bool path_init (struct Path** path_struct) {
	(*path_struct)->dir_path = NULL;
	(*path_struct)->dir_path = (char*) calloc (m_dir_path_size, sizeof(char));
	if ( !((*path_struct)->dir_path) ) return 0;

	(*path_struct)->prefix = NULL;
	(*path_struct)->prefix = (char*) calloc (m_prefix_size, sizeof(char));
	if ( !((*path_struct)->prefix) ) return 0;

	(*path_struct)->file_name = NULL;
	(*path_struct)->file_name = (char*) calloc (m_file_name_size, sizeof(char));
	if ( !((*path_struct)->file_name) ) return 0;

	(*path_struct)->format = NULL;
	(*path_struct)->format = (char*) calloc (m_format_size, sizeof(char));
	if ( !((*path_struct)->format) ) return 0;

	return 1;
}

_Bool path_compile (struct Path* path_struct, char* path) {
	if ( strcat(path, path_struct->dir_path) == FALSE ) return 0;
	if ( strcat(path, path_struct->prefix) == FALSE ) return 0;
	if ( strcat(path, path_struct->file_name) == FALSE ) return 0;
	if ( strcat(path, path_struct->format) == FALSE ) return 0;

	return 1;
}

void path_free (struct Path** path_struct) {
	free((*path_struct)->dir_path);
	free((*path_struct)->prefix);
	free((*path_struct)->file_name);
	free((*path_struct)->format);
	free(*path_struct);
}

struct StringPosition {
	int begin;
	int end;
};

struct ArrayList_str {
        int size;
        int len;
	char** string_array;
};

const int ARRAY_LIST_CHUNK = 10;

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

_Bool replace_ch (char __text[], char __old, char __new) {
        for ( int i = 0; __text[i] != '\0'; i++) {
                if ( __text[i] == __old) __text[i] = __new;
        }

        return 1;
}


_Bool delsub (char* __text, char* substr) {
        char *p, *q, *r;
        if ((q = r = strstr(__text, substr)) != NULL) {
                size_t len = strlen(substr);
                while ((r = strstr(p = r + len, substr)) != NULL) {
                        while (p < r)
                                *q++ = *p++;
                }
                while ((*q++ = *p++) != '\0')
                continue;
       }

	return 1;
}

_Bool substr (char* __text, struct StringPosition* str_pos, const char* __regex) {
        regex_t regex;
        regmatch_t *regmatch = NULL;
	if ( !(regmatch = (regmatch_t*) malloc (sizeof(regmatch_t))) ) return 0;

        char* pattern = NULL;
	if ( !(pattern = (char*) calloc (strlen(__regex) + 1, sizeof(char))) ) return 0;
	strcpy(pattern, __regex);

        if ( regcomp(&regex, pattern, REG_EXTENDED) != 0 ) return 0;
        if ( regexec(&regex, __text, 1, regmatch, 0) == REG_NOMATCH ) return 0;
        else {
		str_pos->begin = regmatch[0].rm_so;
		str_pos->end = regmatch[0].rm_eo;
        }

        free(regmatch);
	free(pattern);
        regfree(&regex);

        return 1;
}

_Bool get_name (char* __text, char** name) {
        char* pattern = "((([a-zA-Zа-яА-Я]+)(\\s[a-zA-Zа-яА-Я]+)?)|([+]{1}7\\s[0-9]{3}\\s[0-9]{3}-[0-9]{2}-[0-9]{2})):\\s";
        int end_offset = 2;
	struct StringPosition str_pos;

        if ( substr(__text, &str_pos, pattern) == FALSE ) return 0;

	if ( !(*name = (char*) calloc (str_pos.end - str_pos.begin - end_offset, sizeof(char))) ) return 0;
        if ( strncpy(*name, &__text[str_pos.begin], str_pos.end - str_pos.begin - end_offset) == FALSE ) return 0;

        return 1;
}

_Bool filter_delete_garbage (char* __text, char** garbage_list, int garbage_list_size) {
	struct StringPosition str_pos;

        for ( int i = 0; i < garbage_list_size; i++ ) {
                if ( substr(__text, &str_pos, garbage_list[i]) == TRUE ) return 0;
   	}

	return 1;
}

enum meta_data {DATETIME = 1, NAME};
_Bool filter_delete_meta (char* __text, enum meta_data meta) {
        struct StringPosition str_pos;
	char* pattern = NULL;

	switch (meta) {
		case DATETIME: {
			pattern = "[ 0-9/,.:]+\\s-\\s";
			break;
		}
		case NAME: {
			pattern = "((([a-zA-Zа-яА-Я]+)(\\s[a-zA-Zа-яА-Я]+)?)|([+]{1}7\\s[0-9]{3}\\s[0-9]{3}-[0-9]{2}-[0-9]{2})):\\s";
			break;
		}
		default: return 0;
	}

        if ( substr(__text, &str_pos, pattern) == FALSE ) return 0;
	char* sub = NULL;
	if ( !(sub = (char*) calloc (str_pos.end - str_pos.begin, sizeof(char))) ) return 0;
	strncpy(sub, &__text[str_pos.begin], str_pos.end - str_pos.begin);
        if ( delsub(__text, sub) == FALSE ) return 0;

        return 1;
}

_Bool filter_owner_msg (char* __text, struct ArrayList_str *finded_names, char** name) {
	get_name(__text, name);

	char* selected_name = NULL;
	for ( int i = 0; i < arrayList_str_len(finded_names); i++ ) {
		arrayList_str_get(finded_names, i, &selected_name);
		if ( strcmp(*name, selected_name) == 0 ) return 1;
	}

	arrayList_str_append(&finded_names, *name);

	return 1;
}

_Bool filter_wrong_format (char* __text, char* etalon) {
	struct StringPosition str_pos;
	if (substr(__text, &str_pos, etalon) == FALSE) return 0;

	return 1;
}

_Bool validateArgs (int argc) {
	if (argc != 1) return 0;
}

_Bool readFromStream (char* buffer) {
        int s_char, idx = 0;
	memset(buffer, 0, RBMS);

        while ( (s_char = getchar()) > 0 ) {
                buffer[idx] = (char) s_char;
                idx++;

                if ( (s_char == '\n') || (idx + 1 == RBMS) ) return 1;
        }

        return 0;
}

_Bool writeToStream (char* buffer, struct Path* path_struct) {
	if ( mkdir(path_struct->dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0 && errno != EEXIST ) return 0;

	char* path = NULL;
	short int path_size = strlen(path_struct->dir_path) +
				strlen(path_struct->prefix) +
				strlen(path_struct->file_name) +
				strlen(path_struct->format);
	if ( !(path = (char*) calloc (path_size + 1, sizeof(char))) ) return 0;

	path_compile(path_struct, path);
	replace_ch(path, ' ', '-');

	FILE *file = fopen(path, "a");
	fprintf(file, buffer);
	fclose(file);

	free(path);
	return 1;
}

void help_screen () {
	printf("Wrong syntax!!!\n");
	printf("Format: [FILE] | ./parser [PREFIX]\n");
}

int main(int argc, char** argv) {
	if ( validateArgs (argc) ) {
		help_screen();
		return 0;
	}

	setlocale(LC_ALL, "");

	char* garbage_list[100] = {
					"You deleted this message",
					"Media omitted",
                                        "Missed voice call",
                                        "Пропущенный голосовой вызов",
                                        "Без медиафайлов",
                                        "Messages and calls are end-to-end",
                                        "Сообщения и звонки защищены сквозным шифрованием",
					"This message was deleted",
					"file attached",
					"Данное сообщение удалено"
			        };

	char str[RBMS] = "";
	struct ArrayList_str* finded_names = NULL;
	if ( !(finded_names = (struct ArrayList_str*) malloc (sizeof(struct ArrayList_str))) )
		return 0;
	arrayList_str_init(&finded_names);

	char* name = NULL;

	struct Path *path_struct = NULL;
	if ( !(path_struct = (struct Path*) malloc (sizeof(struct Path))) ) return 0;
	path_init(&path_struct);

	strcpy(path_struct->dir_path, "./out.d/");
	strcpy(path_struct->prefix, argv[1]);
	strcpy(path_struct->format, ".txt");

	_Bool isReadable = FALSE;
	while ( isReadable = readFromStream(str) == TRUE ) {
		if ( filter_wrong_format(str,
			"[0-2]{1}[0-9]{1}[/.]{1}[0-2]{1}[0-9]{1}[/.]{1}([0-9]{4}|[0-9]{2}),\\s[0-2]{1}[0-9]{1}:[0-9]{1}[0-9]{1}+\\s-\\s[a-zA-Zа-яА-Я0-9:,.; -+]+:") == FALSE)
			continue;
		if ( filter_owner_msg(str, finded_names, &name) == FALSE) continue;
		if ( filter_delete_meta(str, DATETIME) == FALSE ) continue;
		if ( filter_delete_meta(str, NAME) == FALSE ) continue;
		if ( filter_delete_garbage(str, garbage_list, 10) == FALSE ) continue;

		path_struct->file_name = name;
		writeToStream(str, path_struct);
	}

	path_free(&path_struct);
	arrayList_str_free(&finded_names);

        return 0;
}
