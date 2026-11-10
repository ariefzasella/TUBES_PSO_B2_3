#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

int parse_line(const char *line, char *argv[])
{
    int argc = 0;
    const char *p = line;

    while (*p && argc < MAX_ARGS - 1) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        char *tok = malloc(strlen(p) + 1);
        size_t len = 0;
        char quote = 0;

        while (*p && (quote || !isspace((unsigned char)*p))) {
            if (quote) {
                if (*p == quote) quote = 0;
                else tok[len++] = *p;
            } else if (*p == '\'' || *p == '"') {
                quote = *p;
            } else {
                tok[len++] = *p;
            }
            p++;
        }
        tok[len] = '\0';
        argv[argc++] = tok;
    }
    argv[argc] = NULL;
    return argc;
}

void free_args(char *argv[])
{
    for (int i = 0; argv[i]; i++) { free(argv[i]); argv[i] = NULL; }
}
