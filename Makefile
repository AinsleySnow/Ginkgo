CXX = g++
CFLAGS = -std=c++17 -Wall -Og -g

S = src
T = tests

OBJS = $S/Constant.o 		\
	   $S/errmsg.o			\
	   $S/Error.o			\
	   $S/lexer.o			\
	   $S/yacc.o			\
	   $S/ArithmType.o		\
	   $S/Type.o			\
	   $S/IR.o				\
	   $S/Scope.o			\
	   $S/IRGen.o 			\
	   $S/test-subfuncs.o	\
	   $S/test-main.o


test:
	$(MAKE) -C src all
	$(CXX) $(CFLAGS) $(OBJS) -o tests/test


clean:
	$(MAKE) -C $(MAIN) clean
