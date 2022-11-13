CC=gcc
CFLAGS=-Wall -O2 -I.
DEPS = spstring.h
OBJ = spstring.o demo.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

demo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

check: demo
	./demo

clean:
	rm *.o
