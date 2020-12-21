#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "bool.h"
#include "arrayList.h"
#include "extend.h"

#define RBMS 10000 // Read buff max size
#define RMTS 50 // Regex max template size

static const size_t m_dir_path_size = 100;
static const size_t m_prefix_size = 30;
static const size_t m_file_name_size = 100;

struct Path {
	char* dir_path;
	char* prefix;
	char* file_name;
};

enum meta_data {DATETIME = 1, NAME};

_Bool path_init (struct Path**);
_Bool path_compile (struct Path*, char*);
void path_free (struct Path**);
_Bool get_name (char*, char**);
_Bool filter_delete_garbage (char*, char**, int);
_Bool filter_delete_meta (char*, enum meta_data);
_Bool filter_owner_msg (char*, struct ArrayList_str**, char**);
_Bool filter_wrong_format (char*, char*);
_Bool validateArgs (int, char**);
_Bool readFromStream (char*);
_Bool writeToStream (char*, struct Path*);
void help_screen ();

#endif
