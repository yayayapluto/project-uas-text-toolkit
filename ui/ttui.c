#include "ttui.h"
#include "../common/common.h"
#include <stdio.h>
#include <string.h>

/* Menghitung lebar tampilan: setiap lead byte UTF-8 dihitung satu kolom
 * dan byte kelanjutannya dilompati. */
size_t ttui_width(const char *s) {
    size_t w = 0;
    if (s == NULL) return 0;
    for (; *s != '\0'; s++) {
        w++;
        while (((unsigned char)s[1] & 0xC0) == 0x80) s++;
    }
    return w;
}

/* Menulis urutan escape warna teks 24-bit. */
void ttui_rgb(int r, int g, int b) {
    printf("\033[38;2;%d;%d;%dm", r, g, b);
}

/* Mengembalikan gaya teks ke bawaan terminal. */
void ttui_reset(void) {
    printf("\033[0m");
}

/* Mencetak n karakter garis "─" dengan warna border aktif. */
static void ui_hline(int n) {
    int i;
    for (i = 0; i < n; i++) fputs("\xE2\x94\x80", stdout);
}

/* Menyusun ulang lebar menjadi string sementara untuk pemanggil. */
static char *ui_pad(size_t pad) {
    char *s = (char *)util_malloc(pad + 1);
    size_t i;
    if (s == NULL) return NULL;
    for (i = 0; i < pad; i++) s[i] = ' ';
    s[pad] = '\0';
    return s;
}

/* Panel dengan border sudut bulat; judul memakai warna (r,g,b), isi
 * memakai warna teks bawaan, border memakai warna border bawaan. */
void ttui_panel(const char *title, const char **lines, size_t count,
                int r, int g, int b) {
    size_t w = 0, i;
    char *pad;
    if (title != NULL && ttui_width(title) > w) w = ttui_width(title);
    for (i = 0; i < count; i++) {
        size_t lw = ttui_width(lines[i]);
        if (lw > w) w = lw;
    }
    ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
    fputs("\xE2\x95\xAD\xE2\x94\x80", stdout);
    ui_hline((int)w + 2);
    fputs("\xE2\x95\xAE\n", stdout);
    if (title != NULL) {
        pad = ui_pad(w - ttui_width(title));
        if (pad != NULL) {
            ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
            fputs("\xE2\x94\x82 ", stdout);
            ttui_rgb(r, g, b);
            fputs(title, stdout);
            ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
            printf(" %s", pad);
            fputs("\xE2\x94\x82\n", stdout);
            util_free(pad);
        }
    }
    for (i = 0; i < count; i++) {
        size_t lw = ttui_width(lines[i]);
        pad = ui_pad(w - lw);
        if (pad == NULL) continue;
        ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
        fputs("\xE2\x94\x82 ", stdout);
        ttui_rgb(TTUI_TEXT_R, TTUI_TEXT_G, TTUI_TEXT_B);
        printf("%s", lines[i]);
        printf(" %s", pad);
        ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
        fputs("\xE2\x94\x82\n", stdout);
        util_free(pad);
    }
    ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
    fputs("\xE2\x95\xB0\xE2\x94\x80", stdout);
    ui_hline((int)w + 2);
    fputs("\xE2\x95\xAF\n", stdout);
    ttui_reset();
}

/* Lebar kolom maksimum dari header dan seluruh sel kolom terkait. */
static size_t ui_col_width(const char **headers, const char ***rows,
                           size_t nrows, size_t col) {
    size_t w = ttui_width(headers[col]), i;
    for (i = 0; i < nrows; i++) {
        size_t cw = ttui_width(rows[i][col]);
        if (cw > w) w = cw;
    }
    return w;
}

/* Satu baris sel tabel: "│ isi │ isi │" dengan padding spasi. */
static void ui_row(const char **cells, const size_t *w, size_t ncols, int head) {
    size_t c;
    char *pad;
    ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
    fputs("\xE2\x94\x82", stdout);
    for (c = 0; c < ncols; c++) {
        pad = ui_pad(w[c] - ttui_width(cells[c]));
        if (pad == NULL) return;
        fputs(" ", stdout);
        if (head) ttui_rgb(TTUI_ACCENT_R, TTUI_ACCENT_G, TTUI_ACCENT_B);
        else ttui_rgb(TTUI_TEXT_R, TTUI_TEXT_G, TTUI_TEXT_B);
        fputs(cells[c], stdout);
        ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
        printf(" %s", pad);
        fputs("\xE2\x94\x82", stdout);
        util_free(pad);
    }
    fputs("\n", stdout);
}

/* Baris pemisah: ├─┼─┤ atau ╰─┴─╯ mengikuti jenis ujung yang diminta. */
static void ui_sep(const size_t *w, size_t ncols, const char *left,
                   const char *mid, const char *right) {
    size_t c;
    ttui_rgb(TTUI_BORDER_R, TTUI_BORDER_G, TTUI_BORDER_B);
    fputs(left, stdout);
    for (c = 0; c < ncols; c++) {
        ui_hline((int)w[c] + 2);
        if (c + 1 < ncols) fputs(mid, stdout);
    }
    fputs(right, stdout);
    fputs("\n", stdout);
}

/* Tabel dengan sudut bulat: header berwarna aksen, isi warna teks. */
void ttui_table(const char **headers, const char ***rows,
                size_t nrows, size_t ncols) {
    size_t *w, i;
    if (headers == NULL || rows == NULL || ncols == 0) return;
    w = (size_t *)util_malloc(ncols * sizeof(size_t));
    if (w == NULL) return;
    for (i = 0; i < ncols; i++) w[i] = ui_col_width(headers, rows, nrows, i);
    ui_sep(w, ncols, "\xE2\x95\xAD\xE2\x94\x80", "\xE2\x94\x80\xE2\x94\xAC\xE2\x94\x80", "\xE2\x94\x80\xE2\x94\xAE");
    ui_row(headers, w, ncols, 1);
    ui_sep(w, ncols, "\xE2\x94\x9C\xE2\x94\x80", "\xE2\x94\x80\xE2\x94\xBC\xE2\x94\x80", "\xE2\x94\x80\xE2\x94\xA4");
    for (i = 0; i < nrows; i++) {
        ui_row(rows[i], w, ncols, 0);
    }
    ui_sep(w, ncols, "\xE2\x95\xB0\xE2\x94\x80", "\xE2\x94\x80\xE2\x94\xB4\xE2\x94\x80", "\xE2\x94\x80\xE2\x95\xAF");
    ttui_reset();
    util_free(w);
}

/* Badge: titik dan label berwarna mengikuti jenis status. */
void ttui_badge(const char *label, int kind) {
    int r, g, b;
    switch (kind) {
        case TTUI_BADGE_OK:   r = 70;  g = 200; b = 120; break;
        case TTUI_BADGE_WARN: r = 240; g = 190; b = 60;  break;
        case TTUI_BADGE_FAIL: r = 240; g = 90;  b = 90;  break;
        default:              r = 100; g = 160; b = 240; break;
    }
    ttui_rgb(r, g, b);
    fputs("\xE2\x97\x8F ", stdout);
    fputs(label != NULL ? label : "", stdout);
    ttui_reset();
}
