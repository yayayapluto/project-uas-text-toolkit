CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -g -I.
LDLIBS = -lm

SRC = tt_main.c text_toolkit.c tt_tests.c \
      common/common.c \
      tt_text/tt_text.c \
      tt_grep/tt_grep.c \
      tt_analyzer/tt_analyzer.c \
      tt_cipher/tt_cipher.c \
      tt_report/tt_report.c

EXE = text_toolkit
DEMO = ttui_demo

all: $(EXE) $(DEMO)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LDLIBS)

$(DEMO): ttui_demo.c ui/ttui.c common/common.c
	$(CC) $(CFLAGS) ttui_demo.c ui/ttui.c common/common.c -o $(DEMO) $(LDLIBS)

test: $(EXE)
	./$(EXE) --test

asan:
	$(CC) $(CFLAGS) -fsanitize=address,undefined $(SRC) -o text_toolkit_asan $(LDLIBS)
	./text_toolkit_asan --test

clean:
	-rm $(EXE) $(DEMO) text_toolkit_asan
