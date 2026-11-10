/*
 * MyShell - Custom Linux Shell (POSIX)
 * Tugas Besar Proyek Sistem Operasi (INF-201) - Kelompok 3
 * Topik: History & Command Alias
 *
 * Fitur : history (!!, !N, !-N, !awalan), riwayat persisten (.myshell_history),
 *         alias / unalias, cd, help, exit.
 * POSIX : fork(), execvp(), waitpid(), chdir(), getcwd(), signal()
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include "history.h"
#include "alias.h"
#include "parser.h"

#define LINE_MAX_LEN 1024

static void trim(char *s)
{
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
}

/* ------------------------- Built-in commands ------------------------- */

static void builtin_help(void)
{
    puts("MyShell - Kelompok 3 (History & Alias)\n"
         "  history          tampilkan riwayat perintah\n"
         "  history -c       hapus seluruh riwayat\n"
         "  !!  !N  !-N  !x  jalankan ulang (terakhir / nomor N / N dari belakang / awalan x)\n"
         "  alias            tampilkan semua alias\n"
         "  alias nama       tampilkan satu alias\n"
         "  alias n='cmd'    buat alias, contoh: alias ll='ls -la'\n"
         "  unalias nama     hapus alias\n"
         "  cd [dir]         pindah direktori\n"
         "  help             bantuan ini\n"
         "  exit             keluar dari shell");
}

static void builtin_alias(char *argv[])
{
    if (!argv[1]) { alias_print_all(); return; }
    for (int i = 1; argv[i]; i++) {
        char *eq = strchr(argv[i], '=');
        if (!eq) { alias_print_one(argv[i]); continue; }
        *eq = '\0';
        if (alias_set(argv[i], eq + 1) != 0)
            fprintf(stderr, "myshell: alias: gagal membuat alias '%s'\n", argv[i]);
    }
}

static void builtin_unalias(char *argv[])
{
    if (!argv[1]) { fprintf(stderr, "unalias: usage: unalias nama\n"); return; }
    for (int i = 1; argv[i]; i++)
        if (alias_remove(argv[i]) != 0)
            fprintf(stderr, "myshell: unalias: %s: not found\n", argv[i]);
}

static void builtin_cd(char *argv[])
{
    const char *dir = argv[1] ? argv[1] : getenv("HOME");
    if (!dir || chdir(dir) != 0) perror("myshell: cd");
}

/* Return 1 jika perintah adalah built-in dan sudah dijalankan */
static int run_builtin(char *argv[])
{
    if (strcmp(argv[0], "exit") == 0)    { exit(0); }
    if (strcmp(argv[0], "cd") == 0)      { builtin_cd(argv);      return 1; }
    if (strcmp(argv[0], "help") == 0)    { builtin_help();        return 1; }
    if (strcmp(argv[0], "alias") == 0)   { builtin_alias(argv);   return 1; }
    if (strcmp(argv[0], "unalias") == 0) { builtin_unalias(argv); return 1; }
    if (strcmp(argv[0], "history") == 0) {
        if (argv[1] && strcmp(argv[1], "-c") == 0) history_clear();
        else                                       history_print();
        return 1;
    }
    return 0;
}

/* ----------------------- Eksekusi program eksternal ----------------------- */

static void run_external(char *argv[])
{
    pid_t pid = fork();
    if (pid < 0) { perror("myshell: fork"); return; }

    if (pid == 0) {                         /* proses anak */
        signal(SIGINT, SIG_DFL);
        execvp(argv[0], argv);
        fprintf(stderr, "myshell: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    int status;                             /* proses induk */
    if (waitpid(pid, &status, 0) < 0) perror("myshell: waitpid");
}

/* ------------------------------- main loop ------------------------------- */

int main(void)
{
    char line[LINE_MAX_LEN], expanded[LINE_MAX_LEN], aliased[LINE_MAX_LEN];
    char cwd[1024];
    char *argv[MAX_ARGS];

    signal(SIGINT, SIG_IGN);                /* Ctrl+C tidak mematikan shell */
    history_load();

    for (;;) {
        if (!getcwd(cwd, sizeof cwd)) strcpy(cwd, "?");
        printf("myshell:%s> ", cwd);
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) { putchar('\n'); break; }   /* Ctrl+D */
        trim(line);
        if (!line[0]) continue;

        /* 1. ekspansi riwayat (!!, !N, ...) */
        int r = history_expand(line, expanded, sizeof expanded);
        if (r < 0) continue;
        if (r > 0) { printf("%s\n", expanded); snprintf(line, sizeof line, "%s", expanded); }

        /* 2. simpan ke riwayat (bentuk setelah ekspansi riwayat) */
        history_add(line);

        /* 3. ekspansi alias */
        alias_expand(line, aliased, sizeof aliased);

        /* 4. tokenisasi & eksekusi */
        if (parse_line(aliased, argv) == 0) { free_args(argv); continue; }
        if (!run_builtin(argv)) run_external(argv);
        free_args(argv);
    }
    return 0;
}
