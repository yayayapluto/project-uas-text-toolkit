#ifndef TT_CIPHER_H
#define TT_CIPHER_H

#include "../common/common.h"

/* Sandi Caesar in-place; geser dinormalisasi ((shift % 26) + 26) % 26,
 * karakter selain huruf tidak berubah. Kembali ST_BAD_INPUT saat text
 * NULL. */
Status tt_caesar(char *text, int shift);

/* Sandi Vigenere in-place. Kunci divalidasi lebih dulu (hanya huruf,
 * tidak boleh kosong) -> ST_BAD_INPUT tanpa mengubah teks. Indeks kunci
 * maju hanya pada karakter huruf; kapitalisasi teks dipertahankan;
 * decrypt = 1 mengurangi geseran kunci. */
Status tt_vigenere(char *text, const char *key, int decrypt);

/* Proses berkas ke berkas; berkas sumber tidak diubah. mode:
 * 0 = Caesar enkripsi, 1 = Caesar dekripsi, 2 = Vigenere enkripsi,
 * 3 = Vigenere dekripsi. Kembali ST_FILE_NOT_FOUND, ST_FILE_EMPTY,
 * ST_BAD_INPUT, ST_FILE_WRITE, atau ST_NO_MEMORY. */
Status tt_cipher_file(const char *in_path, const char *out_path, int mode,
                      int shift, const char *key);

/* Brute force Caesar: mencetak 26 baris (geser 0 sampai 25), satu baris
 * per kemungkinan. */
void tt_caesar_brute(const char *text);

/* Menghitung huruf kapital, huruf kecil, dan karakter lain dengan
 * keluaran ganda lewat pointer. */
void tt_count_letters(const char *s, int *upper, int *lower, int *other);

/* Pembalik string rekursif ke buffer out (kapasitas minimal strlen+1). */
void tt_reverse_rec(const char *s, char *out);

/* Cek palindrom rekursif: melewati karakter non-alfanumerik dan abaikan
 * huruf besar/kecil; string kosong dianggap palindrom (kembali 1). */
int tt_palindrome_rec(const char *s);

#endif
