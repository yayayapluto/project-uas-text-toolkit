# PRD Text Toolkit

| | |
|---|---|
| **Produk** | Text Toolkit (projek 01 dalam `uas-suite/`) |
| **Mata kuliah** | Dasar Pemrograman (INF124102), S1 Informatika UPNVJ, Ganjil 2026/2027 |
| **Versi dokumen** | 1.0, 11 November 2026 |
| **Folder** | `01-text-toolkit/` |
| **Prefix fungsi publik** | `tt_` |
| **Pintu masuk** | `void run_text_toolkit(void)` dan `int test_text_toolkit(void)` |
| **Tim** | 5 orang |
| **Sumber spesifikasi** | Dokumen *Ide Projek UAS Daspro*, Bagian 0, 1.1, 2.0, 2.1, dan 2.5 |

Dokumen ini menguraikan kebutuhan Text Toolkit sampai tingkat yang cukup untuk dikerjakan, diuji, dan didemonstrasikan. Nama fungsi, tipe, dan format file mengikuti dokumen sumber. Jika ada perbedaan, dokumen sumber yang berlaku. Keputusan yang diambil di sini karena sumber belum menetapkannya ditandai **[Keputusan]** dan dikumpulkan di Bagian 15.

---

## 1. Ringkasan

Text Toolkit adalah program menu berbahasa C yang mengolah berkas teks. Isinya lima alat: pencari pola dengan nomor baris dan kolom (*mini grep*), penganalisis statistik teks, pencarian kata dengan Binary Search, sandi Caesar dan Vigenère, serta utilitas palindrom dan pembalik string. Hasil analisis dapat ditulis ke berkas laporan.

Program berjalan di terminal, hanya memakai pustaka standar C99, dan menjadi salah satu dari empat projek yang diluncurkan lewat satu menu utama (`./suite`).

## 2. Latar belakang dan tujuan

### 2.1 Latar belakang

Projek menyumbang 50% nilai akhir praktikum dan dinilai lewat penilaian projek, demonstrasi, dan tanya jawab (minggu 16). Indikator penilaiannya menuntut satu program modular yang memadukan struktur kontrol, fungsi, array dan string, pointer, struct, searching dan sorting, serta file teks dengan penanganan kesalahan. Pengujian dengan *test case* dan kemampuan menjelaskan logika program juga dinilai.

Text Toolkit dipilih sebagai projek yang paling aman. Risiko teknisnya rendah dan pekerjaannya mudah dibagi merata ke lima orang. Kekurangannya adalah kesan "wow" yang biasa saja, sehingga nilai tambahnya harus datang dari kerapian kode, kelengkapan *test case*, dan penanganan error yang matang.

### 2.2 Sasaran produk

| Kode | Sasaran | Ukuran keberhasilan |
|---|---|---|
| S1 | Seluruh fitur pada Bagian 6 berfungsi sungguhan, bukan *stub* | Semua kebutuhan P0 lulus kriteria penerimaan |
| S2 | Program tidak crash pada input apa pun | Input salah, berkas hilang, kosong, atau rusak selalu menghasilkan pesan yang jelas |
| S3 | Setiap anggota dapat menjelaskan bagian yang dipegangnya | Tiap modul punya satu pemilik dan komentar fungsi dalam bahasa Indonesia |
| S4 | Tidak ada kebocoran memori | `util_alloc_balance() == 0` di akhir tiap `test_text_toolkit()` |
| S5 | Cakupan materi RPS terpenuhi | Tabel keterlacakan di Bagian 13 terisi penuh |

### 2.3 Di luar cakupan

- Pencarian dengan ekspresi reguler. Pola yang dicari adalah teks biasa.
- Pembacaan berkas biner, termasuk teks berenkode selain ASCII atau UTF-8 yang diperlakukan per byte.
- Antarmuka grafis, warna terminal, dan fungsi khusus sistem operasi (`system()`, `cls`, dan sejenisnya).
- Pencarian ke banyak berkas atau ke dalam folder secara rekursif.
- Sandi selain Caesar dan Vigenère.
- Fitur ubah atau hapus isi berkas sumber. Program hanya membaca berkas masukan dan menulis berkas keluaran yang diminta.

## 3. Pengguna dan skenario

| Pengguna | Kebutuhan | Skenario utama |
|---|---|---|
| Mahasiswa pengguna (anggota tim saat uji coba) | Mengolah satu berkas teks dengan beberapa alat dari satu menu | Memuat `contoh.txt`, mencari kata, menganalisis, lalu menulis laporan |
| Dosen atau asisten penguji | Melihat program berjalan, menguji batas, dan menanyakan logika | Memasukkan berkas kosong, kunci Vigenère berisi angka, atau pilihan menu di luar rentang |
| Anggota tim lain | Mengintegrasikan modul tanpa bentrok | Memanggil fungsi `tt_` lewat header, tanpa menyentuh isi modul lain |

## 4. Gambaran fitur

| Modul | File | Isi | Pemilik |
|---|---|---|---|
| Menu dan muat teks | `text_toolkit.c`, `tt_text.c` | Menu, pemuatan dan pembebasan teks, *test harness* | Anggota 1 |
| Mini grep | `tt_grep.c` | Pencarian pola dengan nomor baris dan kolom | Anggota 2 |
| Analyzer | `tt_analyzer.c` | Statistik, frekuensi huruf dan kata, sorting kata | Anggota 3 |
| Cipher dan utilitas | `tt_cipher.c` | Caesar, Vigenère, *brute force*, palindrom, pembalik string | Anggota 4 |
| Laporan dan pencarian kata | `tt_report.c` | Laporan ke berkas, Binary Search, integrasi, data uji | Anggota 5 |

## 5. Alur pengguna

Menu Text Toolkit:

```
=== TEXT TOOLKIT ===
1. Muat file teks
2. Mini grep
3. Analisis teks (statistik, frekuensi huruf, frekuensi kata)
4. Cipher (Caesar / Vigenere / brute force)
5. Palindrom dan reverse
6. Tulis laporan
0. Kembali
```

Aturan alur:

1. Menu 2, 3, dan 6 memerlukan teks yang sudah dimuat. Jika belum, program menampilkan pesan agar pengguna memilih menu 1 lebih dahulu, lalu kembali ke menu.
2. Menu 4 dan 5 dapat dipakai tanpa memuat berkas, karena keduanya menerima teks yang diketik. Menu 4 juga menyediakan proses berkas ke berkas.
3. Memuat berkas baru melepaskan teks lama sebelum menggantinya.
4. Semua input lewat fungsi `util_read_*`. Pilihan menu di luar rentang atau berisi huruf diulang sampai valid.
5. Keluar dari menu (pilihan 0) melepaskan seluruh memori yang masih dipegang.

## 6. Kebutuhan fungsional

Prioritas **P0** berarti wajib ada sebelum demonstrasi. **P1** adalah opsi yang tercantum dalam spesifikasi dan dikerjakan setelah seluruh P0 lulus uji.

### 6.1 Muat teks (`tt_text`)

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-01 | Memuat seluruh isi berkas teks ke memori dinamis lewat `tt_load_text` | P0 | Isi, panjang, dan path tersimpan pada `TtText` |
| TT-02 | Berkas yang tidak ada menghasilkan `ST_FILE_NOT_FOUND` | P0 | Pesan jelas, tidak crash, tidak ada alokasi tersisa |
| TT-03 | Berkas 0 byte menghasilkan `ST_FILE_EMPTY` | P0 | Berlaku untuk `data/kosong.txt` |
| TT-04 | Akhir baris `\r\n` diperlakukan sama dengan `\n` | P0 | Jumlah baris `crlf.txt` sama dengan versi `\n` |
| TT-05 | `tt_free_text` melepaskan data dan menjadikan pointer `NULL` | P0 | Pemanggilan ganda tidak menimbulkan galat |

### 6.2 Mini grep (`tt_grep`)

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-10 | Mencari kata atau pola, lalu menampilkan nomor baris dan kolom kemunculan pertama pada tiap baris yang cocok (kolom mulai dari 1) | P0 | Hasil sama dengan hitungan manual |
| TT-11 | Opsi abaikan huruf besar dan kecil, memakai fungsi pencarian buatan sendiri karena `strstr` peka huruf | P0 | Hasil dengan dan tanpa opsi berbeda sesuai hitungan manual |
| TT-12 | Opsi hanya menghitung kemunculan | P1 | Keluaran berupa angka `total_occurrences` saja |
| TT-13 | Opsi menampilkan baris yang tidak cocok (`invert`) | P1 | Keluaran berisi tepat baris yang tidak memuat pola |
| TT-14 | Kata yang tidak ditemukan bukan error | P0 | `match_count = 0` dan status `ST_OK` |
| TT-15 | `total_occurrences` menghitung semua kemunculan, termasuk lebih dari satu pada baris yang sama | P0 | Contoh pada Bagian 8.1 |
| TT-16 | Salinan baris pada `TtMatch` dilepas oleh `tt_free_matches` | P0 | `util_alloc_balance() == 0` |

### 6.3 Analyzer (`tt_analyzer`)

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-20 | Menghitung jumlah karakter, kata, kalimat, dan baris (`tt_compute_stats`) sesuai definisi di Bagian 7 | P0 | Teks `"Halo, halo! Apa kabar?"` menghasilkan 22, 4, 2, 1 |
| TT-21 | Menghitung frekuensi tiap huruf A sampai Z tanpa membedakan huruf besar dan kecil | P0 | Jumlah seluruh `letters[]` sama dengan jumlah huruf pada teks |
| TT-22 | Menampilkan histogram teks frekuensi huruf (`tt_print_letter_histogram`) | P0 | Contoh pada Bagian 8.2 |
| TT-23 | Menghitung frekuensi kata dengan array dinamis (`tt_word_frequency`), kapasitas awal kecil dan digandakan lewat `realloc`, pengecekan kata yang sudah ada memakai Sequential Search | P0 | Teks yang sama menghasilkan `halo`=2, `apa`=1, `kabar`=1 |
| TT-24 | Mengurutkan frekuensi kata dari yang terbanyak dengan Selection Sort, lalu alfabetis jika jumlahnya sama (`tt_sort_freq_desc`) | P0 | Urutan stabil dan deterministik |
| TT-25 | Mengurutkan daftar kata unik secara alfabetis dengan Bubble Sort (`tt_sort_words_alpha`) | P0 | Terurut naik, tanpa kata hilang |
| TT-26 | Menghitung huruf kapital, huruf kecil, dan karakter lain (`tt_count_letters`) dengan keluaran ganda lewat pointer | P1 | Jumlah ketiganya sama dengan panjang string |
| TT-27 | Kata lebih panjang dari `TT_MAX_WORD` (40) dipotong dengan aman | P0 | Tidak ada tulis di luar buffer |

### 6.4 Pencarian kata (Binary Search)

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-30 | Mencari kata di daftar unik yang terurut alfabetis lewat `tt_binary_search_word` | P0 | Mengembalikan indeks jika ada dan `-1` jika tidak |
| TT-31 | Menu menjamin daftar sudah terurut alfabetis sebelum pencarian | P0 | Pencarian pada daftar yang urutannya masih berdasarkan frekuensi tidak pernah terjadi |
| TT-32 | Pencarian tidak membedakan huruf besar dan kecil | P0 | `Halo` dan `halo` menghasilkan indeks yang sama |

### 6.5 Cipher (`tt_cipher`)

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-40 | Sandi Caesar in-place dengan geser positif, negatif, atau besar (`tt_caesar`) | P0 | `"Hello, World!"` dengan geser 3 menjadi `"Khoor, Zruog!"` |
| TT-41 | Geser dinormalisasi dengan `((shift % 26) + 26) % 26` | P0 | Geser 26 menghasilkan teks asli, geser 29 sama dengan geser 3 |
| TT-42 | Karakter selain huruf tidak berubah, huruf besar memakai basis `'A'` dan huruf kecil `'a'` | P0 | Tanda baca dan angka utuh |
| TT-43 | Sandi Vigenère enkripsi dan dekripsi in-place (`tt_vigenere`), indeks kunci maju hanya pada karakter huruf, kapitalisasi teks dipertahankan | P0 | `"ATTACKATDAWN"` dengan kunci `"LEMON"` menjadi `"LXFOPVEFRNHR"` |
| TT-44 | Kunci kosong atau memuat selain huruf ditolak dengan `ST_BAD_INPUT` | P0 | Teks tidak berubah ketika kunci ditolak |
| TT-45 | *Brute force* Caesar menampilkan 26 kemungkinan geser (0 sampai 25) | P0 | Tepat 26 baris, satu per geser |
| TT-46 | Proses berkas ke berkas (`tt_cipher_file`) | P1 | Hasil enkripsi lalu dekripsi identik dengan berkas awal, lihat `tests/` |
| TT-47 | Enkripsi lalu dekripsi mengembalikan teks asli untuk Caesar dan Vigenère | P0 | Diuji untuk beberapa geser dan kunci |

### 6.6 Palindrom dan pembalik string

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-50 | Pembalik string rekursif (`tt_reverse_rec`) | P0 | `"abc"` menjadi `"cba"` |
| TT-51 | Cek palindrom rekursif (`tt_palindrome_rec`) yang melewati karakter non-alfanumerik dan mengabaikan huruf besar dan kecil | P0 | `"A man, a plan, a canal: Panama"` dan `"Katak"` menghasilkan 1, `"Halo"` menghasilkan 0 |
| TT-52 | String kosong dianggap palindrom | P0 | Hasil 1 |

### 6.7 Laporan (`tt_report`)

| ID | Kebutuhan | Prioritas | Kriteria penerimaan |
|---|---|---|---|
| TT-60 | Menulis hasil analisis ke berkas teks lewat `tt_write_report` | P0 | Isi berkas sama dengan keluaran layar |
| TT-61 | Laporan memuat path sumber, statistik, tabel frekuensi huruf, dan frekuensi kata terurut | P0 | Format pada Bagian 8.3 |
| TT-62 | Kegagalan menulis (folder tidak ada, tidak berizin) menghasilkan `ST_FILE_WRITE` | P0 | Pesan jelas, tidak crash |

## 7. Definisi baku

Ketiga definisi pertama harus konsisten di seluruh modul. Perbedaan tafsir antaranggota adalah sumber galat paling lazim pada projek ini.

| Istilah | Definisi |
|---|---|
| **Kata** | Deretan huruf atau angka yang berurutan. Apostrof dianggap pemisah. **[Keputusan]** |
| **Kalimat** | Satu atau lebih tanda akhir `.`, `!`, `?` yang berurutan dihitung satu kalimat. Jika teks berakhir tanpa tanda akhir tetapi memuat kata, jumlah kalimat bertambah satu. |
| **Baris** | Jumlah karakter `\n`, ditambah satu jika baris terakhir tidak kosong. `\r\n` diperlakukan seperti `\n`. |
| **Karakter** | Semua karakter pada teks termasuk spasi, kecuali `\n` dan `\r`. **[Keputusan]** |
| **Frekuensi kata** | Tidak membedakan huruf besar dan kecil. Kata disimpan dalam huruf kecil. |
| **Kolom grep** | Posisi kemunculan pertama pada baris itu, dihitung mulai dari 1. |

## 8. Contoh keluaran

Contoh ini bersifat ilustrasi dan nilainya dapat dicek dengan tangan.

### 8.1 Mini grep

Isi berkas contoh:

```
Halo dunia.
Dunia itu luas.
Halo lagi, halo semua!
```

| Perintah | Baris cocok | Total kemunculan |
|---|---|---|
| `halo`, peka huruf | baris 3, kolom 12 | 1 |
| `halo`, abaikan huruf besar dan kecil | baris 1 kolom 1, baris 3 kolom 1 | 3 |
| `halo`, abaikan huruf besar dan kecil, tampilkan yang tidak cocok | baris 2 | 3 |
| `matahari` | tidak ada | 0 (status `ST_OK`) |

### 8.2 Analisis teks `"Halo, halo! Apa kabar?"`

```
Karakter : 22
Kata     : 4
Kalimat  : 2
Baris    : 1

Frekuensi huruf
A | ###### 6
B | # 1
H | ## 2
K | # 1
L | ## 2
O | ## 2
P | # 1
R | # 1
(huruf lain: 0, tidak ditampilkan)

Frekuensi kata (terbanyak dulu)
halo   2
apa    1
kabar  1
```

Jumlah huruf adalah 16, sama dengan 4 + 4 + 3 + 5 huruf pada keempat katanya.

### 8.3 Format berkas laporan

```
LAPORAN ANALISIS TEKS
Sumber   : 01-text-toolkit/data/contoh.txt
Panjang  : 22 karakter

[Statistik]
Karakter : 22
Kata     : 4
Kalimat  : 2
Baris    : 1

[Frekuensi huruf]
A 6
B 1
...

[Frekuensi kata]
1. halo 2
2. apa 1
3. kabar 1
```

Laporan hanya memuat bagian yang sudah dihitung. Jika pengguna menulis laporan sebelum menganalisis, program menjalankan analisis terlebih dahulu.

## 9. Desain teknis

### 9.1 Struktur folder

```
01-text-toolkit/
├── text_toolkit.h/.c    run_text_toolkit, test_text_toolkit, menu
├── tt_text.h/.c         muat dan bebaskan teks
├── tt_grep.h/.c
├── tt_analyzer.h/.c
├── tt_cipher.h/.c
├── tt_report.h/.c
├── data/                contoh.txt, kosong.txt, satu_baris.txt, crlf.txt
├── tests/               berkas expected untuk uji berkas ke berkas
└── README.md
```

### 9.2 Struct

```c
typedef struct { char *data; size_t length; char path[UTIL_MAX_PATH]; } TtText;
typedef struct { int line; int col; char *text; } TtMatch;
typedef struct { int chars, words, sentences, lines; int letters[26]; } TtStats;
typedef struct { char word[TT_MAX_WORD]; int count; } TtWordFreq;   /* TT_MAX_WORD = 40 */
```

### 9.3 Fungsi publik

| Modul | Fungsi | Materi RPS yang tercakup |
|---|---|---|
| `tt_text` | `tt_load_text`, `tt_free_text` | File teks, error handling, alokasi dinamis |
| `tt_grep` | `tt_grep`, `tt_free_matches` | String, pointer, array of struct |
| `tt_analyzer` | `tt_compute_stats`, `tt_word_frequency`, `tt_sort_freq_desc`, `tt_sort_words_alpha`, `tt_binary_search_word`, `tt_print_letter_histogram` | Array, struct, Selection Sort, Bubble Sort, Sequential Search, Binary Search |
| `tt_cipher` | `tt_caesar`, `tt_vigenere`, `tt_count_letters`, `tt_reverse_rec`, `tt_palindrome_rec`, `tt_cipher_file` | Rekursi, string, keluaran ganda lewat pointer, file teks |
| `tt_report` | `tt_write_report` | File teks, penanganan error tulis |

Prototipe lengkap mengikuti Bagian 2.1 dokumen sumber. Semua fungsi bernilai kembali `Status` memakai `ST_OK` untuk sukses, dan hasil dikembalikan lewat parameter keluaran.

### 9.4 Algoritma penting

- **Caesar.** Geser dinormalisasi dengan `((shift % 26) + 26) % 26`, karena operator `%` di C menghasilkan nilai negatif untuk geser negatif.
- **Vigenère.** Indeks kunci bertambah hanya setelah karakter huruf diproses. Kunci dipakai berulang, dan huruf besar atau kecil pada kunci tidak memengaruhi hasil.
- **Palindrom rekursif.** Kedua indeks bergerak ke dalam. Karakter non-alfanumerik dilewati dari kiri atau kanan, lalu `tolower` membandingkan sisanya. Kasus dasar terjadi ketika `left >= right`.
- **Frekuensi kata.** Array dimulai dari kapasitas kecil, misalnya 16, dan kapasitas digandakan dengan `realloc` saat penuh. Hasil `realloc` disimpan ke variabel sementara sebelum menimpa pointer lama.
- **Pencarian tanpa membedakan huruf.** Pencocokan berjalan per posisi dengan membandingkan karakter yang sudah diubah ke huruf kecil.
- **Urutan alfabetis untuk Binary Search.** Daftar yang tampil berdasarkan frekuensi tidak dipakai untuk pencarian. Menu mengurutkan salinan daftar secara alfabetis dengan Bubble Sort, lalu mencari di salinan itu. **[Keputusan]**

### 9.5 Manajemen memori

| Alokasi | Pemilik | Fungsi pelepas |
|---|---|---|
| Isi berkas pada `TtText` | Pemanggil `tt_load_text` | `tt_free_text` |
| Salinan baris pada `TtMatch` dan arraynya | Pemanggil `tt_grep` | `tt_free_matches` |
| Array `TtWordFreq` (dan salinannya) | Pemanggil `tt_word_frequency` | `util_free` |
| Buffer hasil cipher dan laporan | Fungsi yang membuatnya | `util_free` |

Aturan: semua alokasi lewat `util_malloc`, `util_calloc`, dan `util_realloc`. Hasil `NULL` dikembalikan sebagai `ST_NO_MEMORY`, semua alokasi yang sudah berhasil dilepas di jalur error, pointer diatur ke `NULL` setelah `free`, dan parameter yang hanya dibaca diberi `const`.

### 9.6 Ketergantungan pada pustaka bersama

Text Toolkit memakai `common/common.h` (`Status`, `UTIL_MAX_LINE`, `UTIL_MAX_PATH`) dan fungsi `util_read_line`, `util_read_int`, `util_trim`, `util_to_lower`, `util_stricmp`, `util_read_file`, `util_write_file`, `util_malloc` sampai `util_free`, `util_alloc_balance`, `util_fail_alloc_after`, serta fungsi *test harness* (`util_test_begin`, `util_check`, `util_check_double`, `util_test_end`). Modul ini tidak memanggil `scanf` secara langsung.

## 10. Kebutuhan nonfungsional

| Aspek | Kebutuhan |
|---|---|
| Bahasa dan kompilator | C99, kompilasi dengan `-std=c99 -Wall -Wextra -Wpedantic -g`, nol *warning*, tautan `-lm` |
| Portabilitas | Dapat dikompilasi di Windows (MinGW-w64 gcc) dan Linux, tanpa fungsi khusus POSIX atau Windows |
| Gaya kode | `snake_case`, fungsi di bawah 50 baris, satu file `.c` di bawah 400 baris, tanpa `goto`, tanpa variabel global |
| Dokumentasi kode | Komentar bahasa Indonesia di atas tiap fungsi: tujuan, parameter, nilai kembali, dan siapa yang melepas memori |
| Ketahanan | Input yang lebih panjang dari buffer dipotong tanpa merusak memori, dan tidak ada *loop* tak berujung |
| Memori | Tidak ada kebocoran, dibuktikan oleh `util_alloc_balance()`, lalu oleh `valgrind` atau `-fsanitize=address` di Linux atau WSL |
| Berkas | Hanya berkas teks. Path relatif terhadap folder root, misalnya `01-text-toolkit/data/contoh.txt` |
| Performa | Berkas sampai beberapa ratus KB diproses dalam waktu yang terasa seketika di laptop biasa |

## 11. Penanganan error

| Situasi | Status | Perilaku |
|---|---|---|
| Berkas tidak ditemukan | `ST_FILE_NOT_FOUND` | Pesan, kembali ke menu |
| Berkas kosong | `ST_FILE_EMPTY` | Pesan, kembali ke menu |
| Gagal menulis laporan atau hasil cipher | `ST_FILE_WRITE` | Pesan, berkas sumber tidak berubah |
| Kunci Vigenère kosong atau memuat angka dan simbol | `ST_BAD_INPUT` | Pesan, minta kunci lagi |
| Alokasi gagal | `ST_NO_MEMORY` | Semua alokasi sebelumnya dilepas, pesan, kembali ke menu |
| Pilihan menu bukan angka atau di luar rentang | (ditangani `util_read_int`) | Ulangi sampai valid |
| Pola grep kosong | `ST_BAD_INPUT` | Pesan, minta pola lagi |

Pesan dihasilkan oleh `util_status_message`, sehingga bunyinya seragam di seluruh menu.

## 12. Data uji dan rencana pengujian

### 12.1 Berkas contoh (`data/`)

| Berkas | Isi | Tujuan |
|---|---|---|
| `contoh.txt` | Teks beberapa paragraf yang realistis, memuat huruf besar, kecil, angka, dan tanda baca | Demo dan uji umum |
| `kosong.txt` | 0 byte | Uji `ST_FILE_EMPTY` |
| `satu_baris.txt` | Satu baris tanpa `\n` di akhir | Uji penghitungan baris dan kalimat tanpa tanda akhir |
| `crlf.txt` | Isi sama dengan `contoh.txt` berakhir baris `\r\n` | Uji TT-04 |

Folder `tests/` memuat berkas *expected* untuk uji berkas ke berkas, yaitu hasil Caesar dan Vigenère dari teks pendek yang sudah dihitung dengan tangan.

### 12.2 Kasus uji minimum

Seluruh baris berikut harus menjadi pemeriksaan `util_check` di `test_text_toolkit()`. Jika program menghasilkan nilai berbeda, periksa kodenya lebih dulu sebelum mengubah nilai harapan.

| ID uji | Uji | Hasil yang diharapkan | Kebutuhan |
|---|---|---|---|
| U01 | Statistik `"Halo, halo! Apa kabar?"` | 22 karakter, 4 kata, 2 kalimat, 1 baris | TT-20 |
| U02 | Frekuensi kata pada teks yang sama | `halo`=2, `apa`=1, `kabar`=1, 3 kata unik | TT-23 |
| U03 | `tt_caesar("Hello, World!", 3)` | `"Khoor, Zruog!"` | TT-40 |
| U04 | Caesar geser 26 dan geser 29 | Sama dengan teks asli, dan sama dengan geser 3 | TT-41 |
| U05 | Enkripsi lalu dekripsi (Caesar dan Vigenère) | Kembali ke teks asli | TT-47 |
| U06 | Vigenère `"ATTACKATDAWN"`, kunci `"LEMON"` | `"LXFOPVEFRNHR"` | TT-43 |
| U07 | Kunci Vigenère kosong atau berisi angka | `ST_BAD_INPUT` | TT-44 |
| U08 | Palindrom `"A man, a plan, a canal: Panama"` dan `"Katak"` | 1 | TT-51 |
| U09 | Palindrom `"Halo"` | 0 | TT-51 |
| U10 | `tt_reverse_rec("abc")` | `"cba"` | TT-50 |
| U11 | Grep dengan dan tanpa `ignore_case` | Jumlah sesuai hitungan manual (Bagian 8.1) | TT-10, TT-11 |
| U12 | Grep kata yang tidak ditemukan | `match_count` = 0, status `ST_OK` | TT-14 |
| U13 | Berkas tidak ada, berkas kosong | `ST_FILE_NOT_FOUND`, `ST_FILE_EMPTY` | TT-02, TT-03 |
| U14 | Berkas `\r\n` | Jumlah baris sama dengan versi `\n` | TT-04 |
| U15 | Binary Search kata yang ada dan yang tidak | Indeks yang benar, dan `-1` | TT-30 |
| U16 | Alokasi gagal lewat `util_fail_alloc_after` | `ST_NO_MEMORY` dan `util_alloc_balance() == 0` | Bagian 9.5 |
| U17 | Akhir test | `util_alloc_balance() == 0` | S4 |

Uji tambahan yang dianjurkan: grep dengan `invert`, string kosong pada palindrom, kata melebihi 40 karakter, geser negatif pada Caesar, brute force menghasilkan 26 baris, dan penulisan laporan ke folder yang tidak ada.

### 12.3 Uji menu interaktif

Alur menu diuji dengan input terpipa agar tidak perlu mengetik, misalnya `printf "1\n01-text-toolkit/data/contoh.txt\n3\n0\n0\n" | ./suite`. Tiga skenario minimum:

1. Muat berkas, analisis, tulis laporan, keluar.
2. Pilih menu 2 sebelum memuat berkas, lalu perhatikan bahwa program meminta berkas dimuat dulu.
3. Masukkan huruf pada pilihan menu dan angka di luar rentang.

### 12.4 Kriteria selesai

- [ ] `make` menghasilkan satu *executable* tanpa *warning*, dan `build.bat` setara.
- [ ] Seluruh kebutuhan P0 lulus kriteria penerimaannya.
- [ ] Seluruh baris U01 sampai U17 ada di kode dan lulus.
- [ ] `./suite --test` menghasilkan kode keluar 0.
- [ ] Input salah dan berkas bermasalah tidak pernah membuat program crash.
- [ ] Folder `data/` lengkap dan `README.md` projek terisi (tujuan, cara menjalankan, format berkas, daftar fungsi publik, daftar uji, keputusan desain).
- [ ] Tidak ada *stub* atau `TODO` tersisa.

## 13. Keterlacakan terhadap RPS

| Sub-CPMK | Materi | Terpenuhi lewat |
|---|---|---|
| 1 | I/O, tipe data, operator, boolean | Menu, analyzer, `util_read_*` |
| 2 | `if`, `switch`, perulangan, *nested loop*, `break`, `continue` | Menu `switch`, validasi input, pemindaian teks |
| 3 | Fungsi, parameter, return, rekursi, modular | `tt_reverse_rec`, `tt_palindrome_rec`, pemisahan lima modul |
| 4 | Array, string, pointer, struct, array of struct | `TtWordFreq[]`, `TtMatch[]`, `TtStats` |
| 5 | Bubble Sort, Selection Sort, Sequential Search, Binary Search | `tt_sort_words_alpha`, `tt_sort_freq_desc`, pengecekan kata ganda, `tt_binary_search_word` |
| 6 | File teks, penanganan error, *test case* | `tt_load_text`, `tt_cipher_file`, `tt_write_report`, `test_text_toolkit()` |
| Bahan kajian 14 | Manajemen memori | `util_malloc` sampai `util_free`, pelacak alokasi |

## 14. Rencana kerja dan pembagian tugas

### 14.1 Pembagian lima anggota

| No. | Peran | Tanggung jawab | Berkas | Bergantung pada |
|---|---|---|---|---|
| 1 | Fondasi dan integrasi menu | Menu, `util`, pemuat teks, *test harness* | `text_toolkit.c`, `tt_text.c`, `common/` | Tidak ada |
| 2 | Mini grep | `tt_grep`, `tt_free_matches`, pencarian tanpa membedakan huruf | `tt_grep.c` | `tt_load_text` |
| 3 | Analyzer | Statistik, frekuensi huruf dan kata, Selection Sort dan Bubble Sort | `tt_analyzer.c` | `tt_load_text` |
| 4 | Cipher dan utilitas | Caesar, Vigenère, brute force, palindrom, pembalik string | `tt_cipher.c` | `util` |
| 5 | Laporan, pencarian kata, data uji | `tt_write_report`, Binary Search, `contoh.txt` dan data lain, integrasi akhir | `tt_report.c` | Hasil modul 3 |

Anggota 1 memegang `common/` karena semua modul lain bergantung padanya. Anggota 5 memegang integrasi akhir karena beban modul laporannya paling ringan.

### 14.2 Tahap pengerjaan

Tahap berikut disusun secara berurutan dan durasinya disesuaikan dengan tenggat dari dosen.

| Tahap | Isi | Keluaran |
|---|---|---|
| T1 Fondasi | `common/`, kerangka menu, `tt_load_text`, build bersih | Kerangka dapat dikompilasi, `./suite` menampilkan menu |
| T2 Modul paralel | Anggota 2 sampai 5 mengerjakan modul masing-masing dengan `test_` lokal | Tiap modul lulus uji sendiri |
| T3 Integrasi | Menu memanggil semua modul, data contoh lengkap, `test_text_toolkit()` memuat U01 sampai U17 | `./suite --test` hijau |
| T4 Verifikasi | Pemeriksaan kebocoran (`valgrind` atau ASan), uji menu terpipa, README projek | Semua butir Bagian 12.4 tercentang |
| T5 Latihan demo | Tiap anggota menjelaskan bagian yang dipegangnya dan menjawab pertanyaan silang | Naskah demo dan daftar pertanyaan antisipasi |

## 15. Keputusan dan pertanyaan terbuka

### 15.1 Keputusan yang diambil di dokumen ini

| Keputusan | Pilihan | Alasan |
|---|---|---|
| Perlakuan apostrof | Pemisah kata | Aturan paling sederhana dan konsisten dengan definisi kata sebagai deretan huruf atau angka |
| Penghitungan karakter | Semua karakter kecuali `\n` dan `\r` | Hasil `"Halo, halo! Apa kabar?"` tetap 22 dan tidak bergantung pada akhir baris |
| Nilai parameter `mode` pada `tt_cipher_file` | 0 = Caesar enkripsi, 1 = Caesar dekripsi, 2 = Vigenère enkripsi, 3 = Vigenère dekripsi | Sumber tidak mendefinisikan nilainya |
| Daftar untuk Binary Search | Salinan terurut alfabetis | Daftar tampilan tetap terurut menurut frekuensi |
| Rentang brute force | Geser 0 sampai 25 | Sumber meminta 26 kemungkinan |

Tiap keputusan dicatat juga di `README.md` projek.

### 15.2 Perlu dikonfirmasi ke dosen atau asisten

1. Apakah `malloc`, `realloc`, dan `free` boleh dipakai. Bahan kajian nomor 14 menyebut manajemen memori, tetapi rencana mingguan tidak memuatnya. Jika tidak boleh, array dinamis diganti array berukuran tetap dan pelacak alokasi dihapus.
2. Apakah kode boleh dipecah ke beberapa file `.c` dan `.h`. Jika tidak boleh, file digabung per projek dengan pemisah bagian yang rapi.

## 16. Risiko dan mitigasi

| Risiko | Dampak | Mitigasi |
|---|---|---|
| Kesan "wow" rendah | Nilai demonstrasi biasa saja | Fokus pada kerapian, kelengkapan uji, dan penanganan error; siapkan demo kasus tepi |
| Tafsir definisi kata, kalimat, dan baris berbeda antaranggota | Hasil analyzer, grep, dan laporan tidak cocok | Definisi di Bagian 7 menjadi acuan tunggal, diuji lewat U01 dan U14 |
| Modulo negatif pada Caesar | Huruf keluar dari rentang alfabet | Normalisasi `((shift % 26) + 26) % 26`, uji geser negatif |
| `strstr` peka huruf besar dan kecil | Opsi abaikan huruf tidak bekerja | Pencarian buatan sendiri (TT-11) |
| Kata melebihi `TT_MAX_WORD` | Tulis di luar buffer | Potong aman (TT-27) |
| Lupa `+ 1` untuk `'\0'` saat alokasi | Kerusakan memori dan crash acak | Tinjauan silang antaranggota dan pemeriksaan ASan atau `valgrind` |
| Pencarian Binary Search pada daftar yang belum terurut | Hasil salah tanpa pesan | TT-31, menu mengurutkan salinan sebelum mencari |
| Anggota tidak paham bagian lain saat tanya jawab | Nilai individu turun | Latihan demo silang pada tahap T5 |

## Lampiran A. Ringkasan struktur menu utama

```
=== UAS SUITE: DASAR PEMROGRAMAN (C) ===
1. Text Toolkit
2. Kalkulator Ekspresi + Matrix
3. Data Analyzer
4. Maze Lab
5. Jalankan semua self-test
0. Keluar
```

Pilihan 1 memanggil `run_text_toolkit()`. Pilihan 5 menjalankan `test_text_toolkit()` bersama ketiga uji projek lain, dan argumen `--test` menjalankan semuanya tanpa interaksi.
