CC = g++
CFLAGS = -Wall -Og 
F = front

genlexer: $F/lexer.l
	lex -o $F/lexer.c $F/lexer.l 

genparser: $F/yacc.y
	yacc -o $F/parser.cc -dy $F/yacc.y

parser: genlexer genparser
	gcc $(CFLAGS) $F/lexer.c -c -o $F/lexer.o;
	$(CC) $(CFLAGS) $F/parser.cc $F/lexer.o -o $F/parser.out
