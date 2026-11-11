# Text Toolkit

Program menu berbahasa C99 untuk mengolah berkas teks: mini grep, analisis
teks, Binary Search kata, sandi Caesar/Vigenere, serta palindrom dan reverse.
Projek UAS Dasar Pemrograman (INF124102), Ganjil 2026/2027.

## Cara menjalankan

Windows (MinGW-w64):

```
build.bat
```

Linux/WSL:

```
make
./text_toolkit --test   # jalankan seluruh uji
./text_toolkit          # masuk menu
```

Tanpa argumen program membuka menu utama; argumen `--test` menjalankan
test harness U01-U17 dan keluar dengan kode 0 saat lulus.

## Format berkas

- Berkas masukan dan keluaran adalah berkas teks biasa (ASCII/UTF-8 per byte).
- `data/contoh.txt` teks demo; `data/kosong.txt` 0 byte; `data/satu_baris.txt`
  satu baris tanpa `\n` akhir; `data/crlf.txt` isi `contoh.txt` dengan akhir
  baris `\r\n` (dijaga byte-persis lewat `.gitattributes`).
- Laporan (`tt_write_report`) memuat path sumber, statistik, frekuensi huruf,
  dan frekuensi kata terurut (lihat `data` laporan pada menu 6).

## Daftar fungsi publik

| Modul | Fungsi |
|---|---|
| `tt_text` | `tt_load_text`, `tt_free_text` |
| `tt_grep` | `tt_grep`, `tt_free_matches` |
| `tt_analyzer` | `tt_compute_stats`, `tt_word_frequency`, `tt_sort_freq_desc`, `tt_sort_words_alpha`, `tt_binary_search_word`, `tt_print_letter_histogram` |
| `tt_cipher` | `tt_caesar`, `tt_vigenere`, `tt_cipher_file`, `tt_caesar_brute`, `tt_count_letters`, `tt_reverse_rec`, `tt_palindrome_rec` |
| `tt_report` | `tt_write_report` |
| `text_toolkit` | `run_text_toolkit`, `test_text_toolkit` |

Pustaka bersama di `common/`: `Status`, `UTIL_MAX_LINE`, `UTIL_MAX_PATH`,
`util_read_line`, `util_read_int`, `util_trim`, `util_to_lower`,
`util_stricmp`, `util_read_file`, `util_write_file`, `util_malloc` sampai
`util_free`, pelacak alokasi, dan test harness (`util_test_begin`, `util_check`,
`util_check_double`, `util_test_end`).

## Daftar uji

`--test` menjalankan U01-U17 sesuai PRD Bagian 12.2 ditambah uji tambahan:
grep invert, palindrom string kosong, kata 41 huruf dipotong 39, geser negatif
Caesar, pola grep kosong, laporan ke folder yang tidak ada, dan proses berkas
ke berkas dibandingkan dengan `tests/expected_caesar3.txt` serta
`tests/expected_vigenere_lemon.txt` yang dihitung terpisah dari program.
Setiap kelompok uji diakhiri `util_alloc_balance() == 0`.

## Keputusan desain

1. Menu 3 punya sub-menu (statistik, histogram, frekuensi kata, Binary Search)
   karena PRD tidak menetapkan satu layar untuk keempat analisis.
2. Test harness diletakkan di `tt_tests.c` (bukan `text_toolkit.c`) agar
   kedua berkas tetap di bawah 400 baris; pintu masuknya tetap
   `test_text_toolkit()` yang dideklarasikan di `text_toolkit.h`.
3. Report builder tunggal di `tt_report.c` dipakai untuk keluaran layar
   (nama berkas kosong di menu 6) dan berkas, sehingga isi keduanya identik.
4. Kata adalah deretan huruf/angka; apostrof dianggap pemisah; karakter
   statistik tidak menghitung `\n` dan `\r`.
5. Laporan memakai akhir baris `\r\n` agar tampil sama di Windows dan
   editor Unix.
