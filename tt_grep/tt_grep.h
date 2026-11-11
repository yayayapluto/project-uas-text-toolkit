#ifndef TT_GREP_H
#define TT_GREP_H

#include "../common/common.h"
#include "../tt_text/tt_text.h"

typedef struct {
    char *line;
} TtMatch;

/* Mencari pattern di tiap baris; pembanding manual tanpa strstr.
 * matches: baris pertama-tama cocok (atau tidak cocok saat invert);
 * total_occurrences: semua kemunculan termasuk beberapa per baris.
 * Pemanggil melepas matches lewat tt_free_matches. */
Status tt_grep(const TtText *t, const char *pattern, int ignore_case,
               int invert, TtMatch **matches, int *match_count, int *total_occurrences);

/* Melepas array TtMatch dan teks tiap baris di dalamnya. */
void tt_free_matches(TtMatch *matches, int match_count);

#endif