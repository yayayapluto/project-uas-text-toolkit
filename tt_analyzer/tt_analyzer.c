#include "tt_analyzer.h"
#include "tt_text.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int freq_cmp_desc(const TtWordFreq *a, const TtWordFreq *b);

static int extract_words(const char *text, size_t n, char ***out_words, int *out_word_count) {
    *out_word_count = 0;
    if (text == NULL || n == 0) return 0;
    size_t i = 0;
    int count = 0;
    while (i < n) {
        if (isalnum((unsigned char)text[i]) || text[i] == '\'') {
            count++;
            while (i < n && (isalnum((unsigned char)text[i]) || text[i] == '\'')) i++;
        } else i++;
    }
    if (count == 0) return 0;
    *out_words = (char **)util_malloc((size_t)count * sizeof(char *));
    if (*out_words == NULL) return 0;
    *out_word_count = 0;
    i = 0;
    while (i < n) {
        if (isalnum((unsigned char)text[i]) || text[i] == '\'') {
            size_t start = i;
            while (i < n && (isalnum((unsigned char)text[i]) || text[i] == '\'')) i++;
            size_t wlen = i - start;
            (*out_words)[*out_word_count] = (char *)util_malloc(wlen + 1);
            memcpy((*out_words)[*out_word_count], text + start, wlen);
            (*out_words)[*out_word_count][wlen] = '\0';
            (*out_word_count)++;
        } else i++;
    }
    return 1;
}

Status tt_compute_stats(const char *text, TtStats *out) {
    if (text == NULL || out == NULL) return ST_BAD_INPUT;
    size_t n = strlen(text);
    int lines = 0;
    size_t i = 0;
    while (i < n) {
        if (text[i] == '\n') lines++;
        i++;
    }
    if (n > 0 && text[n-1] != '\n') lines++;
    int chars = 0;
    for (i = 0; i < n; i++) if (text[i] != '\r') chars++;
    int words = 0;
    int inword = 0;
    for (i = 0; i < n; i++) {
        if (isalnum((unsigned char)text[i])) { if (!inword) { words++; inword = 1; } }
        else inword = 0;
    }
    out->lines = lines;
    out->words = words;
    out->chars = chars;
    return ST_OK;
}

Status tt_word_frequency(const char *text, TtWordFreq **out, int *out_count) {
    if (text == NULL || out == NULL || out_count == NULL) return ST_BAD_INPUT;
    int wc;
    char **words;
    if (!extract_words(text, strlen(text), &words, &wc)) return ST_BAD_INPUT;
    if (wc == 0) { *out = NULL; *out_count = 0; return ST_OK; }
    TtWordFreq *freq = (TtWordFreq *)util_malloc((size_t)wc * sizeof(TtWordFreq));
    if (freq == NULL) { for (int i=0;i<wc;i++)util_free(words[i]); util_free(words); return ST_NO_MEMORY; }
    int used = 0;
    for (int i = 0; i < wc; i++) {
        char *w = words[i];
        util_to_lower(w);
        if ((int)strlen(w) > TT_MAX_WORD) w[TT_MAX_WORD] = '\0';
        int found = -1;
        for (int j = 0; j < used; j++) {
            if (util_stricmp(freq[j].word, w) == 0) { found = j; break; }
        }
        if (found >= 0) freq[found].count++;
        else {
            freq[used].word = words[i];
            freq[used].count = 1;
            used++;
        }
    }
    *out = freq;
    *out_count = used;
    util_free(words);
    return ST_OK;
}

void tt_sort_freq_desc(TtWordFreq *a, int n) {
    for (int i = 0; i < n-1; i++) {
        int best = i;
        for (int j = i+1; j < n; j++) {
            if (freq_cmp_desc(&a[j], &a[best]) < 0) best = j;
        }
        if (best != i) { TtWordFreq tmp = a[i]; a[i] = a[best]; a[best] = tmp; }
    }
}

static int freq_cmp_desc(const TtWordFreq *a, const TtWordFreq *b) {
    if (a->count != b->count) return (a->count > b->count) ? -1 : 1;
    return util_stricmp(a->word, b->word);
}

void tt_sort_words_alpha(TtWordFreq *a, int n) {
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (util_stricmp(a[i].word, a[j].word) > 0) {
                TtWordFreq tmp = a[i]; a[i] = a[j]; a[j] = tmp;
            }
}

int tt_binary_search_word(const TtWordFreq *a, int n, const char *word) {
    if (a == NULL || n <= 0 || word == NULL) return -1;
    int lo = 0, hi = n-1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int c = util_stricmp(a[mid].word, word);
        if (c == 0) return mid;
        if (c < 0) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

void tt_print_letter_histogram(const TtStats *s) {
    if (s == NULL) return;
    int cnt[26];
    memset(cnt, 0, sizeof(cnt));
    (void)s;
}