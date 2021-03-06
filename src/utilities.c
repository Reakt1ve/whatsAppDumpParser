#include "utilities.h"

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

_Bool filter_owner_msg (char* __text, struct ArrayList_str **finded_names, char** name) {
        get_name(__text, name);

        char* selected_name = NULL;
        for ( int i = 0; i < arrayList_str_len(*finded_names); i++ ) {
                arrayList_str_get(*finded_names, i, &selected_name);
                if ( strcmp(*name, selected_name) == 0 ) return 1;
        }

        arrayList_str_append(&(*finded_names), *name);

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
        if ( mkdir(path_struct->dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0 && errno != EEXIST ) {
		printf("No sudo permission!\n");
		exit(1);
	}

        char* path = NULL;
        short int path_size = strlen(path_struct->dir_path) +
                                strlen(path_struct->prefix) +
                                strlen(path_struct->file_name) +
                                strlen(path_struct->format);
        if ( !(path = (char*) calloc (path_size + 1, sizeof(char))) ) return 0;

        path_compile(path_struct, path);
        replace_ch(&path, ' ', '-');

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

