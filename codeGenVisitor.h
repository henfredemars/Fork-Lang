class Visitor {
public:
	virtual Value* visitExpression(Expression e);
	virtual Value* visitStatement(Statement s);
	virtual Value* visitInteger(Integer i);
	virtual Value* visitFloat(Float f);
	virtual Value* visitIdentifier(Identifier i);
	virtual Value* visitNullaryOperator(NullaryOperator n);
	virtual Value* visitUnaryOperator(UnaryOperator u);
	virtual Value* visitBinaryOperator(BinaryOperator b);
	virtual Value* visitAssignment(Assignment a);
	virtual Value* visitBlock(Block b);
	virtual Value* visitFunctionCall(FunctionCall f);
	virtual Value* visitKeyword(Keyword k);
	virtual Value* visitVariableDefinition(VariableDefinition v);
	virtual Value* visitStructureDefinition(StructureDefinition s);
	virtual Value* visitFunctionDefinition(FunctionDefinition f);
	virtual Value* visitStructureDeclaration(StructureDeclaration s);
	virtual Value* visitExpressionStatement(ExpressionStatement e);
	virtual Value* visitReturnStatement(ReturnStatement r);
	virtual Value* visitAssignStatement(AssignStatement a);
	virtual Value* visitIfStatement(IfStatement i);
};

class CodeGenVisitor : public Visitor {
public:
	Value* visitExpression(Expression e);
	Value* visitStatement(Statement s);
	Value* visitInteger(Integer i);
	Value* visitFloat(Float f);
	Value* visitIdentifier(Identifier i);
	Value* visitNullaryOperator(NullaryOperator n);
	Value* visitUnaryOperator(UnaryOperator u);
	Value* visitBinaryOperator(BinaryOperator b);
	Value* visitAssignment(Assignment a);
	Value* visitBlock(Block b);
	Value* visitFunctionCall(FunctionCall f);
	Value* visitKeyword(Keyword k);
	Value* visitVariableDefinition(VariableDefinition v);
	Value* visitStructureDefinition(StructureDefinition s);
	Value* visitFunctionDefinition(FunctionDefinition f);
	Value* visitStructureDeclaration(StructureDeclaration s);
	Value* visitExpressionStatement(ExpressionStatement e);
	Value* visitReturnStatement(ReturnStatement r);
	Value* visitAssignStatement(AssignStatement a);
	Value* visitIfStatement(IfStatement i);
};