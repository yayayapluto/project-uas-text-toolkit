#include "tt_cipher.h"
#include "common.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static int is_key_valid(const char *key) {
    if (key == NULL) return 0;
    for (;; key++) {
        if (!isalpha((unsigned char)key[0])) return 0;
    }
    return 1;
}

static char shift_char(char c, int shift, int encrypt) {
    if (isupper((unsigned char)c))
        return (char)(((unsigned char)c - 'A' + (encrypt ? shift : -shift) % 26 + 26) % 26 + 'A');
    if (islower((unsigned char)c))
        return (char)(((unsigned char)c - 'a' + (encrypt ? shift : -shift) % 26 + 26) % 26 + 'a');
    return c;
}

Status tt_caesar(char *text, int shift) {
    if (text == NULL) return ST_BAD_INPUT;
    shift = ((shift % 26) + 26) % 26;
    for (char *p = text; *p; p++) *p = shift_char(*p, shift, 1);
    return ST_OK;
}

Status tt_vigenere(char *text, const char *key, int decrypt) {
    if (text == NULL || key == NULL) return ST_BAD_INPUT;
    if (!is_key_valid(key)) return ST_BAD_INPUT;
    int klen = (int)strlen(key);
    int shifts[26];
    for (int i = 0; i < klen; i++) shifts[i] = tolower((unsigned char)key[i]) - 'a';
    int ki = 0;
    for (char *p = text; *p; p++) {
        if (isalpha((unsigned char)*p)) {
            int s = shifts[ki % klen];
            if (!decrypt) s = -s;
            *p = shift_char(*p, s, 1);
            ki++;
        }
    }
    return ST_OK;
}

Status tt_cipher_file(const char *in_path, const char *out_path, int mode,
                      int shift, const char *key) {
    char *data;
    size_t len;
    Status s = util_read_file(in_path, &data, &len);
    if (s != ST_OK) return s;
    if (mode == 0) tt_caesar(data, shift);
    else if (mode == 1) tt_caesar(data, -shift);
    else if (mode == 2) s = tt_vigenere(data, key, 0);
    else if (mode == 3) s = tt_vigenere(data, key, 1);
    else { util_free(data); return ST_BAD_INPUT; }
    if (s != ST_OK) { util_free(data); return s; }
    s = util_write_file(out_path, data, len);
    util_free(data);
    return s;
}

void tt_caesar_brute(const char *text) {
    if (text == NULL) return;
    size_t n = strlen(text);
    char *buf = (char *)util_malloc(n + 1);
    memcpy(buf, text, n); buf[n] = '\0';
    for (int i = 0; i < 26; i++) {
        tt_caesar(buf, i);
        printf("Geser %d: %s\n", i, buf);
    }
    util_free(buf);
}

void tt_count_letters(const char *s, int *upper, int *lower, int *other) {
    if (s == NULL || upper == NULL || lower == NULL || other == NULL) return;
    *upper = *lower = *other = 0;
    for (const char *p = s; *p; p++) {
        if (isupper((unsigned char)*p)) (*upper)++;
        else if (islower((unsigned char)*p)) (*lower)++;
        else (*other)++;
    }
}