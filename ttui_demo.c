/* Demo output CLI ala rich: panel, tabel, dan badge dengan true color. */
#include "ui/ttui.h"
#include <locale.h>
#include <stdio.h>

int main(void) {
    const char *lines[] = {
        "Analisis teks, mini grep, dan sandi dalam satu program",
        "C99 murni, stdout biasa, tanpa pustaka TUI"
    };
    const char *headers[] = {"Modul", "Fungsi", "Status"};
    const char *r0[] = {"tt_text", "tt_load_text, tt_free_text", "OK"};
    const char *r1[] = {"tt_grep", "tt_grep, tt_free_matches", "OK"};
    const char *r2[] = {"tt_analyzer", "stats, sort, histogram", "PERINGATAN"};
    const char *r3[] = {"tt_cipher", "Caesar, Vigenere, brute force", "OK"};
    const char *r4[] = {"tt_report", "tt_write_report", "GAGAL"};
    const char **rows[] = {r0, r1, r2, r3, r4};

    setlocale(LC_ALL, "");
    ttui_panel("Text Toolkit CLI", lines, 2, TTUI_ACCENT_R, TTUI_ACCENT_G, TTUI_ACCENT_B);
    printf("\n");
    ttui_table(headers, rows, 5, 3);
    printf("\nRingkasan: ");
    ttui_badge("LULUS", TTUI_BADGE_OK);
    printf("   ");
    ttui_badge("PERINGATAN", TTUI_BADGE_WARN);
    printf("   ");
    ttui_badge("GAGAL", TTUI_BADGE_FAIL);
    printf("   ");
    ttui_badge("INFO", TTUI_BADGE_INFO);
    printf("\n");
    return 0;
}
