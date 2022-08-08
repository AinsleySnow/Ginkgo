CC = g++
CFLAGS = -Wall -I. -Og -g
D = declaration

all: $D/Declaration.o $D/Declarator.o $D/DeclSpec.o \
	$D/DirDecl.o $D/Init.o $D/InitDecl.o   		    \
	parser.o lexer.o

clean:
	rm -f *.o


$D/Declaration.o: $D/Declaration.cc $D/Declaration.h
	$(CC) $(CFLAGS) -o $@ -c $D/Declaration.cc 

$D/Declarator.o: $D/Declarator.cc $D/Declarator.h
	$(CC) $(CFLAGS) -o $@ -c $D/Declarator.cc

$D/DeclSpec.o: $D/DeclSpec.cc $D/DeclSpec.h
	$(CC) $(CFLAGS) -o $@ -c $D/DeclSpec.cc

$D/DirDecl.o: $D/DirDecl.cc $D/DirDecl.h
	$(CC) $(CFLAGS) -o $@ -c $D/DirDecl.cc

$D/Init.o: $D/Init.cc $D/Init.h
	$(CC) $(CFLAGS) -o $@ -c $D/Init.cc

$D/InitDecl.o: $D/InitDecl.cc $D/InitDecl.h
	$(CC) $(CFLAGS) -o $@ -c $D/InitDecl.cc

parser.cc: yacc.y
	yacc -o $@ -dy -t yacc.y
	
lexer.cc: lexer.l
	lex -o $@ lexer.l 

parser.o: parser.cc
	$(CC) $(CFLAGS) parser.cc -c -o parser.o

lexer.o: lexer.cc
	$(CC) $(CFLAGS) lexer.cc -c -o lexer.o
