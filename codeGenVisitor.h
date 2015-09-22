class Visitor {
public:
	virtual Value* visitExpression();
	virtual Value* visitStatement();
	virtual Value* visitInteger();
	virtual Value* visitFloat();
	virtual Value* visitIdentifier();
	virtual Value* visitNullaryOperator();
	virtual Value* visitUnaryOperator();
	virtual Value* visitBinaryOperator();
	virtual Value* visitAssignment();
	virtual Value* visitBlock();
	virtual Value* visitFunctionCall();
	virtual Value* visitKeyword();
	virtual Value* visitVariableDefinition();
	virtual Value* visitStructureDefinition();
	virtual Value* visitFunctionDefinition();
	virtual Value* visitStructureDeclaration();
	virtual Value* visitExpressionStatement();
	virtual Value* visitReturnStatement();
	virtual Value* visitAssignStatement();
	virtual Value* visitIfStatement();
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