CC = g++
CFLAGS = -Wall -Og -g

all: test-declaration.o test.o

clean:
	rm -f *.o


test-subfuncs.o: test-subfuncs.cc
	$(CC) $(CFLAGS) $^ -c -o $@

test-main.o: test-main.cc
	$(CC) $(CFLAGS) $^ -c -o $@
