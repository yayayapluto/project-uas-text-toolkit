#include "text_toolkit.h"
#include "common/common.h"
#include "tt_text/tt_text.h"
#include "tt_grep/tt_grep.h"
#include "tt_analyzer/tt_analyzer.h"
#include "tt_cipher/tt_cipher.h"
#include "tt_report/tt_report.h"
#include <stdio.h>
#include <string.h>

/* State menu: teks termuat, hasil analisis, dan frekuensi kata. */
typedef struct {
    TtText text;
    int has_text;
    TtStats stats;
    int has_stats;
    TtWordFreq *freq;
    int freq_count;
    int has_freq;
} MenuState;

/* Membebaskan frekuensi lama dan menandai hasil analisis basi. */
static void analysis_invalidate(MenuState *st) {
    util_free(st->freq);
    st->freq = NULL;
    st->freq_count = 0;
    st->has_freq = 0;
    st->has_stats = 0;
}

/* Membuang tanda kutip pembuka dan penutup dari path tempel, karena
 * "Copy as path" Windows menyertakan kutip di kedua ujung. */
static void strip_quotes(char *s) {
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
        s[len - 1] = '\0';
        memmove(s, s + 1, len - 1);
    }
}

/* Menu 1: muat file teks; teks lama dibebaskan dulu. */
static void menu_load_text(MenuState *st) {
    char path[UTIL_MAX_PATH];
    Status s;
    if (util_read_line("Path file: ", path, sizeof(path)) != ST_OK) return;
    util_trim(path);
    strip_quotes(path);
    if (path[0] == '\0') {
        printf("%s\n", util_status_message(ST_BAD_INPUT));
        return;
    }
    tt_free_text(&st->text);
    s = tt_load_text(path, &st->text);
    if (s == ST_OK) {
        st->has_text = 1;
        analysis_invalidate(st);
        printf("File dimuat (%d karakter).\n", (int)st->text.length);
    } else {
        st->has_text = 0;
        printf("%s\n", util_status_message(s));
    }
}

/* Menampilkan tiap match: baris, kolom, dan isi baris. */
static void print_matches(const TtMatch *matches, int count, int invert) {
    int i;
    for (i = 0; i < count; i++) {
        if (invert) {
            printf("Baris %d: %s\n", matches[i].line, matches[i].text);
        } else {
            printf("Baris %d, kolom %d: %s\n", matches[i].line,
                   matches[i].col, matches[i].text);
        }
    }
}

/* Menu 2: mini grep dengan opsi abaikan huruf dan invert. */
static void menu_grep(const MenuState *st) {
    char pattern[UTIL_MAX_LINE];
    TtMatch *matches = NULL;
    int count = 0, total = 0;
    int ignore_case, invert;
    Status s;
    if (!st->has_text) {
        printf("Muat file lewat menu 1 lebih dahulu.\n");
        return;
    }
    if (util_read_line("Pola yang dicari: ", pattern, sizeof(pattern)) != ST_OK) return;
    util_trim(pattern);
    if (util_read_int("Abaikan huruf besar/kecil? (1=ya, 0=tidak): ", 0, 1, &ignore_case) != ST_OK) return;
    if (util_read_int("Tampilkan baris yang tidak cocok? (1=ya, 0=tidak): ", 0, 1, &invert) != ST_OK) return;
    s = tt_grep(&st->text, pattern, ignore_case, invert, &matches, &count, &total);
    if (s == ST_OK) {
        print_matches(matches, count, invert);
        printf("Baris cocok: %d, total kemunculan: %d\n", count, total);
    } else {
        printf("%s\n", util_status_message(s));
    }
    tt_free_matches(matches, count);
}

/* Menjalankan analisis penuh: statistik dan frekuensi kata terurut. */
static Status analysis_run(MenuState *st) {
    Status s;
    if (!st->has_text) return ST_BAD_INPUT;
    analysis_invalidate(st);
    s = tt_compute_stats(st->text.data, &st->stats);
    if (s != ST_OK) return s;
    s = tt_word_frequency(st->text.data, &st->freq, &st->freq_count);
    if (s != ST_OK) return s;
    tt_sort_freq_desc(st->freq, st->freq_count);
    st->has_stats = 1;
    st->has_freq = 1;
    return ST_OK;
}

/* Sub-menu 3 opsi 4: binary search kata pada salinan alfabetis. */
static void analysis_search_word(const MenuState *st) {
    TtWordFreq *sorted;
    char word[UTIL_MAX_LINE];
    int idx;
    if (util_read_line("Kata yang dicari: ", word, sizeof(word)) != ST_OK) return;
    util_trim(word);
    util_to_lower(word);
    sorted = (TtWordFreq *)util_calloc((size_t)st->freq_count, sizeof(TtWordFreq));
    if (sorted == NULL) {
        printf("%s\n", util_status_message(ST_NO_MEMORY));
        return;
    }
    memcpy(sorted, st->freq, (size_t)st->freq_count * sizeof(TtWordFreq));
    tt_sort_words_alpha(sorted, st->freq_count);
    idx = tt_binary_search_word(sorted, st->freq_count, word);
    if (idx >= 0) {
        printf("Ditemukan: %s (%d kali).\n", sorted[idx].word, sorted[idx].count);
    } else {
        printf("Kata tidak ditemukan.\n");
    }
    util_free(sorted);
}

/* Sub-menu 3: statistik, histogram, frekuensi kata, binary search. */
static void menu_analyze(MenuState *st) {
    int pilihan;
    if (analysis_run(st) != ST_OK) {
        printf("Muat file lewat menu 1 lebih dahulu.\n");
        return;
    }
    for (;;) {
        printf("--- Analisis teks ---\n");
        printf("1. Statistik\n2. Histogram huruf\n3. Frekuensi kata\n4. Cari kata (binary search)\n0. Kembali\n");
        if (util_read_int("Pilihan: ", 0, 4, &pilihan) != ST_OK) return;
        if (pilihan == 0) return;
        if (pilihan == 1) {
            printf("Karakter : %d\nKata     : %d\nKalimat  : %d\nBaris    : %d\n",
                   st->stats.chars, st->stats.words, st->stats.sentences, st->stats.lines);
        } else if (pilihan == 2) {
            tt_print_letter_histogram(&st->stats);
        } else if (pilihan == 3) {
            int i;
            for (i = 0; i < st->freq_count; i++) {
                printf("%-12s %d\n", st->freq[i].word, st->freq[i].count);
            }
        } else if (pilihan == 4) {
            analysis_search_word(st);
        }
    }
}

/* Menu 4 opsi 4: proses file ke file sesuai mode tt_cipher_file. */
static void cipher_file_to_file(void) {
    char in_path[UTIL_MAX_PATH], out_path[UTIL_MAX_PATH], key[UTIL_MAX_LINE];
    int mode, shift;
    Status s;
    printf("Mode: 0=Caesar enkripsi, 1=Caesar dekripsi, 2=Vigenere enkripsi, 3=Vigenere dekripsi\n");
    if (util_read_int("Mode: ", 0, 3, &mode) != ST_OK) return;
    if (util_read_line("File sumber: ", in_path, sizeof(in_path)) != ST_OK) return;
    util_trim(in_path);
    strip_quotes(in_path);
    if (util_read_line("File hasil: ", out_path, sizeof(out_path)) != ST_OK) return;
    util_trim(out_path);
    strip_quotes(out_path);
    shift = 0;
    if (mode == 0 || mode == 1) {
        if (util_read_int("Geser: ", -1000000, 1000000, &shift) != ST_OK) return;
    } else {
        if (util_read_line("Kunci (huruf saja): ", key, sizeof(key)) != ST_OK) return;
        util_trim(key);
    }
    s = tt_cipher_file(in_path, out_path, mode, shift, key);
    if (s == ST_OK) printf("Hasil ditulis ke %s\n", out_path);
    else printf("%s\n", util_status_message(s));
}

/* Menu 4: sandi teks yang diketik dan proses file ke file. */
static void menu_cipher(void) {
    int pilihan;
    for (;;) {
        printf("--- Cipher ---\n");
        printf("1. Caesar (teks diketik)\n2. Vigenere (teks diketik)\n3. Brute force Caesar\n4. file ke file\n0. Kembali\n");
        if (util_read_int("Pilihan: ", 0, 4, &pilihan) != ST_OK) return;
        if (pilihan == 0) return;
        if (pilihan == 1 || pilihan == 2) {
            char text[UTIL_MAX_LINE], key[UTIL_MAX_LINE];
            int shift, decrypt;
            if (util_read_line("Teks: ", text, sizeof(text)) != ST_OK) return;
            if (pilihan == 1) {
                if (util_read_int("Geser (negatif untuk dekripsi): ", -1000000, 1000000, &shift) != ST_OK) return;
                tt_caesar(text, shift);
            } else {
                if (util_read_int("1=enkripsi, 0=dekripsi: ", 0, 1, &decrypt) != ST_OK) return;
                if (util_read_line("Kunci (huruf saja): ", key, sizeof(key)) != ST_OK) return;
                util_trim(key);
                if (tt_vigenere(text, key, decrypt ? 0 : 1) != ST_OK) {
                    printf("%s\n", util_status_message(ST_BAD_INPUT));
                }
            }
            printf("Hasil: %s\n", text);
        } else if (pilihan == 3) {
            char text[UTIL_MAX_LINE];
            if (util_read_line("Teks terenkripsi: ", text, sizeof(text)) != ST_OK) return;
            tt_caesar_brute(text);
        } else if (pilihan == 4) {
            cipher_file_to_file();
        }
    }
}

/* Menu 5: cek palindrom dan hasil pembalik string dari teks diketik. */
static void menu_palindrome(void) {
    char text[UTIL_MAX_LINE], rev[UTIL_MAX_LINE];
    if (util_read_line("Teks: ", text, sizeof(text)) != ST_OK) return;
    tt_reverse_rec(text, rev);
    printf("Palindrom : %s\n", tt_palindrome_rec(text) ? "ya" : "bukan");
    printf("Reverse   : %s\n", rev);
}

/* Menu 6: tulis laporan; analisis dijalankan dulu bila belum ada. */
static void menu_report(MenuState *st) {
    char out_path[UTIL_MAX_PATH];
    Status s;
    if (!st->has_text) {
        printf("Muat file lewat menu 1 lebih dahulu.\n");
        return;
    }
    if (!st->has_stats || !st->has_freq) {
        s = analysis_run(st);
        if (s != ST_OK) {
            printf("%s\n", util_status_message(s));
            return;
        }
    }
    if (util_read_line("Nama file laporan (kosong = ke layar): ", out_path, sizeof(out_path)) != ST_OK) return;
    util_trim(out_path);
    strip_quotes(out_path);
    if (out_path[0] == '\0') {
        s = tt_write_report(&st->stats, st->freq, st->freq_count, st->text.path, NULL);
    } else {
        s = tt_write_report(&st->stats, st->freq, st->freq_count, st->text.path, out_path);
        if (s == ST_OK) printf("Laporan ditulis ke %s\n", out_path);
    }
    if (s != ST_OK && out_path[0] != '\0') {
        printf("%s\n", util_status_message(s));
    }
}

/* Membebaskan seluruh memori yang masih dipegang state. */
static void state_free(MenuState *st) {
    tt_free_text(&st->text);
    st->has_text = 0;
    analysis_invalidate(st);
}

/* Menu utama Text Toolkit; keluar (0 atau EOF\) membebaskan semua memori. */
void run_text_toolkit(void) {
    MenuState st;
    int pilihan;
    memset(&st, 0, sizeof(st));
    for (;;) {
        printf("=== TEXT TOOLKIT ===\n");
        printf("1. Muat file teks\n");
        printf("2. Mini grep\n");
        printf("3. Analisis teks (statistik, frekuensi huruf, frekuensi kata)\n");
        printf("4. Cipher (Caesar / Vigenere / brute force)\n");
        printf("5. Palindrom dan reverse\n");
        printf("6. Tulis laporan\n");
        printf("0. Kembali\n");
        if (util_read_int("Pilihan: ", 0, 6, &pilihan) != ST_OK) break;
        if (pilihan == 0) break;
        if (pilihan == 1) menu_load_text(&st);
        else if (pilihan == 2) menu_grep(&st);
        else if (pilihan == 3) menu_analyze(&st);
        else if (pilihan == 4) menu_cipher();
        else if (pilihan == 5) menu_palindrome();
        else if (pilihan == 6) menu_report(&st);
    }
    state_free(&st);
}
