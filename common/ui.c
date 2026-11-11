#include "ui.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Lebar tampilan string: byte ASCII = 1, karakter UTF-8 multi-byte = 1
 * kolom per puncak lead byte-nya. */
static int ui_width(const char *s) {
    int w = 0;
    for (; *s != '\0'; s++) {
        w++;
        while (((unsigned char)s[1] & 0xC0) == 0x80) s++;
    }
    return w;
}

/* Mencetak n karakter c di baris saat ini lalu newline. */
static void ui_line(char c) {
    int i;
    for (i = 0; i < UI_WIDTH; i++) putchar(c);
    putchar('\n');
}

/* Satu pesan berawalan prefix dan berakhiran newline. */
static void ui_msg(const char *prefix, const char *fmt, va_list args) {
    printf("%s ", prefix);
    vprintf(fmt, args);
    putchar('\n');
}

/* Bingkai judul pembuka program. */
void ui_banner(const char *title, const char *line1, const char *line2) {
    ui_line('=');
    printf(" %s\n", title != NULL ? title : "");
    printf(" %s\n", line1 != NULL ? line1 : "");
    printf(" %s\n", line2 != NULL ? line2 : "");
    ui_line('=');
}

/* Pembuka layar menu: garis, judul, status, garis. */
void ui_screen_header(const char *title, const char *status) {
    ui_line('=');
    printf(" %s\n", title != NULL ? title : "");
    printf(" %s\n", status != NULL ? status : "");
    ui_line('=');
}

/* Item menu bernomor dengan lebar nomor tetap dua karakter. */
void ui_menu_item(int number, const char *label) {
    printf("%3d) %s\n", number, label != NULL ? label : "");
}

/* Judul bagian hasil diikuti garis tipis. */
void ui_section(const char *title) {
    printf(" %s\n", title != NULL ? title : "");
    ui_line('-');
}

/* Garis pemisah selebar layout. */
void ui_rule(void) {
    ui_line('-');
}

/* Pesan sukses, info, peringatan, dan error. */
void ui_ok(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ui_msg("[OK]", fmt, args);
    va_end(args);
}

void ui_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ui_msg("[i]", fmt, args);
    va_end(args);
}

void ui_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ui_msg("[!]", fmt, args);
    va_end(args);
}

void ui_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ui_msg("[Error]", fmt, args);
    va_end(args);
}

/* Konfirmasi ya/tidak; jawaban kosong memakai bawaan. */
Status ui_confirm(const char *question, int default_yes, int *answer) {
    char buf[UTIL_MAX_LINE];
    if (question == NULL || answer == NULL) return ST_BAD_INPUT;
    for (;;) {
        printf("%s [%s] ", question, default_yes ? "Y/n" : "y/N");
        if (util_read_line(NULL, buf, sizeof(buf)) != ST_OK) return ST_BAD_INPUT;
        util_trim(buf);
        if (buf[0] == '\0') {
            *answer = default_yes;
            return ST_OK;
        }
        if (util_stricmp(buf, "y") == 0 || util_stricmp(buf, "ya") == 0) {
            *answer = 1;
            return ST_OK;
        }
        if (util_stricmp(buf, "n") == 0 || util_stricmp(buf, "tidak") == 0) {
            *answer = 0;
            return ST_OK;
        }
        ui_warn("Jawab y, ya, n, atau tidak; kosong memilih bawaan.");
    }
}

/* Prompt pilihan seragam tanpa newline. */
void ui_prompt_label(void) {
    printf("Pilihan > ");
}

/* Membuang satu pasang kutip di kedua ujung s bila panjangnya cukup. */
void ui_strip_quotes(char *s) {
    size_t len;
    char q;
    if (s == NULL) return;
    len = strlen(s);
    if (len < 2) return;
    if (s[0] != '"' && s[0] != '\'') return;
    q = s[0];
    if (s[len - 1] != q) return;
    s[len - 1] = '\0';
    memmove(s, s + 1, len - 1);
}

/* Membaca path; kosong ditolak dengan ui_warn lalu diulang. */
Status ui_read_path(const char *prompt, char *buf, size_t size) {
    if (prompt == NULL || buf == NULL || size < 2) return ST_BAD_INPUT;
    for (;;) {
        if (util_read_line(prompt, buf, size) != ST_OK) return ST_BAD_INPUT;
        util_trim(buf);
        ui_strip_quotes(buf);
        if (buf[0] != '\0') return ST_OK;
        ui_warn("Path tidak boleh kosong.");
    }
}

/* Mencetak s terpotong: bila lebar muat dicetak utuh, jika tidak dicetak
 * sampai batas max_cols - 3 lalu "..."; potongan mundur ke batas
 * karakter UTF-8 supaya tidak terbelah. */
void ui_print_truncated(const char *s, int max_cols) {
    int total, limit, used = 0;
    size_t cut = 0, i;
    if (s == NULL) return;
    if (max_cols <= 0) return;
    total = ui_width(s);
    if (total <= max_cols) {
        fputs(s, stdout);
        return;
    }
    limit = max_cols - 3;
    for (i = 0; s[i] != '\0'; i++) {
        size_t end = i;
        if (used + 1 > limit) break;
        if (((unsigned char)s[i] & 0x80) != 0) {
            while (((unsigned char)s[end + 1] & 0xC0) == 0x80) end++;
        }
        cut = end + 1;
        used++;
        i = end;
    }
    fwrite(s, 1, cut, stdout);
    fputs("...", stdout);
}
