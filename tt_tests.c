/* Test harness Text Toolkit: U01 sampai U17 plus uji tambahan PRD 12.2.
 * Semua pemeriksaan memakai util_check; berkas sementara dihapus setelah
 * dipakai. Kembali 0 saat seluruh uji lulus. */
#include "text_toolkit.h"
#include "common/common.h"
#include "tt_text/tt_text.h"
#include "tt_grep/tt_grep.h"
#include "tt_analyzer/tt_analyzer.h"
#include "tt_cipher/tt_cipher.h"
#include "tt_report/tt_report.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Menulis berkas teks sementara untuk pengujian. */
static Status write_temp(const char *path, const char *content) {
    return util_write_file(path, content, strlen(content));
}

/* U01 dan U02: statistik dan frekuensi kata "Halo, halo! Apa kabar?". */
static void test_stats_and_freq(void) {
    const char *teks = "Halo, halo! Apa kabar?";
    TtStats s;
    TtWordFreq *f = NULL;
    int n = 0, i, total_letter = 0;
    util_test_begin("U01-U02 statistik dan frekuensi kata");
    util_check(tt_compute_stats(teks, &s) == ST_OK, "U01 stats ST_OK");
    util_check(s.chars == 22, "U01 karakter = 22");
    util_check(s.words == 4, "U01 kata = 4");
    util_check(s.sentences == 2, "U01 kalimat = 2");
    util_check(s.lines == 1, "U01 baris = 1");
    util_check(s.letters[0] == 6, "U01 huruf A = 6");
    util_check(s.letters[1] == 1, "U01 huruf B = 1");
    util_check(s.letters[7] == 2, "U01 huruf H = 2");
    for (i = 0; i < 26; i++) total_letter += s.letters[i];
    util_check(total_letter == 16, "U01 jumlah huruf = 16");
    util_check(tt_word_frequency(teks, &f, &n) == ST_OK, "U02 freq ST_OK");
    util_check(n == 3, "U02 3 kata unik");
    tt_sort_freq_desc(f, n);
    util_check(strcmp(f[0].word, "halo") == 0 && f[0].count == 2, "U02 halo = 2");
    util_check(strcmp(f[1].word, "apa") == 0 && f[1].count == 1, "U02 apa = 1");
    util_check(strcmp(f[2].word, "kabar") == 0 && f[2].count == 1, "U02 kabar = 1");
    util_free(f);
    util_check(util_test_end() == 0, "U01-U02 lulus");
}

/* U03 sampai U05: Caesar, normalisasi geser, dan roundtrip. */
static void test_caesar(void) {
    char s1[64], s2[64], s3[64];
    util_test_begin("U03-U05 Caesar");
    strcpy(s1, "Hello, World!");
    util_check(tt_caesar(s1, 3) == ST_OK, "U03 ST_OK");
    util_check(strcmp(s1, "Khoor, Zruog!") == 0, "U03 geser 3");
    strcpy(s2, "Abc");
    util_check(tt_caesar(s2, 26) == ST_OK && strcmp(s2, "Abc") == 0, "U04 geser 26 = asli");
    strcpy(s3, "Hello, World!");
    tt_caesar(s3, 29);
    util_check(strcmp(s3, "Khoor, Zruog!") == 0, "U04 geser 29 = geser 3");
    strcpy(s3, "abc");
    tt_caesar(s3, -3);
    util_check(strcmp(s3, "xyz") == 0, "tambahan geser -3");
    strcpy(s3, "Khoor, Zruog!");
    tt_caesar(s3, -3);
    util_check(strcmp(s3, "Hello, World!") == 0, "U05 dekripsi Caesar");
    util_check(util_test_end() == 0, "U03-U05 lulus");
}

/* U06 dan U07: Vigenere referensi dan penolakan kunci. */
static void test_vigenere(void) {
    char s1[32], s2[32];
    util_test_begin("U06-U07 Vigenere");
    strcpy(s1, "ATTACKATDAWN");
    util_check(tt_vigenere(s1, "LEMON", 0) == ST_OK, "U06 ST_OK");
    util_check(strcmp(s1, "LXFOPVEFRNHR") == 0, "U06 ATTACKATDAWN+LEMON");
    tt_vigenere(s1, "LEMON", 1);
    util_check(strcmp(s1, "ATTACKATDAWN") == 0, "U05 dekripsi Vigenere");
    strcpy(s2, "Halo");
    util_check(tt_vigenere(s2, "", 0) == ST_BAD_INPUT, "U07 kunci kosong ditolak");
    util_check(strcmp(s2, "Halo") == 0, "U07 teks tidak berubah");
    util_check(tt_vigenere(s2, "123abc", 0) == ST_BAD_INPUT, "U07 kunci angka ditolak");
    util_check(util_test_end() == 0, "U06-U07 lulus");
}

/* U08 sampai U10: palindrom dan pembalik rekursif. */
static void test_rec(void) {
    char rev[32];
    util_test_begin("U08-U10 palindrom dan reverse");
    util_check(tt_palindrome_rec("A man, a plan, a canal: Panama") == 1, "U08 Panama");
    util_check(tt_palindrome_rec("Katak") == 1, "U08 Katak");
    util_check(tt_palindrome_rec("Halo") == 0, "U09 Halo bukan palindrom");
    util_check(tt_palindrome_rec("") == 1, "tambahan string kosong");
    tt_reverse_rec("abc", rev);
    util_check(strcmp(rev, "cba") == 0, "U10 reverse abc");
    tt_reverse_rec("", rev);
    util_check(strcmp(rev, "") == 0, "tambahan reverse kosong");
    util_check(util_test_end() == 0, "U08-U10 lulus");
}

/* U11 dan U12: mini grep pada teks tiga baris PRD 8.1. */
static void test_grep(void) {
    TtText t;
    TtMatch *m = NULL;
    int n = 0, total = 0;
    util_test_begin("U11-U12 mini grep");
    util_check(write_temp("tests/tmp_grep.txt",
                          "Halo dunia.\nDunia itu luas.\nHalo lagi, halo semua!\n") == ST_OK,
               "tulis berkas uji grep");
    util_check(tt_load_text("tests/tmp_grep.txt", &t) == ST_OK, "muat berkas grep");
    util_check(tt_grep(&t, "halo", 0, 0, &m, &n, &total) == ST_OK && n == 1 &&
               m[0].line == 3 && m[0].col == 12 && total == 1, "U11 peka huruf");
    tt_free_matches(m, n);
    util_check(tt_grep(&t, "halo", 1, 0, &m, &n, &total) == ST_OK && n == 2 &&
               m[0].line == 1 && m[0].col == 1 && m[1].line == 3 &&
               m[1].col == 1 && total == 3, "U11 abaikan huruf");
    tt_free_matches(m, n);
    util_check(tt_grep(&t, "halo", 1, 1, &m, &n, &total) == ST_OK && n == 1 &&
               m[0].line == 2 && total == 3, "tambahan invert");
    tt_free_matches(m, n);
    util_check(tt_grep(&t, "matahari", 1, 0, &m, &n, &total) == ST_OK && n == 0,
               "U12 kata tidak ada");
    tt_free_matches(m, n);
    util_check(tt_grep(&t, "", 1, 0, &m, &n, &total) == ST_BAD_INPUT,
               "tambahan pola kosong");
    tt_free_text(&t);
    remove("tests/tmp_grep.txt");
    util_check(util_test_end() == 0, "U11-U12 lulus");
}

/* U13 dan U14: berkas hilang, kosong, dan CRLF. */
static void test_files(void) {
    TtText t1, t2;
    TtStats s1, s2;
    util_test_begin("U13-U14 berkas");
    util_check(tt_load_text("data/tidak_ada.txt", &t1) == ST_FILE_NOT_FOUND, "U13 tidak ada");
    util_check(tt_load_text("data/kosong.txt", &t1) == ST_FILE_EMPTY, "U13 kosong");
    util_check(tt_load_text("data/contoh.txt", &t1) == ST_OK, "muat contoh");
    util_check(tt_compute_stats(t1.data, &s1) == ST_OK, "stats contoh");
    util_check(tt_load_text("data/crlf.txt", &t2) == ST_OK, "muat crlf");
    util_check(tt_compute_stats(t2.data, &s2) == ST_OK, "stats crlf");
    util_check(s1.lines == s2.lines, "U14 baris crlf = baris lf");
    util_check(s1.words == s2.words && s1.chars == s2.chars, "U14 isi sama");
    tt_free_text(&t1);
    tt_free_text(&t2);
    util_check(util_test_end() == 0, "U13-U14 lulus");
}

/* U15: binary search pada daftar alfabetis. */
static void test_binary_search(void) {
    TtWordFreq *f = NULL;
    int n = 0;
    util_test_begin("U15 binary search");
    util_check(tt_word_frequency("Halo, halo! Apa kabar?", &f, &n) == ST_OK, "freq siap");
    tt_sort_words_alpha(f, n);
    util_check(tt_binary_search_word(f, n, "halo") == 1, "U15 halo di indeks 1");
    util_check(tt_binary_search_word(f, n, "Halo") == 1, "U15 Halo abai kapital");
    util_check(tt_binary_search_word(f, n, "xyz") == -1, "U15 xyz tidak ada");
    util_check(tt_binary_search_word(f, n, "") == -1, "tambahan kata kosong");
    util_free(f);
    util_check(util_test_end() == 0, "U15 lulus");
}

/* U16: injeksi kegagalan alokasi dan keseimbangan memori. */
static void test_alloc_failure(void) {
    TtText t;
    TtWordFreq *f = NULL;
    int n = 0;
    util_test_begin("U16 alokasi gagal");
    util_fail_alloc_after(0);
    util_check(tt_load_text("data/contoh.txt", &t) == ST_NO_MEMORY, "U16 load gagal alokasi");
    util_fail_alloc_after(1);
    util_check(tt_word_frequency("a b c d e f g h i j k l m n o p q", &f, &n) == ST_NO_MEMORY,
               "U16 freq gagal realloc");
    util_fail_alloc_after(1);
    util_check(tt_write_report(NULL, NULL, 0, "", "tests/tmp_lapor.txt") == ST_BAD_INPUT,
               "U16 report invalid input");
    util_fail_alloc_after(-1);
    util_check(util_alloc_balance() == 0, "U16 balance kembali 0");
    remove("tests/tmp_lapor.txt");
    util_check(util_test_end() == 0, "U16 lulus");
}

/* Uji tambahan: kata > 39 karakter dipotong, laporan ke folder hilang,
 * dan proses berkas cipher berkas ke berkas. */
static void test_extras(void) {
    TtWordFreq *f = NULL;
    TtStats s;
    TtText t;
    int n = 0;
    char longword[64];
    util_test_begin("Tambahan potong kata, laporan, cipher berkas");
    strcpy(longword, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaX");
    util_check(tt_word_frequency(longword, &f, &n) == ST_OK && n == 1, "freq siap");
    util_check(strlen(f[0].word) == 39, "kata 41 huruf dipotong 39");
    util_free(f);
    util_check(tt_compute_stats("", &s) == ST_OK && s.lines == 0, "teks kosong 0 baris");
    util_check(tt_write_report(&s, NULL, 0, "x", "tests/tidak_ada/lapor.txt") == ST_FILE_WRITE,
               "laporan ke folder tidak ada");
    util_check(write_temp("tests/tmp_in.txt", "Hello, World!\n") == ST_OK, "tulis input cipher");
    util_check(tt_cipher_file("tests/tmp_in.txt", "tests/tmp_out.txt", 0, 3, "") == ST_OK,
               "cipher berkas Caesar");
    util_check(tt_cipher_file("tests/tmp_out.txt", "tests/tmp_rt.txt", 1, 3, "") == ST_OK,
               "cipher berkas roundtrip");
    util_check(tt_load_text("tests/tmp_rt.txt", &t) == ST_OK && strcmp(t.data, "Hello, World!\n") == 0,
               "roundtrip berkas identik");
    tt_free_text(&t);
    util_check(tt_cipher_file("tests/tmp_in.txt", "tests/tidak_ada_f/a.txt", 0, 1, "") == ST_FILE_WRITE,
               "cipher tulis gagal -> ST_FILE_WRITE");
    util_fail_alloc_after(0);
    util_check(tt_load_text("data/contoh.txt", &t) == ST_NO_MEMORY, "injeksi load");
    util_fail_alloc_after(-1);
    remove("tests/tmp_in.txt");    remove("tests/tmp_out.txt");
    remove("tests/tmp_rt.txt");
    util_check(util_test_end() == 0, "tambahan lulus");
}

/* Test harness lengkap: U01 sampai U17 ditambah uji tambahan. */
int test_text_toolkit(void) {
    int fail;
    util_fail_alloc_after(-1);
    test_stats_and_freq();
    test_caesar();
    test_vigenere();
    test_rec();
    test_grep();
    test_files();
    test_binary_search();
    test_alloc_failure();
    test_extras();
    util_fail_alloc_after(-1);
    fail = 0;
    util_test_begin("U17 akhir");
    util_check(util_alloc_balance() == 0, "U17 util_alloc_balance == 0");
    fail = util_test_end();
    printf("%s\n", fail == 0 ? "SEMUA UJI LULUS" : "ADA UJI GAGAL");
    return fail == 0 ? 0 : 1;
}
