#ifndef TT_CIPHER_H
#define TT_CIPHER_H

#include "common.h"

Status tt_caesar(char *text, int shift);
Status tt_vigenere(char *text, const char *key, int decrypt);
Status tt_cipher_file(const char *in_path, const char *out_path, int mode,
                      int shift, const char *key);
void tt_caesar_brute(const char *text);
void tt_count_letters(const char *s, int *upper, int *lower, int *other);

#endif