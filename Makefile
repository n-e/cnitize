CFLAGS=-Wall -O3

debug: CFLAGS=-Wall -g

cnitize: cnitize.c

cnitize-afl: cnitize.c
	AFL_HARDEN=1 afl-clang cnitize.c -o cnitize-afl

debug: cnitize

clean:
	rm -r tmp cnitize

test: cnitize
	./test.sh
