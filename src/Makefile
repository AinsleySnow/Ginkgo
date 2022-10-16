CC = g++
CFLAGS = -Wall -I. -Og -g
D = declaration
E = expression

all: $D/Declaration.o $D/Declarator.o $D/DeclSpec.o 				   \
	$D/DirDecl.o $D/Init.o $D/InitDecl.o   		    				   \
	$E/AddExpr.o $E/AndExpr.o $E/AssignExpr.o $E/CastExpr.o 			\
	$E/CondExpr.o $E/EqualExpr.o $E/EqualExpr.o $E/Expression.o 		\
	$E/InclOrExpr.o $E/LogicalAndExpr.o $E/LogicalOrExpr.o				\
	$E/MultiExpr.o $E/PostfixExpr.o $E/PrimaryExpr.o $E/RelationExpr.o  \
	$E/ShiftExpr.o $E/UnaryExpr.o $E/XorExpr.o							\
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

$E/AddExpr.o: $E/AddExpr.cc $E/AddExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/AddExpr.cc

$E/AndExpr.o: $E/AndExpr.cc $E/AndExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/AndExpr.cc

$E/AssignExpr.o: $E/AssignExpr.cc $E/AssignExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/AssignExpr.cc

$E/CastExpr.o: $E/CastExpr.cc $E/CastExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/CastExpr.cc

$E/CondExpr.o: $E/CondExpr.cc $E/CondExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/CondExpr.cc

$E/EqualExpr.o: $E/EqualExpr.cc $E/EqualExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/EqualExpr.cc

$E/Expression.o: $E/Expression.cc $E/Expression.h
	$(CC) $(CFLAGS) -o $@ -c $E/Expression.cc

$E/InclOrExpr.o: $E/InclOrExpr.cc $E/InclOrExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/InclOrExpr.cc

$E/LogicalAndExpr.o: $E/LogicalAndExpr.cc $E/LogicalAndExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/LogicalAndExpr.cc

$E/LogicalOrExpr.o: $E/LogicalOrExpr.cc $E/LogicalOrExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/LogicalOrExpr.cc

$E/MultiExpr.o: $E/MultiExpr.cc $E/MultiExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/MultiExpr.cc

$E/PostfixExpr.o: $E/PostfixExpr.cc $E/PostfixExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/PostfixExpr.cc

$E/PrimaryExpr.o: $E/PrimaryExpr.cc $E/PrimaryExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/PrimaryExpr.cc

$E/RelationExpr.o: $E/RelationExpr.cc $E/RelationExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/RelationExpr.cc

$E/ShiftExpr.o: $E/ShiftExpr.cc $E/ShiftExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/ShiftExpr.cc

$E/UnaryExpr.o: $E/UnaryExpr.cc $E/UnaryExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/UnaryExpr.cc

$E/XorExpr.o: $E/XorExpr.cc $E/XorExpr.h
	$(CC) $(CFLAGS) -o $@ -c $E/XorExpr.cc

parser.cc: yacc.y
	yacc -o $@ -dy -t yacc.y
	
lexer.cc: lexer.l
	lex -o $@ lexer.l 

parser.o: parser.cc
	$(CC) $(CFLAGS) parser.cc -c -o parser.o

lexer.o: lexer.cc
	$(CC) $(CFLAGS) lexer.cc -c -o lexer.o
