#include "tt_grep.h"
#include <string.h>
#include <ctype.h>

/* Memeriksa apakah pattern muncul mulai dari posisi i pada line;
 * pembanding per karakter dengan tolower bila ignore_case. */
static int match_at(const char *line, size_t i, const char *pattern,
                    int ignore_case) {
    size_t j;
    for (j = 0; pattern[j] != '\0'; j++) {
        char lc = line[i + j];
        char pc = pattern[j];
        if (ignore_case) {
            lc = (char)tolower((unsigned char)lc);
            pc = (char)tolower((unsigned char)pc);
        }
        if (lc != pc) return 0;
    }
    return 1;
}

/* Menghitung seluruh kemunculan pattern pada satu baris. */
static int count_occurrences(const char *line, const char *pattern,
                             int ignore_case) {
    size_t len, plen, i;
    int total = 0;
    if (line == NULL || pattern == NULL) return 0;
    len = strlen(line);
    plen = strlen(pattern);
    if (plen == 0 || plen > len) return 0;
    for (i = 0; i + plen <= len; i++) {
        if (match_at(line, i, pattern, ignore_case)) total++;
    }
    return total;
}

/* Posisi (mulai 1) kemunculan pertama pattern pada baris; 0 jika tidak ada. */
static int first_occurrence(const char *line, const char *pattern,
                            int ignore_case) {
    size_t len, plen, i;
    if (line == NULL || pattern == NULL) return 0;
    len = strlen(line);
    plen = strlen(pattern);
    if (plen == 0 || plen > len) return 0;
    for (i = 0; i + plen <= len; i++) {
        if (match_at(line, i, pattern, ignore_case)) return (int)i + 1;
    }
    return 0;
}

/* Memecah teks menjadi baris-baris tanpa \r maupun \n.
 * Kembali ST_OK, ST_NO_MEMORY, atau ST_BAD_INPUT saat teks kosong. */
static Status split_lines(const char *data, size_t n, char ***out, int *count) {
    size_t nl = 0, i, start;
    char **lines;
    *out = NULL;
    *count = 0;
    if (data == NULL || n == 0) return ST_BAD_INPUT;
    for (i = 0; i < n; i++) {
        if (data[i] == '\n') nl++;
    }
    *count = (data[n - 1] != '\n') ? (int)nl + 1 : (int)nl;
    if (*count == 0) return ST_BAD_INPUT;
    lines = (char **)util_malloc((size_t)*count * sizeof(char *));
    if (lines == NULL) return ST_NO_MEMORY;
    start = 0;
    for (i = 0; i < (size_t)*count; i++) {
        size_t end = start, len, j;
        while (end < n && data[end] != '\n') end++;
        len = end - start;
        while (len > 0 && data[start + len - 1] == '\r') len--;
        lines[i] = (char *)util_malloc(len + 1);
        if (lines[i] == NULL) {
            for (j = 0; j < i; j++) util_free(lines[j]);
            util_free(lines);
            *out = NULL;
            *count = 0;
            return ST_NO_MEMORY;
        }
        memcpy(lines[i], data + start, len);
        lines[i][len] = '\0';
        start = end + 1;
    }
    *out = lines;
    return ST_OK;
}

/* Mengisi array TtMatch dan membebaskan baris yang tidak ikut. */
static void fill_matches(TtMatch *matches, char **lines, int count,
                         const char *pattern, int ignore_case, int invert,
                         int *filled) {
    int i;
    *filled = 0;
    for (i = 0; i < count; i++) {
        int col = first_occurrence(lines[i], pattern, ignore_case);
        int has = (col > 0);
        if (has != invert) {
            matches[*filled].line = i + 1;
            matches[*filled].col = invert ? 0 : col;
            matches[*filled].text = lines[i];
            (*filled)++;
        } else {
            util_free(lines[i]);
            lines[i] = NULL;
        }
    }
}

/* Pencarian baris sesuai kontrak grep mini di tt_grep.h. */
Status tt_grep(const TtText *t, const char *pattern, int ignore_case,
               int invert, TtMatch **matches, int *match_count, int *total_occurrences) {
    char **lines;
    int count, target, filled = 0, i;
    TtMatch *arr;
    if (t == NULL || t->data == NULL || pattern == NULL || matches == NULL ||
        match_count == NULL || total_occurrences == NULL) return ST_BAD_INPUT;
    if (pattern[0] == '\0') return ST_BAD_INPUT;
    *matches = NULL;
    *match_count = 0;
    *total_occurrences = 0;
    if (split_lines(t->data, t->length, &lines, &count) != ST_OK) return ST_BAD_INPUT;
    for (i = 0; i < count; i++) {
        *total_occurrences += count_occurrences(lines[i], pattern, ignore_case);
    }
    target = 0;
    for (i = 0; i < count; i++) {
        int has = first_occurrence(lines[i], pattern, ignore_case) > 0;
        if (has != invert) target++;
    }
    if (target == 0) {
        for (i = 0; i < count; i++) util_free(lines[i]);
        util_free(lines);
        return ST_OK;
    }
    arr = (TtMatch *)util_malloc((size_t)target * sizeof(TtMatch));
    if (arr == NULL) {
        for (i = 0; i < count; i++) util_free(lines[i]);
        util_free(lines);
        return ST_NO_MEMORY;
    }
    fill_matches(arr, lines, count, pattern, ignore_case, invert, &filled);
    util_free(lines);
    *matches = arr;
    *match_count = filled;
    return ST_OK;
}

/* Membebaskan teks tiap match lalu array-nya. */
void tt_free_matches(TtMatch *matches, int match_count) {
    int i;
    if (matches == NULL) return;
    for (i = 0; i < match_count; i++) util_free(matches[i].text);
    util_free(matches);
}
