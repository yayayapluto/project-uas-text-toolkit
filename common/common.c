#include "common.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/* --- static state for alloc balance and test harness --- */
static size_t allocs = 0;
static size_t frees = 0;
static int fail_alloc_after = -1;
static int alloc_count_after_fail = 0;

/* ---- util_status_message ---- */
const char *util_status_message(Status s) {
    switch (s) {
        case ST_OK: return "OK";
        case ST_FILE_NOT_FOUND: return "File not found";
        case ST_FILE_EMPTY: return "File empty";
        case ST_FILE_WRITE: return "File write error";
        case ST_BAD_INPUT: return "Bad input";
        case ST_BAD_FORMAT: return "Bad format";
        case ST_OUT_OF_RANGE: return "Out of range";
        case ST_NO_MEMORY: return "Out of memory";
        default: return "Unknown status";
    }
}

/* ---- util_read_line ---- */
Status util_read_line(const char *prompt, char *buf, size_t size) {
    if (prompt != NULL) printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL) {
        return ST_BAD_INPUT;
    }
    size_t n = strlen(buf);
    while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r'))
        buf[--n] = '\0';
    if (n == 0) return ST_BAD_INPUT;
    return ST_OK;
}

/* ---- util_read_int ---- */
Status util_read_int(const char *prompt, int min, int max, int *out) {
    char buf[UTIL_MAX_LINE];
    for (;;) {
        Status s = util_read_line(prompt, buf, sizeof(buf));
        if (s != ST_OK) return ST_BAD_INPUT;
        errno = 0;
        long lv = strtol(buf, NULL, 10);
        if (errno != 0 || *buf == '\0') {
            printf("Masukkan bilangan bulat yang valid antara %d dan %d.\n", min, max);
            continue;
        }
        int val = (int)lv;
        if (val < min || val > max) {
            printf("Nilai harus antara %d dan %d. Coba lagi: ", min, max);
            continue;
        }
        *out = val;
        return ST_OK;
    }
}

/* ---- util_trim ---- */
void util_trim(char *s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
    size_t start = 0;
    while (s[start] != '\0' && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
    s[len - start] = '\0';
}

/* ---- util_to_lower ---- */
void util_to_lower(char *s) {
    if (s == NULL) return;
    for (size_t i = 0; s[i]; i++) s[i] = tolower((unsigned char)s[i]);
}

/* ---- util_stricmp ---- */
int util_stricmp(const char *a, const char *b) {
    if (a == NULL || b == NULL) return (a == b) ? 0 : (a == NULL ? -1 : 1);
    for (;; a++, b++) {
        int ca = tolower((unsigned char)(*a));
        int cb = tolower((unsigned char)(*b));
        if (ca != cb) return ca < cb ? -1 : 1;
        if (ca == '\0') return 0;
    }
}

/* ---- util_read_file ---- */
Status util_read_file(const char *path, char **out, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return ST_FILE_NOT_FOUND;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        *out = NULL;
        *len = 0;
        return ST_FILE_EMPTY;
    }
    char *buf = (char *)util_malloc((size_t)sz + 1);
    if (buf == NULL) {
        fclose(f);
        return ST_NO_MEMORY;
    }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) {
        util_free(buf);
        return ST_FILE_WRITE;
    }
    buf[sz] = '\0';
    *out = buf;
    *len = (size_t)sz;
    return ST_OK;
}

/* ---- util_write_file ---- */
Status util_write_file(const char *path, const char *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (f == NULL) return ST_FILE_WRITE;
    size_t wr = fwrite(data, 1, len, f);
    fclose(f);
    if (wr != len) return ST_FILE_WRITE;
    return ST_OK;
}

/* ---- memory allocation wrappers ---- */
void *util_malloc(size_t n) {
    void *p = malloc(n);
    if (p != NULL) allocs++;
    if (fail_alloc_after >= 0) {
        alloc_count_after_fail++;
        if (alloc_count_after_fail > fail_alloc_after) {
            free(p);
            return NULL;
        }
    }
    return p;
}

void *util_calloc(size_t n, size_t size) {
    size_t total = n * size;
    void *p = calloc(n, size);
    if (p != NULL) allocs++;
    if (fail_alloc_after >= 0) {
        alloc_count_after_fail++;
        if (alloc_count_after_fail > fail_alloc_after) {
            free(p);
            return NULL;
        }
    }
    return p;
}

void *util_realloc(void *p, size_t n) {
    if (p == NULL) return util_malloc(n);
    if (n == 0) { util_free(p); return NULL; }
    void *np = realloc(p, n);
    if (np != NULL) allocs++;
    else {
        util_free(p);
        return NULL;
    }
    if (fail_alloc_after >= 0) {
        alloc_count_after_fail++;
        if (alloc_count_after_fail > fail_alloc_after) {
            free(np);
            return NULL;
        }
    }
    return np;
}

void util_free(void *p) {
    if (p == NULL) return;
    frees++;
    free(p);
}

int util_alloc_balance(void) {
    return (int)(allocs - frees);
}

void util_fail_alloc_after(int k) {
    fail_alloc_after = k;
    if (k >= 0) alloc_count_after_fail = 0;
    else fail_alloc_after = -1;
}

/* ---- test harness ---- */
void util_test_begin(const char *name) {
    printf("TEST: %s\n", name);
}

/* util_check prints [PASS] or [FAIL] and increments failure counter */
static int util_check_fail_cnt = 0;
void util_check(int cond, const char *desc) {
    if (cond) printf("  [PASS] %s\n", desc);
    else {
        printf("  [FAIL] %s\n", desc);
        util_check_fail_cnt++;
    }
}

void util_check_double(double got, double want, double eps, const char *desc) {
    util_check(fabs(got - want) <= eps, desc);
}

int util_test_end(void) {
    printf("RESULT: %d passed, %d failed\n",
           util_check_fail_cnt == 0 ? 0 : -1, util_check_fail_cnt);
    return util_check_fail_cnt;
}