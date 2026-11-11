#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

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

#endif