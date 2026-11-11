#include "tt_analyzer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int is_word_char(char c) {
    return isalnum((unsigned char)c) != 0;
}

static void reset_stats(TtStats *s) {
    int i;
    s->chars = 0;
    s->words = 0;
    s->sentences = 0;
    s->lines = 0;
    for (i = 0; i < 26; i++) s->letters[i] = 0;
}

/* Satu pass: hitung karakter, huruf, kata, kalimat, dan baris sesuai
 * definisi baku di PRD Bagian 7. */
Status tt_compute_stats(const char *text, TtStats *out) {
    const char *p;
    int in_word = 0, word_seen_since_end = 0, prev_was_end = 0;
    if (text == NULL || out == NULL) return ST_BAD_INPUT;
    reset_stats(out);
    for (p = text; *p != '\0'; p++) {
        char c = *p;
        if (c == '\n' || c == '\r') {
            if (c == '\n') out->lines++;
            in_word = 0;
            continue;
        }
        out->chars++;
        if (is_word_char(c)) {
            if (!in_word) {
                out->words++;
                word_seen_since_end = 1;
            }
            in_word = 1;
            prev_was_end = 0;
        } else {
            in_word = 0;
            if (c == '.' || c == '!' || c == '?') {
                if (!prev_was_end) out->sentences++;
                prev_was_end = 1;
            } else {
                prev_was_end = 0;
            }
        }
        if (c >= 'a' && c <= 'z') out->letters[c - 'a']++;
        else if (c >= 'A' && c <= 'Z') out->letters[c - 'A']++;
    }
    if (out->chars > 0 && text[strlen(text) - 1] != '\n') out->lines++;
    if (word_seen_since_end && !prev_was_end) out->sentences++;
    return ST_OK;
}

/* Mencari word (huruf kecil) pada array dengan Sequential Search;
 * kembali indeks atau -1. */
static int freq_index_of(const TtWordFreq *a, int n, const char *word) {
    int i;
    for (i = 0; i < n; i++) {
        if (strcmp(a[i].word, word) == 0) return i;
    }
    return -1;
}

/* Menyalin kata mentah ke buffer huruf kecil, paling banyak 39 karakter;
 * sisa karakter dari kata yang lebih panjang dibuang. */
static void copy_word_lower(const char *src, size_t len, char *dst) {
    size_t i, keep = len < (size_t)(TT_MAX_WORD - 1) ? len : (size_t)(TT_MAX_WORD - 1);
    for (i = 0; i < keep; i++) dst[i] = (char)tolower((unsigned char)src[i]);
    dst[keep] = '\0';
}

/* Frekuensi kata unik: kapasitas mulai 16, digandakan lewat util_realloc,
 * pencarian kata yang sudah ada memakai Sequential Search. */
Status tt_word_frequency(const char *text, TtWordFreq **out, int *count) {
    const char *p;
    TtWordFreq *arr = NULL;
    int n = 0, cap = 16;
    if (text == NULL || out == NULL || count == NULL) return ST_BAD_INPUT;
    *out = NULL;
    *count = 0;
    arr = (TtWordFreq *)util_calloc((size_t)cap, sizeof(TtWordFreq));
    if (arr == NULL) return ST_NO_MEMORY;
    p = text;
    while (*p != '\0') {
        const char *start;
        size_t len;
        char buf[TT_MAX_WORD];
        int idx;
        while (*p != '\0' && !is_word_char(*p)) p++;
        if (*p == '\0') break;
        start = p;
        while (*p != '\0' && is_word_char(*p)) p++;
        len = (size_t)(p - start);
        copy_word_lower(start, len, buf);
        idx = freq_index_of(arr, n, buf);
        if (idx >= 0) {
            arr[idx].count++;
        } else {
            if (n == cap) {
                TtWordFreq *tmp;
                cap *= 2;
                tmp = (TtWordFreq *)util_realloc(arr, (size_t)cap * sizeof(TtWordFreq));
                if (tmp == NULL) {
                    util_free(arr);
                    return ST_NO_MEMORY;
                }
                arr = tmp;
            }
            copy_word_lower(start, len, arr[n].word);
            arr[n].count = 1;
            n++;
        }
    }
    *out = arr;
    *count = n;
    return ST_OK;
}

/* Selection Sort: terbanyak dulu; tie diurut alfabetis menaik. */
void tt_sort_freq_desc(TtWordFreq *a, int n) {
    int i, j;
    if (a == NULL) return;
    for (i = 0; i < n - 1; i++) {
        int best = i;
        for (j = i + 1; j < n; j++) {
            if (a[j].count > a[best].count ||
                (a[j].count == a[best].count &&
                 strcmp(a[j].word, a[best].word) < 0)) {
                best = j;
            }
        }
        if (best != i) {
            TtWordFreq t = a[i];
            a[i] = a[best];
            a[best] = t;
        }
    }
}

/* Bubble Sort alfabetis menaik. */
void tt_sort_words_alpha(TtWordFreq *a, int n) {
    int i, j;
    if (a == NULL) return;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (strcmp(a[j].word, a[j + 1].word) > 0) {
                TtWordFreq t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
        }
    }
}

/* Binary Search pada array terurut alfabetis; bandingkan huruf kecil. */
int tt_binary_search_word(const TtWordFreq *a, int n, const char *word) {
    char key[TT_MAX_WORD];
    int lo = 0, hi;
    if (a == NULL || word == NULL || word[0] == '\0') return -1;
    copy_word_lower(word, strlen(word), key);
    hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(a[mid].word, key);
        if (cmp == 0) return mid;
        if (cmp < 0) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

/* Histogram "A | ###### 6": huruf 0 disembunyikan; skala jika max > 50. */
void tt_print_letter_histogram(const TtStats *s) {
    int i, max = 0;
    if (s == NULL) return;
    for (i = 0; i < 26; i++) {
        if (s->letters[i] > max) max = s->letters[i];
    }
    for (i = 0; i < 26; i++) {
        int count = s->letters[i];
        int bars, k;
        if (count == 0) continue;
        if (max > 50) {
            bars = count * 50 / max;
            if (bars < 1) bars = 1;
        } else {
            bars = count;
        }
        printf("%c | ", 'A' + i);
        for (k = 0; k < bars; k++) putchar('#');
        printf(" %d\n", count);
    }
}
