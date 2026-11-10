#ifndef ALIAS_H
#define ALIAS_H

#include <stddef.h>

#define ALIAS_MAX      64
#define ALIAS_NAME_LEN 64
#define ALIAS_VAL_LEN  256

/* ---- Alias Manager (Anggota 3) ---- */
int         alias_set(const char *name, const char *value);   /* 0 ok, -1 penuh/invalid */
const char *alias_get(const char *name);                      /* NULL jika tidak ada    */
int         alias_remove(const char *name);                   /* 0 ok, -1 tidak ada     */
void        alias_print_all(void);
void        alias_print_one(const char *name);

/* Ganti kata pertama pada `line` bila merupakan alias (mendukung alias bertingkat,
 * dengan pelindung rekursi tak hingga). Hasil ditulis ke `out`. */
void        alias_expand(const char *line, char *out, size_t outsz);

#endif
