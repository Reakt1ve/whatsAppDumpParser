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
#define WBMS 4000 // Write buff max size
#define RMTS 50 // Regex max template size

#define TRUE 1
#define FALSE 0

struct StringPosition {
	int begin;
	int end;
};

struct ArrayList_str {
        char** string_array;
        int size;
        int len;
};

const int ARRAY_LIST_CHUNK = 2;

_Bool arrayList_str_init (struct ArrayList_str* list) {
        list->size = ARRAY_LIST_CHUNK;
        list->len = 0;
        list->string_array = (char**) calloc (ARRAY_LIST_CHUNK, sizeof(char*));

        return 1;
}

void arrayList_set_len (struct ArrayList_str* list, char* value) {
        list->len++;
        list->string_array[list->len - 1] = value;
}

_Bool arrayList_str_append (struct ArrayList_str* list, char* value) {
        if ( list->len >= list->size ) {
                list->size += ARRAY_LIST_CHUNK;
                if ( !(list->string_array = (char**) realloc (list->string_array, list->size * sizeof(char*))) )
                        return 0;
                arrayList_set_len(list, value);
        } else arrayList_set_len (list, value);

        return 1;
}

_Bool arrayList_str_get (struct ArrayList_str* list, int index, char** element) {
        if ( list->len == 0) return 0;
	if ( index > list->len ) return 0;
        else element[0] = list->string_array[index];

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

void arrayList_str_free (struct ArrayList_str *list) {
        free(list->string_array);
        free(list);
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

	//printf("%d %d", str_pos->begin, str_pos->end);
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

	if ( !(name[0] = (char*) calloc (str_pos.end - str_pos.begin - end_offset, sizeof(char))) ) return 0;
        if ( strncpy(name[0], &__text[str_pos.begin], str_pos.end - str_pos.begin - end_offset) == FALSE ) return 0;
	//printf("NAME: %s\n", name[0]);

        return 1;
}

/*_Bool filter_delete_companion_msg (char* __text, char* target) {
	char** name = NULL;
	if ( !(name = (char**) malloc (sizeof(char*))) );
        if ( get_name(__text, name) == FALSE ) return 0;
	//printf("EXCEPT: %s\n", name[0]);

        if ( strcmp(name[0], target) == FALSE ) return 1;

	return 0;
}*/

_Bool filter_delete_garbage (char* __text, char** garbage_list, int garbage_list_size) {
	struct StringPosition str_pos;

        for ( int i = 0; i < garbage_list_size; i++ ) {
		//printf("PATTERN: %s\n", garbage_list[i]);
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
	//printf("NAME_2: %s\n", name[0]);

	char** selected_name = NULL;
	if ( !(selected_name = (char**) malloc (sizeof(char*))) ) return 0;

	for ( int i = 0; i < arrayList_str_len(finded_names); i++ ) {
		arrayList_str_get(finded_names, i, selected_name);
		if ( strcmp(name[0], selected_name[0]) == 0 ) {
			free(selected_name);
			return 1;
		}
	}

	arrayList_str_append(finded_names, name[0]);
	free(selected_name);

	return 1;
}

_Bool filter_wrong_format (char* __text, char* etalon) {
	struct StringPosition str_pos;
	if (substr(__text, &str_pos, etalon) == FALSE) return 0;

	return 1;
}

_Bool readFromStream (char* buffer) {
        int s_char, idx = 0;
	memset(buffer, 0, WBMS);

        while ( (s_char = getchar()) > 0 ) {
                buffer[idx] = (char) s_char;
                idx++;

                if ( (s_char == '\n') || (idx + 1 == RBMS) ) return 1;
        }

        return 0;
}

_Bool writeToStream (char* buffer, char* path_name) {
	char* home_dir = "./out.d/";
	if (mkdir(home_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0 && errno != EEXIST) return 0;

	int path_format = 12;
	char* path = (char*) calloc (strlen(path_name) + path_format + 1, sizeof(char));
	strcat(path, home_dir);
	strcat(path, path_name);
	strcat(path, ".txt");

	//printf("PATH: %s\n", path);

	FILE *file = fopen(path, "a");
	fprintf(file, buffer);
	fclose(file);

	free(path);
	return 1;
}

int main(void) {
	setlocale(LC_ALL, "");
	/*char str[14][500] = {
				"13/03/2020, 10:01 - Тима Коллектор: Messages and calls are end-to-end encrypted. No one outside of this, not even WhatsApp, can read or listen to them. Tap to learn more.",
				"13/03/2020, 12:20 - Тима Коллектор: Missed voice call",
				"13.03.20, 12:21 - Владик: What's the money?",
				"13/03/2020, 12:25 - Тима Коллектор: <Media omitted>",
				"13/03/2020, 12:30 - +7 985 721-72-72: <Media omitted>",
				"13/03/2020, 12:31 - Тима Коллектор: Do you know how much cost kidneys?",
				"13/03/2020, 17:24 - Владик: Missed voice call",
				"13/03/2020, 17:38 - Владик: <Без медиафайлов>",
				"13/03/2020, 17:45 - Тима Коллектор: sdfdsgrgfg",
				"14/03/2020, 19:02 - Владик: You deleted this message",
				"14/03/2020, 21:21 - Тима Коллектор: Пропущенный голосовой вызов",
				"17/03/2020, 16:00 - Тима Коллектор: i'am coming for you",
				"18/03/2020, 21:22 - +7 925 787-74-53: Test",
				"18/03/2020, 21:26 - +7 925 744-33-22: Hello, guys"
			   };*/

	char* garbage_list[100] = {
					"You deleted this message",
					"Media omitted",
                                        "Missed voice call",
                                        "Пропущенный голосовой вы",
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
	arrayList_str_init(finded_names);

	char** name =  NULL;
	if ( !(name = (char**) malloc (sizeof(char*))) ) return 0;

	char** test = (char**) malloc (sizeof(char*));

	_Bool isReadable = FALSE;
	while ( isReadable = readFromStream(str) == TRUE ) {
	//for (int i = 0; i < 14; i++) {
		//if ( filter_delete_companion_msg (str, argv[1]) == FALSE ) continue;
		if ( filter_wrong_format(str,
			"[0-2]{1}[0-9]{1}[/.]{1}[0-2]{1}[0-9]{1}[/.]{1}([0-9]{4}|[0-9]{2}),\\s[0-2]{1}[0-9]{1}:[0-9]{1}[0-9]{1}+\\s-\\s[a-zA-Zа-яА-Я0-9:,.; -+]+:") == FALSE)
			continue;
		if ( filter_owner_msg(str, finded_names, name) == FALSE) continue;
		if ( filter_delete_meta(str, DATETIME) == FALSE ) continue;
		if ( filter_delete_meta(str, NAME) == FALSE ) continue;
		if ( filter_delete_garbage(str, garbage_list, 10) == FALSE ) continue;
		//printf("STRING: %s\n", str[i]);
		//write(STDOUT_FILENO, str, strlen(str));
		writeToStream (str, name[0]);
	}

	//for(int i = 0; i < arrayList_str_len(finded_names); i++) {
        //                arrayList_str_get(finded_names, i, test);
        //                printf("NAME: %s\n", test[0]);
        //}


	arrayList_str_free(finded_names);

        return 0;
}
