CXX = g++
CFLAGS = -std=c++17 -I. -Wall -Og -g

A = ast
M = messages
P = parser
TE = test
TY = types
U = utils
V = visitors


Constant.o: $A/Constant.cc $A/Constant.h
	$(CXX) $(CFLAGS) $A/Constant.cc -c -o $@


errmsg.o: $M/errmsg-en_us.cc
	$(CXX) $(CFLAGS) $^ -c -o $@

Error.o: $M/Error.cc $M/Error.h
	$(CXX) $(CFLAGS) $M/Error.cc -c -o $@


$P/lexer.cc: $P/lexer.ll
	flex -o $@ $^ 

$P/yacc.cc: $P/yacc.yy
	bison -o $@ -dy $^ 

lexer.o: $P/lexer.cc $P/lexer.ll
	$(CXX) $(CFLAGS) $P/lexer.cc -c -o $@

yacc.o: $P/yacc.cc $P/yacc.yy
	$(CXX) $(CFLAGS) $P/yacc.cc -c -o $@


ArithmType.o: $(TY)/ArithmType.cc $(TY)/ArithmType.h
	$(CXX) $(CFLAGS) $(TY)/ArithmType.cc -c -o $@

Type.o: $(TY)/Type.cc $(TY)/Type.h
	$(CXX) $(CFLAGS) $(TY)/Type.cc -c -o $@


IR.o: $U/IR.cc $U/IR.h
	$(CXX) $(CFLAGS) $U/IR.cc -c -o $@

Scope.o: $U/Scope.cc $U/Scope.h
	$(CXX) $(CFLAGS) $U/Scope.cc -c -o $@


IRGen.o: $V/IRGen.cc $V/IRGen.h
	$(CXX) $(CFLAGS) $V/IRGen.cc -c -o $@


test-main.o: $(TE)/test-main.cc
	$(CXX) $(CFLAGS) $^ -c -o $@

test-subfuncs.o: $(TE)/test-subfuncs.cc
	$(CXX) $(CFLAGS) $^ -c -o $@


all: Constant.o errmsg.o Error.o yacc.o lexer.o ArithmType.o Type.o IR.o Scope.o IRGen.o test-main.o test-subfuncs.o
