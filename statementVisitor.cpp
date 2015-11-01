#include "statementVisitor.h"

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

