all: CTest

CTest: CTest.o Node.o util.o
	g++ -std=c++11 -o CTest CTest.o Node.o util.o -lgc

CTest.o: CTest.cpp
	g++ -std=c++11 -c CTest.cpp -o CTest.o

Node.o: Node.cpp
	g++ -std=c++11 -c Node.cpp -o Node.o

util.o: util.cpp
	g++ -std=c++11 -c util.cpp -o util.o

clean:
	rm -f *.o; rm -f CTest
