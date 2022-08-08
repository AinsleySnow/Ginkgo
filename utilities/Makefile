CC = g++
CFLAGS = -Wall -Og -g


all: ir.o SymbolTable.o

clean:
	rm -f *.o


ir.o: IR.cc IR.h
	$(CC) $(CFLAGS) IR.cc -c -o $@

SymbolTable.o: SymbolTable.cc SymbolTable.h
	$(CC) $(CFLAGS) SymbolTable.cc -c -o $@
