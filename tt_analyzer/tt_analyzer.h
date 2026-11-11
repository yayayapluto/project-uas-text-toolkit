#ifndef TT_ANALYZER_H
#define TT_ANALYZER_H

#include "common.h"
#include "tt_text.h"

#define TT_MAX_WORD 40

typedef struct {
    int lines;
    int words;
    int chars;
} TtStats;

typedef struct {
    char *word;
    int count;
} TtWordFreq;

Status tt_compute_stats(const char *text, TtStats *out);
Status tt_word_frequency(const char *text, TtWordFreq **out, int *count);
void tt_sort_freq_desc(TtWordFreq *a, int n);
void tt_sort_words_alpha(TtWordFreq *a, int n);
int tt_binary_search_word(const TtWordFreq *a, int n, const char *word);
void tt_print_letter_histogram(const TtStats *s);

#endif