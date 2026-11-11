#include "common.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

/* Pelacak alokasi: selisih jumlah alokasi sukses dan free. */
static size_t allocs = 0;
static size_t frees = 0;
static int fail_after = -1;
static int fail_count = 0;

/* Hitungan test harness. */
static int check_total = 0;
static int check_fail = 0;

/* Pesan bahasa Indonesia untuk tiap Status; tidak pernah NULL.
 * Parameter s: kode status. Kembali: string literal pesan. */
const char *util_status_message(Status s) {
    switch (s) {
        case ST_OK:             return "OK";
        case ST_FILE_NOT_FOUND: return "File tidak ditemukan";
        case ST_FILE_EMPTY:     return "File kosong";
        case ST_FILE_WRITE:     return "Gagal menulis file";
        case ST_BAD_INPUT:      return "Masukan tidak valid";
        case ST_BAD_FORMAT:     return "Format tidak valid";
        case ST_OUT_OF_RANGE:   return "Nilai di luar rentang";
        case ST_NO_MEMORY:      return "Memori tidak cukup";
        default:                return "Status tidak dikenal";
    }
}

/* Membaca satu baris stdin dengan fgets; membuang \n dan \r; baris panjang
 * dipotong dan sisanya dibuang. EOF -> ST_BAD_INPUT dan buffer kosong.
 * Parameter prompt boleh NULL; buf/size milik pemanggil. */
Status util_read_line(const char *prompt, char *buf, size_t size) {
    size_t n;
    if (buf == NULL || size < 2) return ST_BAD_INPUT;
    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout);
    }
    buf[0] = '\0';
    if (fgets(buf, (int)size, stdin) == NULL) return ST_BAD_INPUT;
    n = strlen(buf);
    if (n > 0 && buf[n - 1] != '\n') {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) buf[--n] = '\0';
    return ST_OK;
}

/* Mengulang sampai bilangan bulat utuh dalam [min, max]; strtol dengan
 * endptr menolak "12abc". EOF -> ST_BAD_INPUT tanpa berputar. Hasil di *out. */
Status util_read_int(const char *prompt, int min, int max, int *out) {
    char buf[UTIL_MAX_LINE];
    for (;;) {
        long lv;
        char *end;
        if (util_read_line(prompt, buf, sizeof(buf)) != ST_OK) return ST_BAD_INPUT;
        util_trim(buf);
        errno = 0;
        lv = strtol(buf, &end, 10);
        if (end == buf || *end != '\0' || errno == ERANGE) {
            ui_warn("Masukkan bilangan bulat utuh (%d sampai %d).", min, max);
            continue;
        }
        if (lv < (long)min || lv > (long)max) {
            ui_warn("Nilai harus %d sampai %d.", min, max);
            continue;
        }
        *out = (int)lv;
        return ST_OK;
    }
}

/* Menghapus spasi di tepi string dengan memmove. Mengubah buf di tempat. */
void util_trim(char *s) {
    size_t len, start;
    if (s == NULL) return;
    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    start = 0;
    while (s[start] != '\0' && isspace((unsigned char)s[start])) start++;
    if (start > 0) {
        memmove(s, s + start, len - start + 1);
    }
}

/* Mengubah semua huruf menjadi kecil di tempat. */
void util_to_lower(char *s) {
    if (s == NULL) return;
    for (; *s != '\0'; s++) *s = (char)tolower((unsigned char)*s);
}

/* Pembanding string peka-kapital-abai bergaya strcmp; tidak memakai
 * strcasecmp agar portabel. Kembali <0, 0, atau >0. */
int util_stricmp(const char *a, const char *b) {
    for (; *a != '\0' && *b != '\0'; a++, b++) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return (ca < cb) ? -1 : 1;
    }
    if (*a != *b) return (*a == '\0') ? -1 : 1;
    return 0;
}

/* Membaca seluruh file biner ke buffer NUL-terminated via util_malloc;
 * *len = jumlah byte tanpa NUL. Tidak ada -> ST_FILE_NOT_FOUND; 0 byte ->
 * ST_FILE_EMPTY tanpa alokasi tersisa. Pemanggil membebaskan *out lewat
 * util_free. */
Status util_read_file(const char *path, char **out, size_t *len) {
    FILE *f;
    long sz;
    char *buf;
    size_t rd;
    if (path == NULL || out == NULL || len == NULL) return ST_BAD_INPUT;
    *out = NULL;
    *len = 0;
    f = fopen(path, "rb");
    if (f == NULL) return ST_FILE_NOT_FOUND;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return ST_FILE_WRITE; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return ST_FILE_WRITE; }
    if (sz == 0) { fclose(f); return ST_FILE_EMPTY; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return ST_FILE_WRITE; }
    buf = (char *)util_malloc((size_t)sz + 1);
    if (buf == NULL) { fclose(f); return ST_NO_MEMORY; }
    rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) { util_free(buf); return ST_FILE_WRITE; }
    buf[sz] = '\0';
    *out = buf;
    *len = (size_t)sz;
    return ST_OK;
}

/* Menulis len byte data ke file mode "wb". Gagal buka/tulis ->
 * ST_FILE_WRITE. */
Status util_write_file(const char *path, const char *data, size_t len) {
    FILE *f;
    size_t wr;
    if (path == NULL || (data == NULL && len > 0)) return ST_BAD_INPUT;
    f = fopen(path, "wb");
    if (f == NULL) return ST_FILE_WRITE;
    wr = (len > 0) ? fwrite(data, 1, len, f) : 0;
    if (fclose(f) != 0 || wr != len) return ST_FILE_WRITE;
    return ST_OK;
}

/* Alokasi malloc terlacak; NULL saat injeksi kegagalan aktif dan kuota
 * habis. Blok sukses dihitung ke balance. Pemanggil membebaskan via util_free. */
void *util_malloc(size_t n) {
    void *p;
    if (fail_after >= 0) {
        fail_count++;
        if (fail_count > fail_after) return NULL;
    }
    p = malloc(n);
    if (p != NULL) allocs++;
    return p;
}

/* Alokasi calloc terlacak; kontrak sama dengan util_malloc. */
void *util_calloc(size_t n, size_t size) {
    void *p;
    if (fail_after >= 0) {
        fail_count++;
        if (fail_count > fail_after) return NULL;
    }
    p = calloc(n, size);
    if (p != NULL) allocs++;
    return p;
}

/* realloc terlacak; NULL -> alokasi baru. Gagal: blok lama utuh, NULL
 * kembali. n == 0 membebaskan blok. Pemanggil membebaskan hasil via util_free. */
void *util_realloc(void *p, size_t n) {
    void *np;
    if (p == NULL) return util_malloc(n);
    if (n == 0) { util_free(p); return NULL; }
    if (fail_after >= 0) {
        fail_count++;
        if (fail_count > fail_after) return NULL;
    }
    np = realloc(p, n);
    if (np == NULL) return NULL;
    if (np != p) { frees++; allocs++; }
    return np;
}

/* Membebaskan blok yang dialokasi util_*; NULL aman dan tidak dihitung. */
void util_free(void *p) {
    if (p == NULL) return;
    frees++;
    free(p);
}

/* Balance = alokasi sukses dikurangi free; 0 berarti tidak bocor. */
int util_alloc_balance(void) {
    return (int)(allocs - frees);
}

/* Injeksi kegagalan: k alokasi berikut sukses, sisanya NULL sampai
 * dipanggil lagi; -1 mematikan injeksi. */
void util_fail_alloc_after(int k) {
    fail_after = k;
    fail_count = 0;
}

/* Mencetak judul kelompok uji dan mengreset hitungan. */
void util_test_begin(const char *name) {
    printf("=== %s ===\n", name);
    check_total = 0;
    check_fail = 0;
}

/* Memeriksa kondisi; mencetak [PASS]/[FAIL] plus deskripsi dan menghitung. */
void util_check(int cond, const char *desc) {
    check_total++;
    if (cond) {
        printf("[PASS] %s\n", desc);
    } else {
        check_fail++;
        printf("[FAIL] %s\n", desc);
    }
}

/* Memeriksa dua double dengan toleransi selisih mutlak eps. */
void util_check_double(double got, double want, double eps, const char *desc) {
    util_check(fabs(got - want) <= eps, desc);
}

/* Mencetak ringkasan kelompok uji; kembali jumlah gagal (0 = lulus semua). */
int util_test_end(void) {
    printf("Ringkasan: %d lulus, %d gagal\n", check_total - check_fail, check_fail);
    return check_fail;
}
