/* Pintu masuk Text Toolkit: --test menjalankan harness, --plain
 * menonaktifkan layar bersih dan jeda, tanpa argumen masuk menu utama. */
#include "text_toolkit.h"
#include "common/ui.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0) {
            return test_text_toolkit();
        }
        if (strcmp(argv[i], "--plain") == 0) {
            ui_set_plain(1);
        }
    }
    run_text_toolkit();
    return 0;
}
