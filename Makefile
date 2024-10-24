CFLAGS=-Wall -pedantic

.PHONY: all
all: test

test: my-malloc.o
	gcc -o $@ $^
%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^
.PHONY: clean
clean:
	rm -f my-malloc
