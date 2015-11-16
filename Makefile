LLVM_INC := -I./llvm/include -I./llvm/build/include -I./llvm/examples/Kaleidoscope/include
LLVM_BIN := ./llvm/build/Release+Asserts/bin/llvm-config
OPT_LVL := -O1 -flto -Wall -Wno-unused -Wno-format-security

#export REQUIRES_RTTI = 1

all: parser CTest

parser: .gc_built_marker .llvm_built_marker parser.o lex.o node.o codeGenVisitor.o statementVisitor.o main.o parser.hpp lib.so .bcleanup_marker
	g++ -Wl,-rpath=./llvm/build/Release+Asserts/lib -Wl,-rpath=./gc/.libs `$(LLVM_BIN) --cxxflags --ldflags` -Wl,-rpath=. -o parser parser.o lex.o node.o codeGenVisitor.o statementVisitor.o main.o -L./gc/.libs -lpthread -ltinfo `$(LLVM_BIN) --system-libs` -lLLVM-3.8svn -lgc -l :lib.so
#`$(LLVM_BIN) --libfiles`

parser.cpp: parser.y node.h
	touch parser.cpp; bison -d -o parser.cpp parser.y

parser.o: parser.cpp
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -c parser.cpp -o parser.o $(LLVM_INC)

lex.cpp: lex.l node.h
	touch lex.cpp; lex -o lex.cpp lex.l

lex.o: lex.cpp
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -c lex.cpp -o lex.o $(LLVM_INC)

lib.so: lib.o parContextManager.o
	g++ -shared -o lib.so lib.o parContextManager.o

lib.o: lib.cpp lib.h parContextManager.h
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -fno-lto -fPIC -c lib.cpp -o lib.o

parContextManager.o: parContextManager.cpp parContextManager.h
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -fPIC -c parContextManager.cpp -o parContextManager.o

node.o: node.h node.cpp
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -c node.cpp -o node.o $(LLVM_INC)

codeGenVisitor.o: codeGenVisitor.h codeGenVisitor.cpp node.h
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -c codeGenVisitor.cpp -o codeGenVisitor.o $(LLVM_INC)

statementVisitor.o: statementVisitor.h statementVisitor.cpp node.h
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -c statementVisitor.cpp -o statementVisitor.o $(LLVM_INC)

main.o: main.cpp node.h
	g++ `$(LLVM_BIN) --cxxflags` $(OPT_LVL) -c main.cpp -o main.o $(LLVM_INC) `$(LLVM_BIN) --cxxflags`

.gc_built_marker:
	touch .gc_built_marker;cd ./gc;./configure --prefix=/usr/local/ --enable-threads=posix --enable-parallel-mark --enable-cplusplus --enable-gc-assertions;make

CTest: .gc_built_marker
	make -C ./Bench/C++

.llvm_built_marker:
	touch .llvm_built_marker;mkdir ./llvm/build;cd ./llvm/build;../configure --enable-jit --enable-optimized --enable-shared --enable-targets=x86,x86_64;make;

.bcleanup_marker:
	touch .bcleanup_marker; rm -rf ./llvm/build/*.o ./llvm/build/*.lo ./llvm/build/*.a ./gc/*.o ./gc/*.lo ./gc/*.a ./gc/.libs/*.a ./gc/.libs/*.la ./gc/.libs/*.o

clean:
	rm -f lex.cpp parser.cpp *.o fork_log parser parser.hpp *.output *.so;
	rm -f parser.tab.c ./Testing/Programs/*.o ./Testing/Programs/*.bin ./Testing/Programs/*.s ./Testing/Programs/*.ll

distclean:
	rm -f lex.cpp parser.cpp *.o fork_log parser parser.hpp *.output *.so;
	rm -f parser.tab.c .llvm_built_marker .gc_built_marker .bcleanup_marker;
	rm -f ./Testing/Programs/*.o ./Testing/Programs/*.bin ./Testing/Programs/*.s /Testing/Programs/*.ll
	make -C ./gc clean
	make -C ./Bench/C++ clean
	rm -rf ./llvm/build

