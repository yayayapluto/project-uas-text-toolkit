#ifndef TT_TEXT_H
#define TT_TEXT_H

#include "../common/common.h"

typedef struct {
    char *data;
    size_t n;
} TtText;

Status tt_load_text(const char *path, TtText *out);
void tt_free_text(TtText *t);

#endif