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
    -lm -o text_toolkit.exe
if errorlevel 1 (
    echo Build gagal.
    exit /b 1
)
echo Build sukses: text_toolkit.exe
text_toolkit.exe --test

rem Demo output CLI ala rich (ui\ttui).
gcc -std=c99 -Wall -Wextra -Wpedantic -g -I. ^
    ttui_demo.c ui\ttui.c common\common.c ^
    -lm -o ttui_demo.exe
if errorlevel 1 (
    echo Build demo gagal.
    exit /b 1
)
ttui_demo.exe
