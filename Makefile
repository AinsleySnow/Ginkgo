CC = g++
CFLAGS = -Wall -Og -g
F = front
D = front/declaration
MAIN = main
U = utilities
E = front/expression


CHECK = $(MAIN)/test-subfuncs.o	\
		$(MAIN)/test-main.o

NODES = $D/Declaration.o 	\
	   $D/Declarator.o		\
	   $D/DeclSpec.o 		\
	   $D/DirDecl.o 		\
	   $D/Init.o			\
	   $D/InitDecl.o		\
	   $E/AddExpr.o			\
	   $E/AndExpr.o			\
	   $E/AssignExpr.o		\
	   $E/CastExpr.o		\
	   $E/CondExpr.o		\
	   $E/EqualExpr.o		\
	   $E/Expression.o		\
	   $E/InclOrExpr.o		\
	   $E/LogicalAndExpr.o	\
	   $E/LogicalOrExpr.o	\
	   $E/MultiExpr.o		\
	   $E/PostfixExpr.o		\
	   $E/PrimaryExpr.o		\
	   $E/RelationExpr.o	\
	   $E/ShiftExpr.o		\
	   $E/UnaryExpr.o		\
	   $E/XorExpr.o

UTILS = $U/SymbolTable.o	\
		$U/IR.o

$(MAIN)/test: $(UTILS) $(NODES) $(CHECK) $F/parser.o $F/lexer.o
	$(CC) $(UTILS) $(NODES) $F/parser.o $F/lexer.o $(CHECK) -o $@

test: 
	$(MAKE) -C $F all
	$(MAKE) -C $U all
	$(MAKE) $(MAIN)/test

clean:
	$(MAKE) -C $F clean
	$(MAKE) -C $U clean
	$(MAKE) -C $(MAIN) clean
