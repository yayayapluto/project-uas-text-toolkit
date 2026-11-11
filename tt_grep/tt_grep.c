#include "tt_grep.h"
#include "tt_text.h"
#include "common.h"
#include <string.h>
#include <ctype.h>

void cleanup_lines(char **lines, int count);

static int line_contains_pattern(const char *line, const char *pattern, int ignore_case, int *out_occurrences) {
    if (line == NULL || pattern == NULL || *pattern == '\0') {
        if (out_occurrences) *out_occurrences = 0;
        return 0;
    }
    size_t lp = strlen(line);
    size_t mp = strlen(pattern);
    if (mp > lp) {
        if (out_occurrences) *out_occurrences = 0;
        return 0;
    }
    int found = 0;
    int total = 0;
    for (size_t i = 0; i <= lp - mp; i++) {
        int match = 1;
        for (size_t j = 0; j < mp; j++) {
            char lc = line[i+j];
            char pc = pattern[j];
            if (ignore_case) {
                lc = tolower((unsigned char)lc);
                pc = tolower((unsigned char)pc);
            }
            if (lc != pc) { match = 0; break; }
        }
        if (match) {
            found = 1;
            total++;
            /* skip past this match to avoid overlapping? prompt says "hanya kemunculan pertama per baris" so we break after first */
            break;
        }
    }
    if (out_occurrences) *out_occurrences = total;
    return found;
}

static int split_into_lines(const char *data, size_t n, char ***lines, int *line_count) {
    *line_count = 0;
    if (data == NULL || n == 0) return 0;
    /* count lines: number of '\n' + 1 if last char not \n, else just \n count */
    size_t nl = 0;
    for (size_t i = 0; i < n; i++) if (data[i] == '\n') nl++;
    *line_count = (n > 0 && data[n-1] != '\n') ? nl + 1 : nl;
    if (*line_count == 0) return 0;
    *lines = (char **)util_malloc((size_t)(*line_count) * sizeof(char *));
    if (*lines == NULL) return 0;
    size_t start = 0;
    for (int i = 0; i < *line_count; i++) {
        size_t end = start;
        while (end < n && data[end] != '\n') end++;
        size_t len = end - start;
        (*lines)[i] = (char *)util_malloc(len + 1);
        memcpy((*lines)[i], data + start, len);
        (*lines)[i][len] = '\0';
        /* strip \r */
        size_t l = len;
        while (l > 0 && (*lines)[i][l-1] == '\r') (*lines)[i][--l] = '\0';
        (*lines)[i][l] = '\0';
        start = end + 1;
    }
    return 1;
}

Status tt_grep(const TtText *t, const char *pattern, int ignore_case,
               int invert, TtMatch **matches, int *match_count, int *total_occurrences) {
    if (t == NULL || t->data == NULL || pattern == NULL || matches == NULL || match_count == NULL || total_occurrences == NULL)
        return ST_BAD_INPUT;

    char **lines = NULL;
    int line_count = 0;
    if (!split_into_lines(t->data, t->n, &lines, &line_count)) return ST_BAD_INPUT;

    *total_occurrences = 0;
    *match_count = 0;

    /* first pass: compute total occurrences */
    for (int i = 0; i < line_count; i++) {
        int occ = 0;
        line_contains_pattern(lines[i], pattern, ignore_case, &occ);
        *total_occurrences += occ;
    }

    /* allocate matches array */
    *matches = (TtMatch *)util_malloc((size_t)line_count * sizeof(TtMatch));
    if (*matches == NULL) { *match_count = 0; cleanup_lines(lines, line_count); return ST_NO_MEMORY; }
    *match_count = 0;

    /* second pass: fill matches */
    for (int i = 0; i < line_count; i++) {
        int contains = line_contains_pattern(lines[i], pattern, ignore_case, NULL);
        if (invert) {
            if (!contains) {
                (*matches)[*match_count].line = lines[i];
                (*match_count)++;
            } else {
                /* free line we won't use? we still own lines, but we'll return them; for simplicity just null it and free later */
                (*matches)[*match_count].line = NULL;
            }
        } else {
            if (contains) {
                (*matches)[*match_count].line = lines[i];
                (*match_count)++;
            }
        }
    }

    cleanup_lines(lines, line_count);
    return ST_OK;
}

void cleanup_lines(char **lines, int count) {
    if (lines == NULL) return;
    for (int i = 0; i < count; i++) util_free(lines[i]);
    util_free(lines);
}