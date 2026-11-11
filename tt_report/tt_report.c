#include "tt_report.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Builder lokal: buffer string yang tumbuh lewat util_realloc. */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
    Status err;
} RepBuf;

/* Menambahkan teks NUL-terminated ke buffer; menggandakan kapasitas
 * bila perlu. Kegagalan alokasi menandai err ST_NO_MEMORY. */
static void rb_add(RepBuf *b, const char *s) {
    size_t add = strlen(s);
    while (b->err == ST_OK && b->len + add + 1 > b->cap) {
        char *tmp;
        b->cap *= 2;
        tmp = (char *)util_realloc(b->data, b->cap);
        if (tmp == NULL) {
            b->err = ST_NO_MEMORY;
            return;
        }
        b->data = tmp;
    }
    if (b->err == ST_OK) {
        memcpy(b->data + b->len, s, add + 1);
        b->len += add;
    }
}

/* Menambahkan teks berformat printf ke buffer. */
static void rb_addf(RepBuf *b, const char *fmt, ...) {
    char line[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);
    rb_add(b, line);
}

/* Menambahkan frekuensi huruf non-nol ke builder. */
static void build_letters(RepBuf *b, const TtStats *s) {
    int i;
    for (i = 0; i < 26; i++) {
        if (s->letters[i] > 0) {
            rb_addf(b, "%c %d\r\n", 'A' + i, s->letters[i]);
        }
    }
}

/* Menambahkan daftar kata terurut (terbanyak dulu, bernomor). */
static void build_words(RepBuf *b, const TtWordFreq *freq, int count) {
    int i;
    if (count == 0) {
        rb_add(b, "(tidak ada kata)\r\n");
        return;
    }
    for (i = 0; i < count; i++) {
        rb_addf(b, "%d. %s %d\r\n", i + 1, freq[i].word, freq[i].count);
    }
}

/* Membangun isi laporan menurut format PRD Bagian 8.3; isi di buffer
 * dibebaskan pemanggil dengan util_free. */
static Status build_report(const TtStats *s, const TtWordFreq *freq, int freq_count,
                           const char *source_path, RepBuf *b) {
    b->data = (char *)util_malloc(512);
    if (b->data == NULL) return ST_NO_MEMORY;
    b->data[0] = '\0';
    b->len = 0;
    b->cap = 512;
    b->err = ST_OK;
    rb_addf(b, "LAPORAN ANALISIS TEKS\r\n");
    rb_addf(b, "Sumber   : %s\r\n", source_path != NULL ? source_path : "(teks tanpa file)");
    rb_add(b, "\r\n");
    rb_add(b, "[Statistik]\r\n");
    rb_addf(b, "Karakter : %d\r\n", s->chars);
    rb_addf(b, "Kata     : %d\r\n", s->words);
    rb_addf(b, "Kalimat  : %d\r\n", s->sentences);
    rb_addf(b, "Baris    : %d\r\n", s->lines);
    rb_add(b, "\r\n");
    rb_add(b, "[Frekuensi huruf]\r\n");
    build_letters(b, s);
    rb_add(b, "\r\n");
    rb_add(b, "[Frekuensi kata]\r\n");
    build_words(b, freq, freq_count);
    return b->err;
}

/* Menulis laporan ke layar (out_path NULL) atau ke file; isi keduanya
 * identik karena dari builder yang sama. */
Status tt_write_report(const TtStats *s, const TtWordFreq *freq, int freq_count,
                       const char *source_path, const char *out_path) {
    RepBuf b;
    Status st;
    if (s == NULL) return ST_BAD_INPUT;
    b.data = NULL;
    b.len = 0;
    b.cap = 0;
    b.err = ST_OK;
    st = build_report(s, freq, freq_count, source_path, &b);
    if (st == ST_OK) {
        if (out_path == NULL) {
            fputs(b.data, stdout);
        } else {
            st = util_write_file(out_path, b.data, b.len);
        }
    }
    util_free(b.data);
    return st;
}
