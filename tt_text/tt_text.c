#include "tt_text.h"
#include <string.h>

/* Memuat file dan mengisi data, length, serta path sesuai TT-01..TT-03. */
Status tt_load_text(const char *path, TtText *out) {
    Status s;
    if (path == NULL || out == NULL) return ST_BAD_INPUT;
    s = util_read_file(path, &out->data, &out->length);
    if (s != ST_OK) {
        out->data = NULL;
        out->length = 0;
        out->path[0] = '\0';
        return s;
    }
    if (strlen(path) >= UTIL_MAX_PATH) {
        util_free(out->data);
        out->data = NULL;
        out->length = 0;
        out->path[0] = '\0';
        return ST_BAD_INPUT;
    }
    strcpy(out->path, path);
    return ST_OK;
}

/* Membebaskan data dan mengosongkan struct; pemanggilan ganda aman. */
void tt_free_text(TtText *t) {
    if (t == NULL) return;
    util_free(t->data);
    t->data = NULL;
    t->length = 0;
    t->path[0] = '\0';
}
