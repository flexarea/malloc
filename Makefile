CFLAGS=-g -Wall -pedantic
my-malloc.so: my-malloc.c
	gcc $(CFLAGS) -rdynamic -shared -fPIC -o $@ $^

test: test.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: debug
debug: my-malloc.so test
	gdb --args env LD_PRELOAD=./my-malloc.so ./test
	
.PHONY: clean
clean:
	rm -f my-malloc.so test
