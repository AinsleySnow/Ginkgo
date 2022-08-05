ifndef CPP
CPP = g++
endif
ifndef CFLAGS
CFLAGS = -Wall -I.
endif

ifndef OPT
OPT := 0
endif

ifeq ($(OPT), 0)
	$(CFLAGS) += -Og -g
endif
ifeq ($(OPT), 2)
	$(CFLAGS) += -O2
endif
ifeq ($(OPT), 3)
	$(CFLAGS) += -O3
endif

all: Declaration Declarator DeclSpec DirDecl Init InitDecl InitDeclList

clean:
	rm -f *.o

Declaration: Declaration.cc Declaration.h
	$(CPP) $(CFLAGS) -o Declaration.o -c Declaration.cc

Declarator: Declarator.cc Declarator.h
	$(CPP) $(CFLAGS) -o Declarator.o -c Declarator.cc

DeclSpec: DeclSpec.cc DeclSpec.h
	$(CPP) $(CFLAGS) -o DeclSpec.o -c DeclSpec.cc

DirDecl: DirDecl.cc DirDecl.h
	$(CPP) $(CFLAGS) -o DirDecl.o -c DirDecl.cc

Init: Init.cc Init.h
	$(CPP) $(CFLAGS) -o Init.o -c Init.cc

InitDecl: InitDecl.cc InitDecl.h
	$(CPP) $(CFLAGS) -o InitDecl.o -c InitDecl.cc

InitDeclList: InitDeclList.cc InitDeclList.h
	$(CPP) $(CFLAGS) -o InitDeclList.o -c InitDeclList.cc
