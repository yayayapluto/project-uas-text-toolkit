#include "tt_text.h"
#include "../common/common.h"
#include <string.h>
#include <stdlib.h>

/* Memuat seluruh berkas ke TtText; data dialokasi util_read_file.
 * Pemanggil melepas lewat tt_free_text. */
Status tt_load_text(const char *path, TtText *out) {
    char *buf;
    size_t len;
    Status s = util_read_file(path, &buf, &len);
    if (s != ST_OK) return s;
    out->data = buf;
    out->n = len;
    return ST_OK;
}

/* Melepas buffer TtText dan mengosongkan struct. */
void tt_free_text(TtText *t) {
    util_free(t->data);
    t->data = NULL;
    t->n = 0;
}