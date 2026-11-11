#include "common.h"
#include "tt_text.h"
#include "tt_grep.h"
#include "tt_analyzer.h"
#include "tt_cipher.h"
#include "tt_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        return test_text_toolkit();
    }
    printf("Penggunaan: text_toolkit --test\n");
    return 0;
}

int test_text_toolkit(void) {
    TtText text = {0};
    Status s = tt_load_text("data/contoh.txt", &text);
    if (s != ST_OK) { printf("Gagal load data: %s\n", util_status_message(s)); return 1; }
    TtMatch *matches = NULL;
    int match_count = 0, total = 0;
    tt_grep(&text, "kata", 0, 0, &matches, &match_count, &total);
    printf("Grep found %d matches (total occurrences %d)\n", match_count, total);
    tt_free_text(&text);
    return 0;
}