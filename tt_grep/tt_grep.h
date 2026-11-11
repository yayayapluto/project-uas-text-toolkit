#ifndef TT_GREP_H
#define TT_GREP_H

#include "common.h"
#include "tt_text.h"

typedef struct {
    char *line;
} TtMatch;

Status tt_grep(const TtText *t, const char *pattern, int ignore_case,
               int invert, TtMatch **matches, int *match_count, int *total_occurrences);

#endif