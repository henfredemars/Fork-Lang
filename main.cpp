#include <iostream>
#include "stdio.h"
#include "node.h"
#include "gc/include/gc.h"

extern int yyparse();
extern int yydebug;
extern FILE* yyin;

Block* ast_root = NULL;

int main(int argc, char **argv) {
	GC_INIT();
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
	yydebug = 1;
	if(argc == 2) {
		yyin = fopen(argv[1], "r");
		if(yyin) {
			yyparse(); //ast_root points to program
			fclose(yyin);
			yyin = NULL;
			CodeGenVisitor c("LLVM Compiler Backend");
			ast_root->acceptVisitor(&c);
			c.printModule();
			c.executeMain();
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
