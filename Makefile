CC = g++
CFLAGS = -Wall -Og -g
F = front
D = front/declaration

include front/declaration/Makefile

OBJS = $F/Declaration.o 	\
	   $F/Declarator.o		\
	   $F/DeclSpec.o 		\
	   $F/DirDecl.o 		\
	   $F/Init.o			\
	   $F/InitDecl.o		\
	   $F/InitDeclList.o	


lexer: $F/lexer.l
	lex -o $F/lexer.cc $F/lexer.l 
	$(CC) $(CFLAGS) $F/lexer.cc -c -o $F/lexer.o

parser: lexer $F/yacc.y
	yacc -o $F/parser.cc -dy $F/yacc.y
	$(CC) $(CFLAGS) $(OBJS) $F/parser.cc $F/lexer.o -o $F/parser.out
