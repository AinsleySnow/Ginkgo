CC = g++
CFLAGS = -Wall -Og -g
F = front
D = front/declaration
MAIN = main
U = utilities


NODES = $D/Declaration.o 	\
	   $D/Declarator.o		\
	   $D/DeclSpec.o 		\
	   $D/DirDecl.o 		\
	   $D/Init.o			\
	   $D/InitDecl.o		\

UTILS = $U/SymbolTable.o	\
		$U/IR.o


parser: $F/yacc.y
	yacc -o $F/parser.cc -dy -t $F/yacc.y
	
lexer: $F/lexer.l
	lex -o $F/lexer.cc $F/lexer.l 

make-parser:
	$(CC) $(CFLAGS) $F/parser.cc -c -o $F/parser.o

make-lexer:
	$(CC) $(CFLAGS) $F/lexer.cc -c -o $F/lexer.o

test: $(MAIN)/test-main.cc parser lexer make-lexer make-parser
	$(MAKE) -C $D all
	$(MAKE) -C $U symtbl ir
	$(CC) $(CFLAGS) $(UTILS) $(NODES) $F/parser.o $F/lexer.o  $(MAIN)/test-main.cc -o parser-test
