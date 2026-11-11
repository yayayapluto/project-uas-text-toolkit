#include "ui.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

static int plain_mode = 0;
static int tty_out = 0;
static int tty_in = 0;
static int escape_ok = 0;

/* Aktifkan ENABLE_VIRTUAL_TERMINAL_PROCESSING di Windows agar escape
 * ANSI untuk layar bersih berfungsi; gagal diam-diam. */
#ifdef _WIN32
static void ui_enable_vt(void) {
    DWORD mode = 0;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode)) return;
    if (!SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return;
    escape_ok = 1;
}
#endif

void ui_init(void) {
    plain_mode = getenv("NO_CLEAR") != NULL;
    escape_ok = 0;
#ifdef _WIN32
    tty_out = _isatty(_fileno(stdout)) != 0;
    tty_in = _isatty(_fileno(stdin)) != 0;
    if (tty_out) ui_enable_vt();
#else
    tty_out = isatty(fileno(stdout)) != 0;
    tty_in = isatty(fileno(stdin)) != 0;
#endif
}

void ui_set_plain(int plain) {
    plain_mode = plain ? 1 : 0;
}

int ui_is_interactive(void) {
    return tty_out && tty_in && !plain_mode;
}

void ui_clear(void) {
    if (!ui_is_interactive()) return;
    if (escape_ok) {
        fputs("\033[2J\033[H", stdout);
        fflush(stdout);
        return;
    }
    {
        int i;
        for (i = 0; i < 40; i++) putchar('\n');
        fflush(stdout);
    }
}

void ui_pause(void) {
    char buf[2];
    if (!ui_is_interactive()) return;
    printf("Tekan Enter untuk kembali...");
    fflush(stdout);
    (void)util_read_line(NULL, buf, sizeof buf);
}

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

/* Menyalin s terpotong ke out (size-1 byte maksimal) dengan akhiran "..."
 * bila terpotong; potongan mundur ke batas karakter UTF-8 supaya tidak
 * terbelah. Kembali lebar kolom hasil. */
int ui_truncate(const char *s, int max_cols, char *out, size_t size) {
    int total, limit, used = 0;
    size_t cut = 0, i, olen = 0;
    if (s == NULL || out == NULL || size == 0) {
        if (out != NULL && size > 0) out[0] = '\0';
        return 0;
    }
    if (max_cols <= 0) {
        out[0] = '\0';
        return 0;
    }
    total = ui_width(s);
    if (total <= max_cols) {
        while (s[olen] != '\0' && olen < size - 1) {
            out[olen] = s[olen];
            olen++;
        }
        out[olen] = '\0';
        return total;
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
    if (cut > size - 4) cut = size - 4;
    for (i = 0; i < cut; i++) out[i] = s[i];
    out[cut] = '.';
    out[cut + 1] = '.';
    out[cut + 2] = '.';
    out[cut + 3] = '\0';
    return used + 3;
}

/* Mencetak s terpotong: bila lebar muat dicetak utuh, jika tidak dicetak
 * sampai batas max_cols - 3 lalu "...". */
void ui_print_truncated(const char *s, int max_cols) {
    char buf[UI_WIDTH + 4];
    if (s == NULL || max_cols <= 0) return;
    if (max_cols > UI_WIDTH) {
        fputs(s, stdout);
        return;
    }
    (void)ui_truncate(s, max_cols, buf, sizeof buf);
    fputs(buf, stdout);
}
