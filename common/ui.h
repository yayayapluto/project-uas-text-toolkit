#ifndef COMMON_UI_H
#define COMMON_UI_H

#include "common.h"
#include <stddef.h>

/* Tampilan terminal murni ASCII: lebar layout UI_WIDTH, tanpa ANSI,
 * tanpa box drawing Unicode. Semua fungsi hanya menulis ke stdout. */

#define UI_WIDTH 78

/* Inisialisasi UI: deteksi stdout/stdin terminal, aktifkan mode virtual
 * terminal Windows bila perlu, dan periksa penggantian manual (argumen
 * --plain lewat ui_set_plain atau variabel lingkungan NO_CLEAR).
 * Dipanggil sekali di awal run_text_toolkit. */
void ui_init(void);

/* Penanda mode tanpa layar bersih; dipanggil dari main saat --plain. */
void ui_set_plain(int plain);

/* 1 jika stdout dan stdin terminal DAN tidak ada penggantian manual. */
int ui_is_interactive(void);

/* Bersihkan layar: escape "\033[2J\033[H" bila didukung, 40 baris
 * kosong sebagai cadangan, atau tidak sama sekali saat tidak
 * interaktif (output terpipa atau dialihkan). */
void ui_clear(void);

/* Jeda "Tekan Enter untuk kembali..." hanya saat interaktif; EOF pada
 * jeda diabaikan dengan bersih. */
void ui_pause(void);

/* Bingkai judul sekali di awal program: garis sama dengan, judul, dua
 * baris deskripsi, garis penutup. */
void ui_banner(const char *title, const char *line1, const char *line2);

/* Pembuka layar setiap menu ditampilkan ulang: garis, judul, satu baris
 * status ringkas, garis. */
void ui_screen_header(const char *title, const char *status);

/* Satu item menu: "  N) Label" dengan lebar nomor konsisten. */
void ui_menu_item(int number, const char *label);

/* Judul bagian hasil diikuti garis tipis. */
void ui_section(const char *title);

/* Garis pemisah selebar layout. */
void ui_rule(void);

/* Pesan berawalan "[OK] ", "[i] ", "[!] ", "[Error] " + newline. */
void ui_ok(const char *fmt, ...);
void ui_info(const char *fmt, ...);
void ui_warn(const char *fmt, ...);
void ui_error(const char *fmt, ...);

/* Konfirmasi y/ya/n/tidak (tidak peka huruf); kosong memilih bawaan.
 * Kembali ST_OK dengan *answer 1/0, atau ST_BAD_INPUT saat EOF. */
Status ui_confirm(const char *question, int default_yes, int *answer);

/* Prompt pilihan seragam "Pilihan > " (tanpa newline). */
void ui_prompt_label(void);

/* Membaca path lewat util_read_line, trim, buang kutip di kedua ujung,
 * tolak kosong (ui_warn + ulangi). Kembali ST_OK atau ST_BAD_INPUT
 * saat EOF. buf milik pemanggil. */
Status ui_read_path(const char *prompt, char *buf, size_t size);

/* Membuang satu pasang kutip ganda atau tunggal di kedua ujung s. */
void ui_strip_quotes(char *s);

/* Mencetak s terpotong dengan "..." bila lebar tampilan melebihi
 * max_cols; tidak pernah memotong di tengah karakter UTF-8. */
void ui_print_truncated(const char *s, int max_cols);

/* Menyalin s terpotong ke out dengan "..." bila terpotong (batas
 * karakter UTF-8 aman); kembali lebar kolom hasil. */
int ui_truncate(const char *s, int max_cols, char *out, size_t size);

#endif
