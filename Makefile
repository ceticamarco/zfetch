TARGET = zfetch
CC = clang
CFLAGS = -Wall -Wextra -Werror -pedantic-errors -Wwrite-strings -std=c99 -O3

all: clean $(TARGET)

$(TARGET): zfetch.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o *.a $(TARGET)
