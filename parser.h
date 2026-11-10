#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64

/* Memecah baris menjadi argv (mendukung tanda kutip ' dan ").
 * Mengembalikan jumlah token; argv[n] = NULL. Panggil free_args() setelah dipakai. */
int  parse_line(const char *line, char *argv[]);
void free_args(char *argv[]);

#endif
