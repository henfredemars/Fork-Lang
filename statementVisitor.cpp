#include "statementVisitor.h"
#include <iostream>

StatementVisitor::StatementVisitor() {

}

void StatementVisitor::visitBlock(Block* b) {
        auto start = b->statements->begin();
        auto end = b->statements->end();
        for (auto iter=start; iter!=end; iter++) {
          Statement* s = *iter;
          s->acceptVisitor(this);
        }
}

void StatementVisitor::visitStatement(Statement* s) {
        return;
}

void StatementVisitor::visitVariableDefinition(VariableDefinition* v) {
    varList.push_back(v);
}

void StatementVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	std::cout << "Found struct" << std::endl;
	structList.push_back(s);
}

void StatementVisitor::visitStructureDefinition(StructureDefinition* s) {
	return;
}

