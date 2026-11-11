#ifndef TT_GREP_H
#define TT_GREP_H

#include "../common/common.h"
#include "../tt_text/tt_text.h"

/* Hasil grep: nomor baris dan kolom kemunculan pertama (mulai 1),
 * plus salinan baris tanpa \r maupun \n. */
typedef struct {
    int line;
    int col;
    char *text;
} TtMatch;

/* Mencari pattern di tiap baris dengan pembanding buatan sendiri (tanpa
 * strstr). ignore_case: abaikan huruf besar/kecil. invert: matches berisi
 * baris yang tidak memuat pattern (col = 0). total_occurrences selalu
 * menghitung semua kemunculan, termasuk beberapa per baris. Pattern kosong
 * -> ST_BAD_INPUT; tidak ditemukan -> ST_OK dengan match_count 0. Array
 * dan salinan baris dilepas pemanggil lewat tt_free_matches. */
Status tt_grep(const TtText *t, const char *pattern, int ignore_case,
               int invert, TtMatch **matches, int *match_count, int *total_occurrences);

/* Melepas salinan baris tiap TtMatch lalu array matches-nya. */
void tt_free_matches(TtMatch *matches, int match_count);

#endif
