CC=gcc
CFLAGS=-I.
DEPS = spstring.h
OBJ = spstring.o demo.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

demo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

check:
	./demo

clean:
	rm *.o
