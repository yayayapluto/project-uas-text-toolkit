#ifndef TT_REPORT_H
#define TT_REPORT_H

#include "../common/common.h"
#include "../tt_analyzer/tt_analyzer.h"

/* Menulis laporan analisis: ke layar bila out_path NULL, atau ke berkas.
 * Satu builder membuat isi laporan sehingga layar dan berkas identik
 * (TT-60). Kembali ST_OK, ST_BAD_INPUT, ST_NO_MEMORY, atau ST_FILE_WRITE
 * saat berkas tidak dapat ditulis (misal folder tidak ada). */
Status tt_write_report(const TtStats *s, const TtWordFreq *freq, int freq_count,
                       const char *source_path, const char *out_path);

#endif
