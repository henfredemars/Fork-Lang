LLVM_INC := -I./llvm/include -I./llvm/build/include -I./llvm/examples/Kaleidoscope/include
LLVM_BIN := ./llvm/build/*+Asserts/bin/llvm-config

#export REQUIRES_RTTI = 1

all: parser CTest

parser: .gc_built_marker .llvm_built_marker parser.o lex.o node.o codeGenVisitor.o main.o parser.hpp lib.o
	g++ `$(LLVM_BIN) --cxxflags --ldflags` -rdynamic -o parser parser.o lex.o node.o codeGenVisitor.o lib.o main.o `$(LLVM_BIN) --libfiles` ./gc/.libs/libgc.a -L./gc/.libs -lpthread -ltinfo `$(LLVM_BIN) --system-libs`

parser.cpp: parser.y
	touch parser.cpp; bison -d -o parser.cpp parser.y

parser.o: parser.cpp
	g++ `$(LLVM_BIN) --cxxflags` -c parser.cpp -o parser.o $(LLVM_INC)

lex.cpp: lex.l
	touch lex.cpp; lex -o lex.cpp lex.l

lex.o: lex.cpp
	g++ `$(LLVM_BIN) --cxxflags` -c lex.cpp -o lex.o $(LLVM_INC)

lib.o: lib.cpp
	g++ `$(LLVM_BIN) --cxxflags` -shared -c lib.cpp -o lib.o

node.o: node.h node.cpp
	g++ `$(LLVM_BIN) --cxxflags` -c node.cpp -o node.o $(LLVM_INC)

codeGenVisitor.o: codeGenVisitor.h codeGenVisitor.cpp
	g++ `$(LLVM_BIN) --cxxflags` -c codeGenVisitor.cpp -o codeGenVisitor.o $(LLVM_INC)

main.o: main.cpp
	g++ `$(LLVM_BIN) --cxxflags` -c main.cpp -o main.o $(LLVM_INC) `$(LLVM_BIN) --cxxflags`

.gc_built_marker:
	touch .gc_built_marker;cd ./gc;./configure --prefix=/usr/local/ --enable-threads=posix --enable-parallel-mark --enable-cplusplus --enable-gc-assertions;make

CTest: .gc_built_marker
	make -C ./Bench/C++

.llvm_built_marker:
	touch .llvm_built_marker;mkdir ./llvm/build;cd ./llvm/build;../configure --enable-jit --enable-debug --enable-optimized --enable-shared --enable-targets=x86,x86_64;make;

clean:
	rm -f lex.cpp parser.cpp *.o fork_log parser parser.hpp *.output;
	rm -f parser.tab.c .llvm_built_marker .gc_built_marker;
	make -C ./gc clean
	make -C ./Bench/C++ clean
	rm -rf ./llvm/build

