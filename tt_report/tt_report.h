#ifndef TT_REPORT_H
#define TT_REPORT_H

#include "common.h"
#include "tt_text.h"
#include "tt_analyzer.h"
#include "tt_cipher.h"

Status tt_write_report(const char *path, const TtText *t, const TtStats *s,
                       const TtWordFreq *words, int word_count);

#endif