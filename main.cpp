#include <iostream>
#include "stdio.h"
#include "node.h"
#include "gc/include/gc.h"

extern int yyparse();
extern FILE* yyin;

#ifdef YYDEBUG
extern int yydebug;
#endif

Block* ast_root = NULL;

int main(int argc, char **argv) {
	GC_INIT();
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
	free_int(malloc_int(1)); //Force lib.so linkage

	#ifdef YYDEBUG
	yydebug = 1;
	#endif

	if(argc == 2) {
		yyin = fopen(argv[1], "r");
		if(yyin) {
			yyparse(); //ast_root points to program
			fclose(yyin);
			yyin = NULL;
			CodeGenVisitor c("LLVM Compiler Backend");
			if (ast_root) {
			  ast_root->acceptVisitor(&c);
			  c.printModule();
			  c.executeMain();
			} else {
			  printf("Parser closed without returning an AST\n");
			}
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
