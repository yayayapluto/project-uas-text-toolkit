#include "text_toolkit.h"
#include "common/common.h"
#include "common/ui.h"
#include "tt_text/tt_text.h"
#include "tt_grep/tt_grep.h"
#include "tt_analyzer/tt_analyzer.h"
#include "tt_cipher/tt_cipher.h"
#include "tt_report/tt_report.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
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

/* Melepas frekuensi lama dan menandai hasil analisis basi. */
static void analysis_invalidate(MenuState *st) {
    util_free(st->freq);
    st->freq = NULL;
    st->freq_count = 0;
    st->has_freq = 0;
    st->has_stats = 0;
}

/* Nama file dari path tanpa direktori (pemisah / atau \\). */
static const char *base_name(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *back = strrchr(path, '\\');
    const char *last = (slash != NULL && slash > back) ? slash : back;
    return (last != NULL) ? last + 1 : path;
}

/* Baris status untuk header menu: file aktif atau belum ada. */
static void status_text(const MenuState *st, char *buf, size_t size) {
    if (st == NULL || !st->has_text) {
        snprintf(buf, size, "Belum ada file dimuat");
        return;
    }
    snprintf(buf, size, "File aktif: %s (%d karakter)",
             base_name(st->text.path), (int)st->text.length);
}

/* Membaca pilihan angka [min,max] dengan prompt seragam; jawaban salah
 * diwarn lalu diulang. Kembali 1 saat terisi, 0 saat EOF. */
static int menu_choice(int min, int max, int *out) {
    char buf[UTIL_MAX_LINE];
    long lv;
    char *end;
    for (;;) {
        ui_prompt_label();
        if (util_read_line(NULL, buf, sizeof(buf)) != ST_OK) return 0;
        util_trim(buf);
        errno = 0;
        lv = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && errno != ERANGE &&
            lv >= (long)min && lv <= (long)max) {
            *out = (int)lv;
            return 1;
        }
        ui_warn("Masukkan angka %d sampai %d.", min, max);
    }
}

/* Menu 1: muat file teks; teks lama dibebaskan dulu. */
static void menu_load_text(MenuState *st) {
    char path[UTIL_MAX_PATH];
    Status s;
    if (ui_read_path("Path file > ", path, sizeof(path)) != ST_OK) return;
    tt_free_text(&st->text);
    s = tt_load_text(path, &st->text);
    if (s == ST_OK) {
        st->has_text = 1;
        analysis_invalidate(st);
        ui_ok("%s (%d karakter)", base_name(path), (int)st->text.length);
    } else {
        st->has_text = 0;
        ui_error("%s", util_status_message(s));
    }
}

/* Lebar angka dalam digit untuk rata kanan label hasil. */
static int num_digits(int n) {
    int d = 1;
    while (n >= 10) {
        n /= 10;
        d++;
    }
    return d;
}

/* Daftar hasil grep: ringkasan di atas, paling banyak 40 baris isi,
 * sisanya diringkas; tiap isi baris dipotong sesuai sisa lebar. */
static void grep_print(const TtMatch *m, int count, int total, int invert) {
    int lw = 1, cw = 1, i, show, label;
    ui_section("Hasil grep");
    ui_info("Baris cocok: %d, total kemunculan: %d", count, total);
    for (i = 0; i < count; i++) {
        if (num_digits(m[i].line) > lw) lw = num_digits(m[i].line);
        if (num_digits(m[i].col) > cw) cw = num_digits(m[i].col);
    }
    label = lw + cw + 5;
    show = count < 40 ? count : 40;
    for (i = 0; i < show; i++) {
        if (invert) {
            printf("%*d | ", lw, m[i].line);
            ui_print_truncated(m[i].text, UI_WIDTH - lw - 3);
        } else {
            printf("%*d:%*d | ", lw, m[i].line, cw, m[i].col);
            ui_print_truncated(m[i].text, UI_WIDTH - label);
        }
        putchar('\n');
    }
    if (count > show) {
        ui_info("... dan %d lainnya (tidak ditampilkan)", count - show);
    }
}

/* Menu 2: mini grep dengan konfirmasi opsi dan daftar hasil terbatas. */
static void menu_grep(const MenuState *st) {
    char pattern[UTIL_MAX_LINE];
    TtMatch *matches = NULL;
    int count = 0, total = 0, ignore_case, invert;
    Status s;
    if (!st->has_text) {
        ui_warn("Muat file lewat menu 1 lebih dahulu.");
        return;
    }
    if (util_read_line("Pola > ", pattern, sizeof(pattern)) != ST_OK) return;
    util_trim(pattern);
    if (ui_confirm("Abaikan huruf besar/kecil?", 1, &ignore_case) != ST_OK) return;
    if (ui_confirm("Tampilkan baris yang tidak cocok?", 0, &invert) != ST_OK) return;
    s = tt_grep(&st->text, pattern, ignore_case, invert, &matches, &count, &total);
    if (s == ST_OK) {
        grep_print(matches, count, total, invert);
    } else {
        ui_error("%s", util_status_message(s));
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

/* Tabel frekuensi kata: kolom kata rata kiri, jumlah rata kanan. */
static void freq_print(const TtWordFreq *f, int count) {
    int w = 0, cw = 0, i;
    for (i = 0; i < count; i++) {
        size_t len = strlen(f[i].word);
        if ((int)len > w) w = (int)len;
        if (num_digits(f[i].count) > cw) cw = num_digits(f[i].count);
    }
    if (w < 4) w = 4;
    ui_section("Frekuensi kata (terbanyak dulu)");
    printf(" Kata  | Jumlah\n");
    ui_rule();
    for (i = 0; i < count; i++) {
        printf(" %-*s | %*d\n", w, f[i].word, cw, f[i].count);
    }
}

/* Sub-menu 3 opsi 4: binary search kata pada salinan alfabetis. */
static void analysis_search_word(const MenuState *st) {
    TtWordFreq *sorted;
    char word[UTIL_MAX_LINE];
    int idx;
    if (util_read_line("Kata > ", word, sizeof(word)) != ST_OK) return;
    util_trim(word);
    util_to_lower(word);
    sorted = (TtWordFreq *)util_calloc((size_t)st->freq_count, sizeof(TtWordFreq));
    if (sorted == NULL) {
        ui_error("%s", util_status_message(ST_NO_MEMORY));
        return;
    }
    memcpy(sorted, st->freq, (size_t)st->freq_count * sizeof(TtWordFreq));
    tt_sort_words_alpha(sorted, st->freq_count);
    idx = tt_binary_search_word(sorted, st->freq_count, word);
    if (idx >= 0) {
        ui_info("Ditemukan: %s (%d kali)", sorted[idx].word, sorted[idx].count);
    } else {
        ui_warn("Kata tidak ditemukan.");
    }
    util_free(sorted);
}

/* Sub-menu 3: statistik, histogram, frekuensi kata, binary search. */
static void menu_analyze(MenuState *st) {
    int pilihan;
    char status[UTIL_MAX_LINE];
    if (analysis_run(st) != ST_OK) {
        ui_warn("Muat file lewat menu 1 lebih dahulu.");
        return;
    }
    status_text(st, status, sizeof(status));
    for (;;) {
        printf("\n");
        ui_screen_header("Analisis teks", status);
        ui_menu_item(1, "Statistik");
        ui_menu_item(2, "Histogram huruf");
        ui_menu_item(3, "Frekuensi kata");
        ui_menu_item(4, "Cari kata (binary search)");
        ui_menu_item(0, "Kembali");
        ui_rule();
        if (!menu_choice(0, 4, &pilihan)) return;
        if (pilihan == 0) break;
        if (pilihan == 1) {
            printf("Karakter : %d\nKata     : %d\nKalimat  : %d\nBaris    : %d\n",
                   st->stats.chars, st->stats.words, st->stats.sentences,
                   st->stats.lines);
        } else if (pilihan == 2) {
            tt_print_letter_histogram(&st->stats);
        } else if (pilihan == 3) {
            freq_print(st->freq, st->freq_count);
        } else if (pilihan == 4) {
            analysis_search_word(st);
        }
        ui_rule();
        printf("\n");
    }
}

/* Menu 4 opsi 4: proses file ke file sesuai mode tt_cipher_file. */
static void cipher_file_to_file(void) {
    char in_path[UTIL_MAX_PATH], out_path[UTIL_MAX_PATH], key[UTIL_MAX_LINE];
    int mode, shift;
    Status s;
    printf("Mode: 0=Caesar enkripsi, 1=Caesar dekripsi, 2=Vigenere enkripsi, 3=Vigenere dekripsi\n");
    if (util_read_int("Mode > ", 0, 3, &mode) != ST_OK) return;
    if (ui_read_path("File sumber > ", in_path, sizeof(in_path)) != ST_OK) return;
    if (ui_read_path("File hasil > ", out_path, sizeof(out_path)) != ST_OK) return;
    shift = 0;
    if (mode == 0 || mode == 1) {
        if (util_read_int("Geser > ", -1000000, 1000000, &shift) != ST_OK) return;
    } else {
        if (util_read_line("Kunci (huruf saja) > ", key, sizeof(key)) != ST_OK) return;
        util_trim(key);
    }
    s = tt_cipher_file(in_path, out_path, mode, shift, key);
    if (s == ST_OK) ui_ok("Hasil ditulis ke %s", out_path);
    else ui_error("%s", util_status_message(s));
}

/* Satu baris hasil sandi dengan label geser dan teks terpotong. */
static void cipher_print_line(const char *label, const char *text) {
    printf("%s", label);
    ui_print_truncated(text, UI_WIDTH - (int)strlen(label));
    putchar('\n');
}

/* Menu 4: sandi teks yang diketik dan proses file ke file. */
static void menu_cipher(void) {
    int pilihan;
    for (;;) {
        printf("\n");
        ui_screen_header("Cipher", "Teks diketik; tanpa file dimuat");
        ui_menu_item(1, "Caesar (teks diketik)");
        ui_menu_item(2, "Vigenere (teks diketik)");
        ui_menu_item(3, "Brute force Caesar");
        ui_menu_item(4, "File ke file");
        ui_menu_item(0, "Kembali");
        ui_rule();
        if (!menu_choice(0, 4, &pilihan)) return;
        if (pilihan == 0) break;
        if (pilihan == 1) {
            char text[UTIL_MAX_LINE];
            int shift;
            if (util_read_line("Teks > ", text, sizeof(text)) != ST_OK) return;
            if (util_read_int("Geser (negatif untuk dekripsi) > ", -1000000, 1000000, &shift) != ST_OK) return;
            tt_caesar(text, shift);
            cipher_print_line("Hasil geser : ", text);
        } else if (pilihan == 2) {
            char text[UTIL_MAX_LINE], key[UTIL_MAX_LINE];
            int enc;
            if (util_read_line("Teks > ", text, sizeof(text)) != ST_OK) return;
            if (ui_confirm("Enkripsi? (n = dekripsi)", 1, &enc) != ST_OK) return;
            if (util_read_line("Kunci (huruf saja) > ", key, sizeof(key)) != ST_OK) return;
            util_trim(key);
            if (tt_vigenere(text, key, enc ? 0 : 1) != ST_OK) {
                ui_error("%s", util_status_message(ST_BAD_INPUT));
            } else {
                cipher_print_line("Hasil vigenere : ", text);
            }
        } else if (pilihan == 3) {
            char text[UTIL_MAX_LINE];
            if (util_read_line("Teks terenkripsi > ", text, sizeof(text)) != ST_OK) return;
            tt_caesar_brute(text);
        } else if (pilihan == 4) {
            cipher_file_to_file();
        }
        ui_rule();
        printf("\n");
    }
}

/* Menu 5: cek palindrom dan hasil pembalik string dari teks diketik. */
static void menu_palindrome(void) {
    char text[UTIL_MAX_LINE], rev[UTIL_MAX_LINE];
    if (util_read_line("Teks > ", text, sizeof(text)) != ST_OK) return;
    tt_reverse_rec(text, rev);
    ui_info("Palindrom: %s", tt_palindrome_rec(text) ? "ya" : "bukan");
    cipher_print_line("Reverse   : ", rev);
}

/* Menu 6: tulis laporan; analisis dijalankan dulu bila belum ada. */
static void menu_report(MenuState *st) {
    char out_path[UTIL_MAX_PATH];
    Status s;
    if (!st->has_text) {
        ui_warn("Muat file lewat menu 1 lebih dahulu.");
        return;
    }
    if (!st->has_stats || !st->has_freq) {
        s = analysis_run(st);
        if (s != ST_OK) {
            ui_error("%s", util_status_message(s));
            return;
        }
    }
    if (util_read_line("Nama file laporan (kosong = ke layar) > ", out_path, sizeof(out_path)) != ST_OK) return;
    util_trim(out_path);
    ui_strip_quotes(out_path);
    if (out_path[0] == '\0') {
        s = tt_write_report(&st->stats, st->freq, st->freq_count, st->text.path, NULL);
    } else {
        s = tt_write_report(&st->stats, st->freq, st->freq_count, st->text.path, out_path);
        if (s == ST_OK) ui_ok("Laporan ditulis ke %s", out_path);
    }
    if (s != ST_OK && out_path[0] != '\0') {
        ui_error("%s", util_status_message(s));
    }
}

/* Membebaskan seluruh memori yang masih dipegang state. */
static void state_free(MenuState *st) {
    tt_free_text(&st->text);
    st->has_text = 0;
    analysis_invalidate(st);
}

/* Menu utama Text Toolkit; keluar (0 atau EOF) membebaskan semua memori. */
void run_text_toolkit(void) {
    MenuState st;
    int pilihan;
    memset(&st, 0, sizeof(st));
    ui_banner("TEXT TOOLKIT", "Mini grep, analisis, sandi, dan laporan",
              "Ketik angka menu; 0 untuk keluar");
    for (;;) {
        char status[UTIL_MAX_LINE];
        status_text(&st, status, sizeof(status));
        printf("\n");
        ui_screen_header("TEXT TOOLKIT", status);
        ui_menu_item(1, "Muat file teks");
        ui_menu_item(2, "Mini grep");
        ui_menu_item(3, "Analisis teks");
        ui_menu_item(4, "Cipher (Caesar / Vigenere / brute force)");
        ui_menu_item(5, "Palindrom dan reverse");
        ui_menu_item(6, "Tulis laporan");
        ui_menu_item(0, "Keluar");
        ui_rule();
        if (!menu_choice(0, 6, &pilihan)) break;
        if (pilihan == 0) break;
        if (pilihan == 1) menu_load_text(&st);
        else if (pilihan == 2) menu_grep(&st);
        else if (pilihan == 3) menu_analyze(&st);
        else if (pilihan == 4) menu_cipher();
        else if (pilihan == 5) menu_palindrome();
        else if (pilihan == 6) menu_report(&st);
        ui_rule();
        printf("\n");
    }
    state_free(&st);
}
