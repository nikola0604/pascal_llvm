CC=clang++
INC_PATH=/usr/lib/llvm-3.8/include/
CFLAGS=$(shell llvm-config-3.8 --cxxflags) -I$(INC_PATH) -std=c++11
LDFLAGS=$(shell llvm-config-3.8 --ldflags --system-libs --libs) -I$(INC_PATH) -std=c++11

pascal: lex.yy.o parser.tab.o ast.o
	$(CC) -o $@ $^ $(LDFLAGS)
parser.tab.o: parser.tab.cpp parser.tab.hpp ast.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -c -o $@ $<
parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -d -v $<
lex.yy.o: lex.yy.c parser.tab.hpp ast.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -Wno-sign-compare -c -o $@ $<
lex.yy.c: lexer.lex
	flex $<
ast.o: ast.cpp ast.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -rf *~ *tab* lex.yy.* *.o pascal *.output a.out
