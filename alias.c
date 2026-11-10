#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "alias.h"

typedef struct {
    char name[ALIAS_NAME_LEN];
    char value[ALIAS_VAL_LEN];
    int  used;
} Alias;

static Alias table[ALIAS_MAX];

static int find(const char *name)
{
    for (int i = 0; i < ALIAS_MAX; i++)
        if (table[i].used && strcmp(table[i].name, name) == 0) return i;
    return -1;
}

int alias_set(const char *name, const char *value)
{
    if (!name || !*name || strlen(name) >= ALIAS_NAME_LEN ||
        strlen(value) >= ALIAS_VAL_LEN) return -1;
    if (strpbrk(name, " \t=!")) return -1;

    int idx = find(name);
    if (idx < 0) {
        for (int i = 0; i < ALIAS_MAX; i++)
            if (!table[i].used) { idx = i; break; }
        if (idx < 0) return -1;            /* tabel penuh */
    }
    strcpy(table[idx].name, name);
    strcpy(table[idx].value, value);
    table[idx].used = 1;
    return 0;
}

const char *alias_get(const char *name)
{
    int i = find(name);
    return i < 0 ? NULL : table[i].value;
}

int alias_remove(const char *name)
{
    int i = find(name);
    if (i < 0) return -1;
    table[i].used = 0;
    return 0;
}

void alias_print_one(const char *name)
{
    int i = find(name);
    if (i < 0) fprintf(stderr, "myshell: alias: %s: not found\n", name);
    else       printf("alias %s='%s'\n", table[i].name, table[i].value);
}

void alias_print_all(void)
{
    for (int i = 0; i < ALIAS_MAX; i++)
        if (table[i].used) printf("alias %s='%s'\n", table[i].name, table[i].value);
}

void alias_expand(const char *line, char *out, size_t outsz)
{
    char cur[1024];
    char seen[8][ALIAS_NAME_LEN];
    int  nseen = 0;

    snprintf(cur, sizeof cur, "%s", line);

    for (;;) {
        const char *p = cur;
        while (isspace((unsigned char)*p)) p++;
        size_t wl = strcspn(p, " \t");
        if (wl == 0 || wl >= ALIAS_NAME_LEN) break;

        char word[ALIAS_NAME_LEN];
        memcpy(word, p, wl);
        word[wl] = '\0';

        const char *val = alias_get(word);
        if (!val) break;

        /* cegah loop: alias yang sudah pernah diekspansi tidak diekspansi lagi */
        int loop = 0;
        for (int i = 0; i < nseen; i++)
            if (strcmp(seen[i], word) == 0) { loop = 1; break; }
        if (loop || nseen >= 8) break;
        strcpy(seen[nseen++], word);

        char tmp[1024];
        snprintf(tmp, sizeof tmp, "%s%s", val, p + wl);
        snprintf(cur, sizeof cur, "%s", tmp);
    }
    snprintf(out, outsz, "%s", cur);
}
