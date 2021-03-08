CXX=clang++
CXXFLAGS=-std=c++14 -Wno-everything

all:	vsopc

clean:
	rm -rf build

dist-clean:	clean
	rm -f vsopc

install-tools:
	sudo apt install flex bison clang

lex.yy.c: lexer.l
	flex $^

vsopc:	lex.yy.c
	$(CXX) $(CXXFLAGS) -o $@ $^

# Phony rules are those that do not correspond to an actual file
.PHONY: all clean dist-clean
