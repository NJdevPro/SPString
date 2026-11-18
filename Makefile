CC=gcc
CFLAGS=-Wall -O2 -I.
DEPS = spstring.h
OBJ = spstring.o demo.o 

.PHONY: check test test-tidy clean

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

demo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

test_spstring: spstring.c test_spstring.c spstring.h
	$(CC) -o $@ spstring.c test_spstring.c $(CFLAGS) -pthread

test: test_spstring
	./test_spstring

test-tidy:
	$(CC) -DTIDY -o test_spstring_tidy spstring.c test_spstring.c $(CFLAGS) -pthread
	./test_spstring_tidy

check: test-tidy test

clean:
	rm -f *.o test_spstring test_spstring_tidy demo
