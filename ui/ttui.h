#ifndef TTUI_H
#define TTUI_H

#include <stddef.h>

/* Output CLI bergaya modern ala Python "rich" untuk stdout biasa:
 * warna ANSI true color (24-bit) dan border Unicode box drawing.
 * Tidak memakai ncurses; aman dipanggil berulang tanpa state. */

/* Palet aksen bawaan; boleh diabaikan karena tiap fungsi menerima warna. */
#define TTUI_BORDER_R 90
#define TTUI_BORDER_G 100
#define TTUI_BORDER_B 125
#define TTUI_ACCENT_R 120
#define TTUI_ACCENT_G 180
#define TTUI_ACCENT_B 255
#define TTUI_TEXT_R   225
#define TTUI_TEXT_G   228
#define TTUI_TEXT_B   235

/* Jenis badge status untuk ttui_badge. */
enum {
    TTUI_BADGE_OK = 0,
    TTUI_BADGE_WARN,
    TTUI_BADGE_FAIL,
    TTUI_BADGE_INFO
};

/* Menulis kode warna teks true color ke stdout. */
void ttui_rgb(int r, int g, int b);

/* Menyalakan dan mematikan gaya ANSI (reset "\033[0m"). */
void ttui_reset(void);

/* Lebar tampilan string: byte ASCII dihitung 1, karakter UTF-8 multi-byte
 * dihitung satu per puncak lead byte-nya. */
size_t ttui_width(const char *s);

/* Panel dengan sudut bulat dan judul berwarna di baris pertama isi.
 * lines boleh NULL/0 untuk panel kosong; warna (r,g,b) untuk judul. */
void ttui_panel(const char *title, const char **lines, size_t count,
                int r, int g, int b);

/* Tabel sederhana: border box drawing, header berwarna aksen, isi rata
 * kiri. rows[i] adalah array ncols string. Lebar kolom otomatis dari
 * header dan sel terpanjang. */
void ttui_table(const char **headers, const char ***rows,
                size_t nrows, size_t ncols);

/* Badge status satu kata: titik berwarna mengikuti kind, label mengikuti
 * warna yang sama, tanpa newline. */
void ttui_badge(const char *label, int kind);

#endif
