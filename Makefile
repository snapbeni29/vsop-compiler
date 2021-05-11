CXX=clang++
CXXFLAGS=-std=c++14 -Wno-everything
LLFLAGS = `llvm-config-11 --cxxflags --ldflags --libs`

all:	vsopc

clean:
	rm -rf build

dist-clean:	clean
	rm -f vsopc

install-tools:
	sudo apt install flex bison clang

lex.yy.c: parser.tab.c lexer.l
	flex lexer.l

parser.tab.c: parser.y
	bison -v -d parser.y

vsopc:	lex.yy.c
	$(CXX) $(CXXFLAGS) $(LLFLAGS) -g lex.yy.c parser.tab.c tree.cc -o vsopc

# Phony rules are those that do not correspond to an actual file
.PHONY: all clean dist-clean
