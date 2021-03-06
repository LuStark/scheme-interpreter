VPATH = src

objects=\
	LL1.o Number.o scan.o AST.o Fraction.o\
	lex.yy.o procedure.o LispEnvironment.o\
	SymbolTable.o

scheme-interpreter: $(objects)
	g++ -g $(objects) -o scheme-interpreter
	rm $(objects)

LL1.o: LL1.h LL1.cpp Tree.h Token.h tokenType.h LispEnvironment.h procedure.h

Number.o: Number.h Number.cpp

scan.o: Token.h lex.yy.h scan.cpp

AST.o: Tree.h LL1.h Token.h Number.h AST.cpp tokenType.h

Fraction.o: Fraction.h Fraction.cpp

lex.yy.o: Token.h lex.yy.c
	g++ -g -c src/Token.h src/lex.yy.c

procedure.o: procedure.cpp procedure.h Tree.h LL1.h LispEnvironment.h
    
LispEnvironment.o: SymbolTable.h LispEnvironment.cpp LispEnvironment.h procedure.h macro.h

SymbolTable.o: SymbolTable.h SymbolTable.cpp Tree.h macro.h
