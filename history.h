#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

#define HISTORY_MAX   1000
#define HISTORY_FILE  ".myshell_history"

/* ---- History File Manager (Anggota 1) ---- */
void history_load(void);               /* baca riwayat dari berkas saat shell start   */
void history_save_line(const char *l); /* tambahkan satu baris ke berkas (append)     */
void history_rewrite(void);            /* tulis ulang seluruh berkas (dipakai -c)     */
void history_add(const char *line);    /* tambah ke memori + berkas                   */
void history_print(void);              /* perintah built-in: history                  */
void history_clear(void);              /* perintah built-in: history -c               */

/* ---- History Executor (Anggota 2) ---- */
/* Mengubah !!, !N, !-N, !awalan menjadi perintah sebenarnya.
 * Return: 0 = bukan ekspansi, 1 = berhasil diekspansi, -1 = gagal (pesan error dicetak). */
int history_expand(const char *in, char *out, size_t outsz);

#endif
