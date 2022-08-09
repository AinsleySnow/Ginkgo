CC = g++
CFLAGS = -Wall -Og -g
F = front
D = front/declaration
MAIN = main
U = utilities


CHECK = $(MAIN)/test-declaration.o	\
		$(MAIN)/test-main.o

NODES = $D/Declaration.o 	\
	   $D/Declarator.o		\
	   $D/DeclSpec.o 		\
	   $D/DirDecl.o 		\
	   $D/Init.o			\
	   $D/InitDecl.o		\

UTILS = $U/SymbolTable.o	\
		$U/IR.o

$(MAIN)/test: $(UTILS) $(NODES) $(CHECK)
	$(CC) $(UTILS) $(NODES) $F/parser.o $F/lexer.o $(CHECK) -o $@

test: 
	$(MAKE) -C $F all
	$(MAKE) -C $U all
	$(MAKE) $(MAIN)/test
