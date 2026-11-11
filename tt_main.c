/* Pintu masuk Text Toolkit: --test menjalankan harness, tanpa argumen
 * masuk menu utama. */
#include "text_toolkit.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        return test_text_toolkit();
    }
    run_text_toolkit();
    return 0;
}
