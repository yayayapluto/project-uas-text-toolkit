#ifndef TT_ANALYZER_H
#define TT_ANALYZER_H

#include "../common/common.h"

#define TT_MAX_WORD 40

/* Statistik teks; letters[i] = frekuensi huruf ke-i (A..Z, tanpa
 * membedakan kapital). */
typedef struct {
    int chars;
    int words;
    int sentences;
    int lines;
    int letters[26];
} TtStats;

/* Frekuensi satu kata unik; word selalu huruf kecil dan paling banyak
 * 39 karakter + '\0'. */
typedef struct {
    char word[TT_MAX_WORD];
    int count;
} TtWordFreq;

/* Menghitung karakter (tanpa \n dan \r), kata (deretan huruf/angka,
 * apostrof pemisah), kalimat (1+ tanda .!? berurutan = satu; teks berakhir
 * tanpa tanda akhir tapi ada kata = +1), baris (jumlah \n + satu jika
 * baris terakhir tidak kosong; \r\n = \n), serta frekuensi huruf.
 * Pemanggil tidak memegang memori dari fungsi ini. */
Status tt_compute_stats(const char *text, TtStats *out);

/* Menghitung frekuensi kata unik (huruf kecil, kata > 39 karakter
 * dipotong 39 karakter pertama). Array mulai kapasitas 16 dan digandakan
 * lewat util_realloc; pengecekan kata memakai Sequential Search. Array
 * dibebaskan pemanggil dengan util_free. */
Status tt_word_frequency(const char *text, TtWordFreq **out, int *count);

/* Selection Sort frekuensi dari terbanyak; tie diurut alfabetis. */
void tt_sort_freq_desc(TtWordFreq *a, int n);

/* Bubble Sort daftar kata alfabetis menaik. */
void tt_sort_words_alpha(TtWordFreq *a, int n);

/* Binary Search word pada array yang sudah terurut alfabetis; kembali
 * indeks atau -1; pencarian tidak membedakan huruf besar/kecil. */
int tt_binary_search_word(const TtWordFreq *a, int n, const char *word);

/* Mencetak histogram frekuensi huruf ke stdout: "A | ###### 6", huruf
 * berfrekuensi 0 disembunyikan; jika frekuensi terbesar > 50, panjang bar
 * diskalakan (terbesar 50, lainnya proporsional, minimal 1), angka asli
 * tetap dicetak. */
void tt_print_letter_histogram(const TtStats *s);

#endif
