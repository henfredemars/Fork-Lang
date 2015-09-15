all: parser CTest

parser: parser.o lex.o node.o main.o parser.hpp
	g++ -std=c++11 -o parser parser.o lex.o node.o main.o -lgc

parser.cpp: parser.y
	bison -d -o parser.cpp parser.y

parser.o: parser.cpp
	g++ -std=c++11 -c parser.cpp -o parser.o 

lex.cpp: lex.l
	lex -o lex.cpp lex.l

lex.o: lex.cpp
	g++ -std=c++11 -c lex.cpp -o lex.o

node.o: node.h node.cpp
	g++ -std=c++11 -c node.cpp -o node.o 

main.o: main.cpp
	g++ -std=c++11 -c main.cpp -o main.o

CTest:
	make -C ./Bench/C++

log: parser.o lex.o main.o parser.hpp
	g++ -std=c++11 -o parser parser.o lex.o node.o main.o -lgc > fork_log 2>&1

clean:
	rm -f lex.cpp parser.cpp *.o fork_log parser parser.hpp *.output;
	rm -f parser.tab.c;
	make -C ./Bench/C++ clean
