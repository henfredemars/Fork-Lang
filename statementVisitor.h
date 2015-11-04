#include "node.h"

//Visitor for statements that extracts the variable defintions from a block

#ifndef __STATEMENT_VISITOR_H
#define __STATEMENT_VISITOR_H

class StatementVisitor : public gc {
public:
	StatementVisitor();
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> varList;
	std::vector<StructureDeclaration*,gc_allocator<StructureDeclaration*>> structList;
	void visitBlock(Block* b);
	void visitStatement(Statement* s);
	void visitVariableDefinition(VariableDefinition* v);
	void visitStructureDefinition(StructureDefinition* s);
	void visitStructureDeclaration(StructureDeclaration* s);
};

#endif /* __STATEMENT_VISITOR_H */
