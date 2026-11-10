#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include "history.h"

static char *hist[HISTORY_MAX];
static int   hist_count = 0;
static char  hist_path[PATH_MAX];

static void build_path(void)
{
    const char *home = getenv("HOME");
    if (home && *home)
        snprintf(hist_path, sizeof hist_path, "%s/%s", home, HISTORY_FILE);
    else
        snprintf(hist_path, sizeof hist_path, "./%s", HISTORY_FILE);
}

/* Buang entri tertua jika memori penuh */
static void push_memory(const char *line)
{
    if (hist_count == HISTORY_MAX) {
        free(hist[0]);
        memmove(hist, hist + 1, sizeof(char *) * (HISTORY_MAX - 1));
        hist_count--;
    }
    hist[hist_count++] = strdup(line);
}

/* ===================== History File Manager ===================== */

void history_load(void)
{
    char buf[1024];
    build_path();
    FILE *fp = fopen(hist_path, "r");
    if (!fp) return;                       /* belum ada berkas: tidak masalah */
    while (fgets(buf, sizeof buf, fp)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (buf[0]) push_memory(buf);
    }
    fclose(fp);
}

void history_save_line(const char *line)
{
    FILE *fp = fopen(hist_path, "a");
    if (!fp) { perror("myshell: gagal menulis riwayat"); return; }
    fprintf(fp, "%s\n", line);
    fclose(fp);
}

void history_rewrite(void)
{
    FILE *fp = fopen(hist_path, "w");
    if (!fp) { perror("myshell: gagal menulis riwayat"); return; }
    for (int i = 0; i < hist_count; i++) fprintf(fp, "%s\n", hist[i]);
    fclose(fp);
}

void history_add(const char *line)
{
    if (!line || !*line) return;
    /* hindari duplikat berurutan */
    if (hist_count > 0 && strcmp(hist[hist_count - 1], line) == 0) return;
    push_memory(line);
    history_save_line(line);
}

void history_print(void)
{
    for (int i = 0; i < hist_count; i++)
        printf("%5d  %s\n", i + 1, hist[i]);
}

void history_clear(void)
{
    for (int i = 0; i < hist_count; i++) free(hist[i]);
    hist_count = 0;
    history_rewrite();
}

/* ======================== History Executor ======================== */

int history_expand(const char *in, char *out, size_t outsz)
{
    const char *p = in;
    while (isspace((unsigned char)*p)) p++;
    if (*p != '!') return 0;

    /* ambil designator sampai spasi */
    char desig[256];
    size_t n = strcspn(p, " \t");
    if (n >= sizeof desig) n = sizeof desig - 1;
    memcpy(desig, p, n);
    desig[n] = '\0';
    const char *rest = p + n;              /* argumen tambahan setelah !N */

    if (desig[1] == '\0') return 0;        /* "!" sendirian bukan ekspansi */

    const char *found = NULL;

    if (strcmp(desig, "!!") == 0) {                       /* perintah terakhir */
        if (hist_count == 0) { fprintf(stderr, "myshell: !!: event not found\n"); return -1; }
        found = hist[hist_count - 1];
    } else if (isdigit((unsigned char)desig[1])) {         /* !N */
        int num = atoi(desig + 1);
        if (num < 1 || num > hist_count) {
            fprintf(stderr, "myshell: %s: event not found\n", desig);
            return -1;
        }
        found = hist[num - 1];
    } else if (desig[1] == '-' && isdigit((unsigned char)desig[2])) { /* !-N */
        int back = atoi(desig + 2);
        if (back < 1 || back > hist_count) {
            fprintf(stderr, "myshell: %s: event not found\n", desig);
            return -1;
        }
        found = hist[hist_count - back];
    } else {                                               /* !awalan */
        const char *prefix = desig + 1;
        size_t pl = strlen(prefix);
        for (int i = hist_count - 1; i >= 0; i--)
            if (strncmp(hist[i], prefix, pl) == 0) { found = hist[i]; break; }
        if (!found) { fprintf(stderr, "myshell: %s: event not found\n", desig); return -1; }
    }

    snprintf(out, outsz, "%s%s", found, rest);
    return 1;
}
