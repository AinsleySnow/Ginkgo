CC = g++
CFLAGS = -Wall -Og 
F = front

genlexer: $F/lexer.l
	lex -o $F/lexer.c $F/lexer.l 

genparser: $F/yacc.y
	yacc -o $F/parser.c -dy $F/yacc.y

parser: genlexer genparser
	gcc $F/lexer.c -c -o $F/lexer.o;
	gcc $F/parser.c $F/lexer.o -o $F/parser.out
