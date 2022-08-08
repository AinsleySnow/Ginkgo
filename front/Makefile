CPP = g++
CFLAGS = -Wall -I. -Og -g
D = declaration

all: $D/Declaration $D/Declarator $D/DeclSpec \
	$D/DirDecl $D/Init $D/InitDecl			  \
	parser.o lexer.o

clean:
	rm -f *.o


$D/Declaration.o: Declaration.cc Declaration.h
	$(CPP) $(CFLAGS) -o $@ -c Declaration.cc

$D/Declarator.o: Declarator.cc Declarator.h
	$(CPP) $(CFLAGS) -o $@ -c Declarator.cc

$D/DeclSpec.o: DeclSpec.cc DeclSpec.h
	$(CPP) $(CFLAGS) -o $@ -c DeclSpec.cc

$D/DirDecl.o: DirDecl.cc DirDecl.h
	$(CPP) $(CFLAGS) -o $@ -c DirDecl.cc

$D/Init.o: Init.cc Init.h
	$(CPP) $(CFLAGS) -o $@ -c Init.cc

$D/InitDecl.o: InitDecl.cc InitDecl.h
	$(CPP) $(CFLAGS) -o $@ -c InitDecl.cc

parser.cc: yacc.y
	yacc -o $@ -dy -t yacc.y
	
lexer.cc: lexer.l
	lex -o $@ lexer.l 

parser.o: parser.cc
	$(CC) $(CFLAGS) parser.cc -c -o parser.o

lexer.o: lexer.cc
	$(CC) $(CFLAGS) lexer.cc -c -o lexer.o
