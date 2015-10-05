#include <iostream>
#include "stdio.h"
#include "node.h"
#include "gc/include/gc.h"

extern int yyparse();
extern int yydebug;
extern FILE* yyin;

Node* ast_root = NULL;

int main(int argc, char **argv) {
	GC_INIT();
	yydebug = 1;
	if(argc == 2) {
		yyin = fopen(argv[1], "r");
		if(yyin) {
			yyparse(); //ast_root points to program
			fclose(yyin);
			yyin = NULL;
			CodeGenVisitor c;
			c.initModule("LLVM JIT start");
			//((Block)ast_root)->accept(c);
		}
		else {
			cout << "Error, failed to open file: " << argv[1] << "\n";
		}
	}
	else if(argc > 2) {
		cout << "Error, too many inputs.\n";
	}
	else {
		cout << "Error, need file input.\n";
	}
	return 0;
}
