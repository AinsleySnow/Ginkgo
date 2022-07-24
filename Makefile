CC = g++
CFLAGS = -Wall -Og 
F = front

lexer: $F/lexer.l
	lex -o $F/lexer.cc $F/lexer.l 
	$(CC) $(CFLAGS) $F/lexer.cc -c -o $F/lexer.o;

parser: lexer $F/yacc.y
	yacc -o $F/parser.cc -dy $F/yacc.y
	$(CC) $(CFLAGS) $F/parser.cc $F/lexer.o -o $F/parser.out
