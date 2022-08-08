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


test: $(MAIN)/test-main.cc parser lexer make-lexer make-parser
	$(MAKE) -C $D all
	$(MAKE) -C $U all
	$(CC) $(CFLAGS) $(UTILS) $(NODES) $F/parser.o $F/lexer.o  $(MAIN)/test-main.cc -o parser-test
