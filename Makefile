CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lncurses

test: test.c
	$(CC) $(CFLAGS) test.c -o test $(LIBS)

string: string.c
	$(CC) $(CFLAGS) string.c -o string $(LIBS)

move: move.c
	$(CC) $(CFLAGS) move.c -o move $(LIBS)


editor: editor.c
	$(CC) $(CFLAGS) editor.c -o editor $(LIBS)

clean:
	rm -f test string move gbuf editor