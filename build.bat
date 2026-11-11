@echo off
rem Skrip build MinGW-w64 setara Makefile; hasil text_toolkit.exe.
gcc -std=c99 -Wall -Wextra -Wpedantic -g -I. ^
    tt_main.c text_toolkit.c tt_tests.c ^
    common\common.c ^
    tt_text\tt_text.c ^
    tt_grep\tt_grep.c ^
    tt_analyzer\tt_analyzer.c ^
    tt_cipher\tt_cipher.c ^
    tt_report\tt_report.c ^
    ui\ttui.c ^
    -lm -o text_toolkit.exe
if errorlevel 1 (
    echo Build gagal.
    exit /b 1
)
echo Build sukses: text_toolkit.exe
text_toolkit.exe --test
