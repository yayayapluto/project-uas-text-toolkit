#include "tt_report.h"
#include "tt_text.h"
#include "tt_analyzer.h"
#include "tt_cipher.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *build_report(const char *text, const TtStats *s, const TtWordFreq *words, int word_count) {
    if (text == NULL) return NULL;
    size_t text_len = strlen(text);
    size_t sz = 64 + text_len + 64 + (word_count > 0 ? word_count * 32 : 0);
    char *buf = (char *)util_malloc(sz);
    if (buf == NULL) return NULL;
    int n = snprintf(buf, sz, "LAPISAN TEKS\n");
    if (n < 0) { util_free(buf); return NULL; }
    size_t off = (size_t)n;
    size_t remain = sz - off - 1;
    size_t copylen = text_len < remain ? text_len : remain - 1;
    memcpy(buf + off, text, copylen);
    off += copylen;
    buf[off] = '\0';
    if (s != NULL) {
        char tmp[64];
        int wn = snprintf(tmp, sizeof tmp, "\nStats: lines=%d words=%d chars=%d", s->lines, s->words, s->chars);
        if (wn > 0) { size_t need = (size_t)wn + 1; if (off + need < sz) { memcpy(buf + off, tmp, wn); off += wn; } }
    }
    if (words != NULL && word_count > 0) {
        char tmp[128];
        int wn = snprintf(tmp, sizeof tmp, "\nKata (teratas):");
        if (wn > 0) { size_t need = (size_t)wn + 1; if (off + need < sz) { memcpy(buf + off, tmp, wn); off += wn; } }
        for (int i = 0; i < word_count; i++) {
            int wn = snprintf(tmp, sizeof tmp, "\n  %s: %d", words[i].word, words[i].count);
            if (wn > 0) {
                size_t need = (size_t)wn + 1;
                if (off + need < sz) { memcpy(buf + off, tmp, wn); off += wn; }
            }
        }
    }
    buf[off] = '\0';
    return buf;
}

Status tt_write_report(const char *path, const TtText *t, const TtStats *s,
                       const TtWordFreq *words, int word_count) {
    if (path == NULL || t == NULL) return ST_BAD_INPUT;
    char *buf = build_report(t->data, s, words, word_count);
    if (buf == NULL) return ST_NO_MEMORY;
    size_t len = strlen(buf);
    Status s2 = util_write_file(path, buf, len);
    util_free(buf);
    return s2;
}