#ifndef TT_TEXT_H
#define TT_TEXT_H

#include "../common/common.h"

/* Teks termuat: isi file, panjang byte, dan path sumber. */
typedef struct {
    char *data;
    size_t length;
    char path[UTIL_MAX_PATH];
} TtText;

/* Memuat seluruh file teks ke memori dinamis; data, panjang, dan path
 * tersimpan di out. Data dialokasi util_read_file; pemanggil membebaskannya
 * lewat tt_free_text. Kembali ST_OK, ST_FILE_NOT_FOUND, ST_FILE_EMPTY,
 * ST_BAD_INPUT, atau ST_NO_MEMORY. */
Status tt_load_text(const char *path, TtText *out);

/* Membebaskan data TtText, mengosongkan pointer dan path. Aman dipanggil
 * ganda. */
void tt_free_text(TtText *t);

#endif
