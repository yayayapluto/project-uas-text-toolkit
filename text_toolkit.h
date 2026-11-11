#ifndef TEXT_TOOLKIT_H
#define TEXT_TOOLKIT_H

/* Menu utama Text Toolkit; seluruh state dipegang lokal, memori dibebaskan
 * saat keluar. */
void run_text_toolkit(void);

/* Test harness U01 sampai U17 beserta uji tambahan; kembali 0 saat lulus. */
int test_text_toolkit(void);

#endif
