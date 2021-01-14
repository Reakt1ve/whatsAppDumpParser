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

        return 1;
}

_Bool path_compile (struct Path* path_struct, char* path) {
        if ( strcat(path, path_struct->dir_path) == FALSE ) return 0;
        if ( strcat(path, path_struct->prefix) == FALSE ) return 0;
        if ( strcat(path, path_struct->file_name) == FALSE ) return 0;

        return 1;
}

void path_free (struct Path** path_struct) {
        free((*path_struct)->dir_path);
        free((*path_struct)->prefix);
        free((*path_struct)->file_name);
        free(*path_struct);
}

_Bool get_name (char* __text, char** name) {
        char* pattern = "(([()a-zA-Zа-яА-Я ]+)+|([+]{1}7\\s[0-9]{3}\\s[0-9]{3}-[0-9]{2}-[0-9]{2}))+:\\s";
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
                        pattern = "(([()a-zA-Zа-яА-Я ]+)+|([+]{1}7\\s[0-9]{3}\\s[0-9]{3}-[0-9]{2}-[0-9]{2}))+:\\s";
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

_Bool validateArgs (int argc, char** argv) {
        if (argc == 3) {
		char* dir_regex = "^(\\/((\\\\b)|[^ \b%|:\n\"/])+)+\\/$";
		struct StringPosition str_pos;

		if ( substr(argv[1], &str_pos, dir_regex) == FALSE ) return 0;
	} else return 0;

	return 1;
}

_Bool createDirectory (const char* dir_path, const int permissions, char** error) {
	*error = NULL;
	if ( !(*error = (char*) calloc (m_dir_err_size + 1, sizeof(char))) ) return 0;

	mkdir(dir_path, permissions);
        switch ( errno ) {
                case EPERM : {
                        strcpy(*error, "Файловая система, содержащая pathname, не поддерживает создание каталогов");
                        break;
                }
                case EFAULT : {
                        strcpy(*error, "pathname указывает за пределы доступного адресного пространства");
                        break;
                }
                case EACCES : {
                        strcpy(*error, "Родительский каталог не позволяет запись, или же один из каталогов, перечисленных в pathname, не позволяет поиск (выполнение)");
                        break;
                }
                case ENOENT : {
                        strcpy(*error, "Компонент пути pathname не существует или является висячей символической ссылкой");
                        break;
                }
                case ENOTDIR : {
                        strcpy(*error, "Компонент пути, использованный как каталог в pathname, в действительности таковым не является");
                        break;
                }
                case ENOMEM : {
                        strcpy(*error, "Ядру не хватило памяти");
                        break;
		}
                case EROFS : {
                        strcpy(*error, "Файл находится на файловой системе, смонтированной только для чтения");
                        break;
                }
                case ELOOP : {
                        strcpy(*error, "pathname является зацикленной символической ссылкой, то есть при подстановке возникает ссылка на неё саму");
                        break;
                }
                case ENOSPC : {
                        strcpy(*error, "На устройстве, содержащем pathname, нет места для создания новой записи в каталоге. Это может произойти также, если исчерпана квота дискового пространства пользователя");
                        break;
                }
                default : return 1;
	}

	return 0;
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
	char* error;

	if ( !createDirectory (path_struct->dir_path, 0777, &error) ) {
		printf("Ошибка папки: %s\n", error);
		exit(1);
	}

        char* path = NULL;
        short int path_size = strlen(path_struct->dir_path) +
                                strlen(path_struct->prefix) +
                                strlen(path_struct->file_name);
        if ( !(path = (char*) calloc (path_size + 1, sizeof(char))) ) return 0;

        path_compile(path_struct, path);
        replace_ch(&path, ' ', '-');

	FILE *file = NULL;
        if ( !(file = fopen(path, "a")) ) {
		printf("Ошибка файла: в доступе отказано\n");
		exit(1);
	}

        fprintf(file, buffer);
        fclose(file);

        free(path);
	free(file);

        return 1;
}

void help_screen () {
        printf("Неверный синтаксис!!!\n");
        printf("Формат: [FILE] | ./parser [DIR_PATH] [PREFIX]\n");
	printf("Примечание: DIR_PATH верный формат - /path/to/dir/\n");
}

