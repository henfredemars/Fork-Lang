#include <iostream>
#include "node.h"
#include "stdio.h"

extern Block *program;
extern int yyparse();
extern FILE* yyin;

int main(int argc, char **argv) {
	GC_INIT();
	if(argc == 2) {
		yyin = fopen(argv[1], "r");
		if(yyin) {
			yyparse();
			fclose(yyin);
		}
		else {
			std::cout << "Error, failed to open file: " << argv[1] << "\n";
		}
	}
	else if(argc > 2) {
		std::cout << "Error, too many inputs.\n";
	}
	else {
		std::cout << "Error, need file input.\n";
	}
	return 0;
}