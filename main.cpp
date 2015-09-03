#include <iostream>
#include "node.h"
extern Block *program;
extern int yyparse();

int main(int argc, char **argv) {
	GC_INIT();
	yyparse();
	std::cout << program << std::endl;
	return 0;
}