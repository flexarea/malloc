my-malloc: my-malloc.c
	gcc -g -Wall -pedantic -rdynamic -shared -fPIC -o my-malloc.so my-malloc.c

test: test.c
	gcc -g -Wall -pedantic -o test test.c

.PHONY: debug
debug: my-malloc.so test
	gdb --args env LD_PRELOAD=./my-malloc.so ./test
	
.PHONY: clean
clean:
	rm -f my-malloc.so test
