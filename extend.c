#include "extend.h"

_Bool replace_ch (char** __text, char __old, char __new) {
        for ( int i = 0; (*__text)[i] != '\0'; i++) {
                if ( (*__text)[i] == __old) (*__text)[i] = __new;
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
