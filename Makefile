CFLAGS=-Wall

cnitize: cnitize.c

clean:
	rm -r tmp cnitize

test: cnitize
	./test.sh
