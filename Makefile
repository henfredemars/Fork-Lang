all: parser

parser: parser.o lex.o node.o main.o parser.hpp
	g++ -std=c++11 -lgc -o parser parser.o lex.o node.o main.o

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
	g++ -std=c++11 -lgc -c main.cpp -o main.o

log: parser.o lex.o main.o parser.hpp
	g++ -std=c++11 -lgc -o parser parser.o lex.o node.o main.o > fork_log 2>&1

clean:
	rm -f lex.cpp parser.cpp *.o fork_log