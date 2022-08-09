CC = g++
CFLAGS = -Wall -Og -g

all: test-declaration.o test.o

clean:
	rm -f *.o


test-declaration.o: test-declaration.cc
	$(CC) $(CFLAGS) $^ -c -o $@

test-main.o: test-main.cc
	$(CC) $(CFLAGS) $^ -c -o $@
