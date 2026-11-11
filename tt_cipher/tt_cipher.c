#include "tt_cipher.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Sandi Caesar: geser huruf dengan basis 'A' atau 'a', non-huruf utuh. */
Status tt_caesar(char *text, int shift) {
    int s = ((shift % 26) + 26) % 26;
    if (text == NULL) return ST_BAD_INPUT;
    for (; *text != '\0'; text++) {
        char c = *text;
        if (c >= 'A' && c <= 'Z') *text = (char)('A' + (c - 'A' + s) % 26);
        else if (c >= 'a' && c <= 'z') *text = (char)('a' + (c - 'a' + s) % 26);
    }
    return ST_OK;
}

/* Kunci valid hanya jika tidak kosong dan seluruhnya huruf. */
static int is_key_valid(const char *key) {
    const char *k;
    if (key == NULL || key[0] == '\0') return 0;
    for (k = key; *k != '\0'; k++) {
        if (!isalpha((unsigned char)*k)) return 0;
    }
    return 1;
}

/* Sandi Vigenere: geseran dari huruf kunci, indeks kunci maju hanya
 * pada karakter huruf teks; dekripsi mengurangi geseran. */
Status tt_vigenere(char *text, const char *key, int decrypt) {
    size_t ki = 0, klen;
    if (text == NULL) return ST_BAD_INPUT;
    if (!is_key_valid(key)) return ST_BAD_INPUT;
    klen = strlen(key);
    for (; *text != '\0'; text++) {
        char c = *text;
        if (isalpha((unsigned char)c)) {
            int kshift = tolower((unsigned char)key[ki % klen]) - 'a';
            if (decrypt) kshift = (26 - kshift) % 26;
            if (c >= 'A' && c <= 'Z') {
                *text = (char)('A' + (c - 'A' + kshift) % 26);
            } else {
                *text = (char)('a' + (c - 'a' + kshift) % 26);
            }
            ki++;
        }
    }
    return ST_OK;
}

/* Memproses isi berkas dengan Caesar atau Vigenere lalu menulis hasil;
 * berkas sumber tetap utuh. */
Status tt_cipher_file(const char *in_path, const char *out_path, int mode,
                      int shift, const char *key) {
    Status s;
    char *data = NULL;
    size_t len = 0;
    if (in_path == NULL || out_path == NULL) return ST_BAD_INPUT;
    if (mode < 0 || mode > 3) return ST_BAD_INPUT;
    if ((mode == 2 || mode == 3) && !is_key_valid(key)) return ST_BAD_INPUT;
    s = util_read_file(in_path, &data, &len);
    if (s != ST_OK) return s;
    if (mode == 0 || mode == 1) {
        s = tt_caesar(data, mode == 0 ? shift : -shift);
    } else {
        s = tt_vigenere(data, key, mode == 3);
    }
    if (s == ST_OK) {
        s = util_write_file(out_path, data, len);
    }
    util_free(data);
    return s;
}

/* Brute force: satu baris per geser 0..25 dengan salinan teks. */
void tt_caesar_brute(const char *text) {
    char *buf;
    size_t len;
    int shift;
    if (text == NULL) return;
    len = strlen(text);
    buf = (char *)util_malloc(len + 1);
    if (buf == NULL) {
        printf("Memori tidak cukup untuk brute force.\n");
        return;
    }
    for (shift = 0; shift < 26; shift++) {
        strcpy(buf, text);
        tt_caesar(buf, shift);
        printf("Geser %2d: %s\n", shift, buf);
    }
    util_free(buf);
}

/* Menghitung kapital, kecil, dan lainnya lewat pointer keluaran. */
void tt_count_letters(const char *s, int *upper, int *lower, int *other) {
    int u = 0, l = 0, o = 0;
    for (; s != NULL && *s != '\0'; s++) {
        if (*s >= 'A' && *s <= 'Z') u++;
        else if (*s >= 'a' && *s <= 'z') l++;
        else o++;
    }
    if (upper != NULL) *upper = u;
    if (lower != NULL) *lower = l;
    if (other != NULL) *other = o;
}

/* Pembalik rekursif: indeks kanan bergerak mundur menyalin karakter. */
static void reverse_at(const char *s, char *out, size_t right) {
    if (right == (size_t)-1) return;
    *out = s[right];
    reverse_at(s, out + 1, right - 1);
}

/* Membalik s ke out secara rekursif; out butuh ruang strlen(s)+1. */
void tt_reverse_rec(const char *s, char *out) {
    size_t len;
    if (s == NULL || out == NULL) return;
    len = strlen(s);
    reverse_at(s, out, len - 1);
    out[len] = '\0';
}

/* Palindrom rekursif: lewati non-alfanumerik, bandingkan huruf kecil. */
static int palindrome_at(const char *s, int left, int right) {
    if (left >= right) return 1;
    if (!isalnum((unsigned char)s[left])) return palindrome_at(s, left + 1, right);
    if (!isalnum((unsigned char)s[right])) return palindrome_at(s, left, right - 1);
    if (tolower((unsigned char)s[left]) != tolower((unsigned char)s[right])) return 0;
    return palindrome_at(s, left + 1, right - 1);
}

/* Cek palindrom string kosong dianggap palindrom. */
int tt_palindrome_rec(const char *s) {
    if (s == NULL) return 0;
    return palindrome_at(s, 0, (int)strlen(s) - 1);
}
