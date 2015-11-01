#include "node.h"

//Visitor for statements that extracts the variable defintions from a block

#ifndef __STATEMENT_VISITOR_H
#define __STATEMENT_VISITOR_H

class StatementVisitor : public gc {
public:
	StatementVisitor();
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> varList;
	void visitBlock(Block* b);
	void visitStatement(Statement* s);
	void visitVariableDefinition(VariableDefinition* v);
};

#endif /* __STATEMENT_VISITOR_H */
