# Prompt: Bangun Text Toolkit (projek 01) di folder `01-text-toolkit/`

## 1. Peran dan tujuan

Kamu adalah engineer C senior. Bangun projek **Text Toolkit** secara utuh: lima modul, data uji, *test harness*, skrip build, dan README, semuanya di folder `01-text-toolkit/`. Satu orang (kamu) mengerjakan seluruh peran. Pembagian lima anggota di PRD hanya konteks tim, tetapi batas modul dan berkas yang dinamai PRD tetap dipertahankan.

Kerjakan semua kebutuhan P0, lalu semua P1. Hasil akhir tidak boleh memuat *stub*, `TODO`, atau fungsi kosong.

## 2. Sumber kebenaran

Baca `PRD_01_Text_Toolkit.md` sampai habis sebelum menulis kode. PRD itu menetapkan ID kebutuhan (TT-01 dst.), definisi baku (Bagian 7), contoh keluaran (Bagian 8), kasus uji U01 sampai U17 (Bagian 12.2), dan kriteria selesai (Bagian 12.4).

- Soal **perilaku fitur**, PRD yang berlaku.
- Soal **struktur folder, build, dan isi `common/`**, prompt ini yang berlaku, karena PRD tidak menetapkannya.
- Jangan bertanya kecuali benar-benar terblokir. Untuk hal yang belum ditetapkan, ambil keputusan yang masuk akal dan catat di `README.md` pada bagian "Keputusan desain".

## 3. Lokasi dan struktur

Direktori kerja saat ini dianggap *root* proyek (`uas-suite/`). Program dijalankan dari root, sehingga path data berbentuk `01-text-toolkit/data/contoh.txt`.

```
./
├── common/
│   ├── common.h
│   └── common.c
└── 01-text-toolkit/
    ├── text_toolkit.h / text_toolkit.c    run_text_toolkit, test_text_toolkit, menu
    ├── tt_text.h / tt_text.c
    ├── tt_grep.h / tt_grep.c
    ├── tt_analyzer.h / tt_analyzer.c
    ├── tt_cipher.h / tt_cipher.c
    ├── tt_report.h / tt_report.c
    ├── tt_main.c                          main() khusus standalone (bukan bagian suite)
    ├── data/                              contoh.txt, kosong.txt, satu_baris.txt, crlf.txt
    ├── tests/                             berkas expected untuk uji berkas ke berkas
    ├── README.md
    ├── Makefile
    ├── build.bat
    └── .gitattributes                     tandai data/crlf.txt sebagai -text agar CRLF tidak berubah
```

- `tt_main.c` hanya berisi `main`: argumen `--test` memanggil `test_text_toolkit()` dan mengembalikan hasilnya sebagai kode keluar, tanpa argumen memanggil `run_text_toolkit()`, argumen lain mencetak petunjuk pemakaian. Nanti saat digabung ke `suite`, berkas ini tidak ikut dikompilasi.
- `Makefile` di dalam `01-text-toolkit/` menghasilkan executable `../text_toolkit` (di root). Target: `all`, `test` (menjalankan `cd .. && ./text_toolkit --test`), `asan` (build `-fsanitize=address,undefined` ke `../text_toolkit_asan`), `clean`. `build.bat` setara untuk MinGW-w64 gcc (menghasilkan `..\text_toolkit.exe`).
- Flag wajib: `-std=c99 -Wall -Wextra -Wpedantic -g`, tautan `-lm`. Nol *warning*.
- Include antarmodul memakai path relatif, misalnya `#include "../common/common.h"`, supaya tidak bergantung pada `-I`.

## 4. Pustaka bersama `common/`

Projek lain memakai `common/` yang sama. **Jika `common/common.h` dan `common.c` sudah ada, pakai apa adanya**: boleh menambah fungsi yang kurang, tetapi jangan mengubah tanda tangan yang sudah ada. Jika belum ada, buat sesuai kontrak berikut (PRD hanya menyebut nama fungsinya).

```c
typedef enum {
    ST_OK = 0, ST_FILE_NOT_FOUND, ST_FILE_EMPTY, ST_FILE_WRITE,
    ST_BAD_INPUT, ST_BAD_FORMAT, ST_OUT_OF_RANGE, ST_NO_MEMORY
} Status;

#define UTIL_MAX_LINE 256
#define UTIL_MAX_PATH 260

const char *util_status_message(Status s);
Status util_read_line(const char *prompt, char *buf, size_t size);
Status util_read_int(const char *prompt, int min, int max, int *out);
void   util_trim(char *s);
void   util_to_lower(char *s);
int    util_stricmp(const char *a, const char *b);
Status util_read_file(const char *path, char **out, size_t *len);
Status util_write_file(const char *path, const char *data, size_t len);
void  *util_malloc(size_t n);
void  *util_calloc(size_t n, size_t size);
void  *util_realloc(void *p, size_t n);
void   util_free(void *p);
int    util_alloc_balance(void);
void   util_fail_alloc_after(int k);
void   util_test_begin(const char *name);
void   util_check(int cond, const char *desc);
void   util_check_double(double got, double want, double eps, const char *desc);
int    util_test_end(void);
```

Perilaku yang harus dipenuhi:

- `util_status_message`: pesan bahasa Indonesia yang seragam untuk tiap `Status`, tidak pernah `NULL`.
- `util_read_line`: menampilkan `prompt` (boleh `NULL`), membaca satu baris dengan `fgets`, membuang `\n` dan `\r`. Baris yang lebih panjang dari buffer dipotong dan sisanya dibuang sampai akhir baris, sehingga tidak tertinggal di `stdin`. EOF menghasilkan `ST_BAD_INPUT` dan buffer kosong.
- `util_read_int`: mengulang sampai pengguna memberi bilangan bulat utuh dalam `[min, max]` (validasi dengan `strtol` dan `endptr`, spasi di tepi boleh, `12abc` ditolak). Menampilkan pesan saat salah. EOF menghasilkan `ST_BAD_INPUT`, tidak boleh berputar selamanya, dan pemanggil menganggap status bukan `ST_OK` sebagai tanda mengakhiri input.
- `util_read_file`: mode `"rb"`, mengembalikan buffer `util_malloc` yang diakhiri `'\0'` (`*len` = jumlah byte tanpa NUL). Berkas tidak ada: `ST_FILE_NOT_FOUND`. Berkas 0 byte: `ST_FILE_EMPTY` tanpa alokasi tersisa. Alokasi gagal: `ST_NO_MEMORY`.
- `util_write_file`: mode `"wb"`, gagal membuka atau menulis menghasilkan `ST_FILE_WRITE`.
- Pelacak alokasi: `util_alloc_balance()` = jumlah alokasi berhasil dikurangi jumlah pelepasan. `util_free(NULL)` aman dan tidak dihitung. `util_realloc(NULL, n)` setara alokasi. Jika `util_realloc` gagal, kembalikan `NULL` dan biarkan blok lama utuh. `util_fail_alloc_after(k)`: `k` alokasi berikutnya berhasil, semua alokasi sesudahnya mengembalikan `NULL` sampai dipanggil lagi dengan nilai negatif (`-1` mematikan injeksi).
- *Test harness*: `util_test_begin` mencetak judul dan mereset hitungan. `util_check` mencetak `[PASS]` atau `[FAIL]` beserta deskripsi dan menghitungnya. `util_check_double` lulus jika selisih mutlak tidak lebih dari `eps`. `util_test_end` mencetak ringkasan dan mengembalikan jumlah yang gagal (0 berarti semua lulus).
- Satu-satunya variabel file-scope yang diizinkan ada di `common.c` (pelacak alokasi dan hitungan harness), bertipe `static`. Kode projek tidak boleh punya variabel global.

## 5. Konvensi kode

- `snake_case`, fungsi di bawah 50 baris, tiap berkas `.c` di bawah 400 baris, tanpa `goto`, tanpa variabel global, parameter yang hanya dibaca diberi `const`.
- Semua alokasi lewat `util_malloc`/`util_calloc`/`util_realloc`. Hasil `NULL` dikembalikan sebagai `ST_NO_MEMORY`, semua alokasi yang sudah berhasil dilepas di jalur error, hasil `realloc` ditampung di variabel sementara dulu, pointer diatur `NULL` setelah dilepas.
- Hanya C99 murni dan portabel (Windows MinGW dan Linux). Jangan pakai `strdup`, `strcasecmp`, `system()`, atau fungsi POSIX/Windows. Salin string manual lewat `util_malloc`. Cetak `size_t` dengan `%lu` dan *cast* ke `unsigned long`. Hapus berkas sementara dengan `remove()`.
- Tidak ada `scanf` langsung. Semua input lewat `util_read_*`.
- **Komentar:** tiap fungsi diberi satu blok komentar bahasa Indonesia yang singkat berisi tujuan, parameter, nilai kembali, dan siapa yang melepas memori (diwajibkan PRD). Di luar itu, komentar hanya untuk logika yang tidak jelas dengan sendirinya. **Dilarang** komentar dekoratif atau pemisah seksi (misalnya `/* ==== */`, `// ----`, `/* ---- Bagian X ---- */`), juga kode yang dikomentari. Berlaku pula untuk Makefile dan `build.bat`.

## 6. Tipe dan prototipe

Tipe pada PRD Bagian 9.2 dipakai persis (`TtText`, `TtMatch`, `TtStats`, `TtWordFreq`, `TT_MAX_WORD = 40`). Nama fungsi publik pada Bagian 9.3 dipakai persis. Dokumen sumber yang memuat prototipe lengkap tidak tersedia, jadi gunakan rancangan berikut (semua mengembalikan `Status` kecuali yang dinyatakan lain, hasil lewat parameter keluaran). Boleh disesuaikan jika ada alasan kuat, asal konsisten dan dicatat di README.

```c
Status tt_load_text(const char *path, TtText *out);
void   tt_free_text(TtText *t);
Status tt_grep(const TtText *t, const char *pattern, int ignore_case, int invert,
               TtMatch **matches, int *match_count, int *total_occurrences);
void   tt_free_matches(TtMatch *matches, int match_count);
Status tt_compute_stats(const char *text, TtStats *out);
Status tt_word_frequency(const char *text, TtWordFreq **out, int *count);
void   tt_sort_freq_desc(TtWordFreq *a, int n);
void   tt_sort_words_alpha(TtWordFreq *a, int n);
int    tt_binary_search_word(const TtWordFreq *a, int n, const char *word);
void   tt_print_letter_histogram(const TtStats *s);
Status tt_caesar(char *text, int shift);
Status tt_vigenere(char *text, const char *key, int decrypt);
void   tt_caesar_brute(const char *text);
void   tt_count_letters(const char *s, int *upper, int *lower, int *other);
void   tt_reverse_rec(char *s);
int    tt_palindrome_rec(const char *s);
Status tt_cipher_file(const char *in_path, const char *out_path, int mode,
                      int shift, const char *key);
Status tt_write_report(const char *path, const TtText *t, const TtStats *s,
                       const TtWordFreq *words, int word_count);
```

`tt_reverse_rec` dan `tt_palindrome_rec` publik berparameter satu string supaya cocok dengan U08 sampai U10, tetapi rekursinya sungguhan lewat fungsi pembantu `static` berindeks `left` dan `right`.

## 7. Poin yang sering salah (patuhi)

**Definisi baku (PRD Bagian 7) harus identik di semua modul.** Gunakan satu pemindai kata bersama agar `tt_compute_stats`, `tt_word_frequency`, dan grep tidak berbeda tafsir. Kata adalah deretan huruf atau angka, apostrof adalah pemisah. Karakter dihitung kecuali `\n` dan `\r`. Satu atau lebih `.`, `!`, `?` berurutan dihitung satu kalimat, dan teks yang berakhir tanpa tanda akhir tetapi memuat kata menambah satu kalimat. Baris adalah jumlah `\n` ditambah satu jika baris terakhir tidak kosong, dengan `\r\n` setara `\n`.

**Mini grep**
- Kolom mulai dari 1, hanya kemunculan pertama per baris yang dilaporkan, tetapi `total_occurrences` menghitung semua kemunculan, termasuk beberapa pada satu baris. Pencarian tidak memakai `strstr`/`strcasestr`; buat pencocok sendiri yang membandingkan karakter hasil `tolower` per posisi (TT-11).
- Pada mode `invert`, `matches` berisi tepat baris yang tidak memuat pola, sedangkan `total_occurrences` tetap menghitung kemunculan pada baris yang cocok (lihat Bagian 8.1: baris 2 tampil, total 3). Mode hitung saja (TT-12) mencetak angka `total_occurrences` saja.
- Pola kosong: `ST_BAD_INPUT`. Tidak ditemukan: `ST_OK`, `match_count = 0`. Teks baris pada `TtMatch` tidak memuat `\r` atau `\n`.

**Analyzer**
- Kata disimpan huruf kecil. Kata di atas `TT_MAX_WORD`: simpan 39 karakter pertama dan buang sisa deretan huruf/angka yang sama (jangan memulai kata baru dari sisanya). Tidak boleh menulis di luar buffer.
- Array frekuensi mulai dari kapasitas 16 dan digandakan lewat `realloc`; pengecekan kata yang sudah ada memakai Sequential Search; urutan frekuensi memakai Selection Sort (jumlah menurun, lalu alfabetis jika sama); urutan alfabetis memakai Bubble Sort. Jangan memakai `qsort`.
- Histogram: satu `#` per hitungan, huruf berfrekuensi 0 tidak ditampilkan, format `A | ###### 6`. Jika hitungan terbesar melebihi 50, skala batang proporsional (terbesar = 50 `#`, nilai yang lebih dari 0 minimal satu `#`) dan angka aslinya tetap dicetak. Catat sebagai keputusan desain.

**Binary Search.** Menu mengurutkan *salinan* daftar secara alfabetis dengan Bubble Sort sebelum mencari, dan daftar tampilan tetap terurut menurut frekuensi. Kata yang dicari diubah ke huruf kecil dulu. Kembalikan indeks atau `-1`.

**Cipher**
- Caesar: normalisasi `((shift % 26) + 26) % 26`, huruf besar berbasis `'A'`, huruf kecil berbasis `'a'`, karakter lain utuh. Brute force mencetak tepat 26 baris untuk geser 0 sampai 25.
- Vigenère: kunci divalidasi sebelum teks disentuh (kosong atau memuat selain huruf berarti `ST_BAD_INPUT` dan teks tidak berubah). Indeks kunci maju hanya pada karakter huruf, kapitalisasi teks dipertahankan, kunci tidak peka huruf besar/kecil. Uji acuan: `"ATTACKATDAWN"` dengan `"LEMON"` menjadi `"LXFOPVEFRNHR"`.
- `tt_cipher_file` memakai `mode`: 0 = Caesar enkripsi, 1 = Caesar dekripsi, 2 = Vigenère enkripsi, 3 = Vigenère dekripsi. Berkas sumber tidak pernah diubah, kegagalan menulis menghasilkan `ST_FILE_WRITE`.

**Laporan.** Buat satu fungsi yang menyusun seluruh teks laporan ke buffer, lalu dipakai untuk layar dan berkas, sehingga isi berkas sama persis dengan keluaran layar (TT-60). Formatnya Bagian 8.3. Jika belum ada hasil analisis saat menulis laporan, jalankan analisis lebih dulu. Folder tujuan yang tidak ada menghasilkan `ST_FILE_WRITE` dan program tidak boleh crash.

## 8. Menu

Ikuti Bagian 5 PRD. Seluruh status (teks termuat, statistik, daftar kata) disimpan sebagai variabel lokal di `run_text_toolkit` dan dilewatkan lewat pointer.

- Menu 2, 3, 6 tanpa teks termuat: tampilkan pesan agar memilih menu 1, lalu kembali ke menu.
- Menu 3 memakai submenu: statistik, frekuensi huruf (histogram), frekuensi kata, cari kata (Binary Search), kembali. Catat susunan submenu ini sebagai keputusan desain.
- Menu 4 (cipher) menerima teks ketikan: Caesar enkripsi/dekripsi, Vigenère enkripsi/dekripsi (kunci diminta ulang sampai valid), brute force, dan proses berkas ke berkas.
- Menu 5: cek palindrom dan pembalik string pada teks ketikan.
- Memuat berkas baru melepaskan teks lama lebih dulu. Keluar dengan pilihan 0 melepaskan seluruh memori. Jika `util_read_int` mengembalikan status bukan `ST_OK` (EOF), menu berakhir dengan bersih.

## 9. Data uji dan pengujian

**`data/`:**
- `contoh.txt`: beberapa paragraf bahasa Indonesia yang realistis, memuat huruf besar, kecil, angka, tanda baca, dan beberapa kalimat dengan `!` dan `?`.
- `kosong.txt`: benar-benar 0 byte (buat dengan `: > berkas` atau setara, lalu periksa dengan `wc -c`).
- `satu_baris.txt`: satu baris tanpa `\n` di akhir (buat dengan `printf`).
- `crlf.txt`: isi sama dengan `contoh.txt` tetapi akhir baris `\r\n`. Periksa dengan `file` atau `od -c` bahwa CRLF benar-benar ada.

**`tests/`:** berkas masukan pendek dan berkas *expected* untuk Caesar (misalnya geser 3) serta Vigenère (kunci `LEMON`). Hitung nilai *expected* secara terpisah, misalnya dengan skrip Python satu baris atau tangan, **jangan** menyalin keluaran program C (itu uji yang melingkar).

**`test_text_toolkit()`** memuat seluruh U01 sampai U17 sebagai `util_check`, ditambah uji tambahan yang dianjurkan PRD (`invert`, palindrom string kosong, kata melebihi 40 karakter, geser negatif, brute force 26 baris, laporan ke folder yang tidak ada). Fungsi ini mengembalikan 0 jika semua lulus, memanggil `util_fail_alloc_after` untuk uji U16 lalu mengembalikannya ke `-1`, dan ditutup dengan pemeriksaan `util_alloc_balance() == 0`. Berkas sementara hasil uji (laporan, hasil cipher) ditulis ke `01-text-toolkit/tests/` dan dihapus dengan `remove()` di akhir.

Jika sebuah uji gagal, periksa kode lebih dulu. Ubah nilai harapan hanya jika hitungan independen membuktikan nilai itu salah.

## 10. Urutan kerja

1. Baca PRD sampai habis.
2. **Fondasi:** `common/`, kerangka menu, `tt_text`, Makefile dan `build.bat`. Pastikan bersih tanpa *warning* dan `./text_toolkit` menampilkan menu.
3. **Modul:** `tt_grep`, `tt_analyzer`, `tt_cipher`, `tt_report`, masing-masing disertai ujinya di `test_text_toolkit()`.
4. **Integrasi:** hubungkan semua modul ke menu, lengkapi `data/` dan `tests/`, pastikan U01 sampai U17 hijau.
5. **Verifikasi (jalankan benar-benar, jangan hanya mengklaim):**
   - `make` dengan tambahan `-Werror` berhasil tanpa satu pun *warning*.
   - `./text_toolkit --test` selesai dengan kode keluar 0 dan `util_alloc_balance() == 0`.
   - Build ASan (`make asan`) lalu jalankan `--test` dan tiga skenario menu terpipa di Bagian 12.3 PRD (sesuaikan masukannya untuk executable standalone). Tidak boleh ada galat ASan atau kebocoran.
   - Menu tidak crash pada EOF di tengah masukan, berkas hilang, berkas kosong, pilihan menu berupa huruf, dan angka di luar rentang.
   - Cek ukuran: tidak ada fungsi 50 baris atau lebih, tidak ada `.c` 400 baris atau lebih, tidak ada `goto`, tidak ada variabel global di kode projek, tidak ada komentar dekoratif.
6. **README.md** projek: tujuan, cara build dan menjalankan (termasuk dari root `suite` kelak), format berkas, daftar fungsi publik, daftar uji (U01 sampai U17 plus tambahan), keputusan desain (semua yang bertanda **[Keputusan]** di PRD ditambah keputusanmu sendiri), dan dua pertanyaan terbuka untuk dosen di Bagian 15.2.

## 11. Laporan akhir

Setelah semua selesai, tampilkan ringkasan singkat: pohon berkas yang dibuat, keluaran `--test` (jumlah lulus/gagal), hasil ASan, kasus tepi yang dicoba, serta daftar keputusan desain atau penyimpangan dari prompt ini beserta alasannya.
