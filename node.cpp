
//Implementations of the AST objects
//Much of the logic of creating these comes from bison grammar

#include "node.h"

/*===================================Node===================================*/
void Node::describe() const {
	printf("---Found generic node object with no fields.");
	printf("---This SHOULD BE AN ERROR.");
}

/*================================Expression================================*/
void Expression::describe() const {
	printf("---Found generic Expression object with no fields\n");
}

llvm::Value* Expression::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitExpression(this);
}

/*================================Statement=================================*/
void Statement::describe() const {
	printf("---Found generic statement object with no fields\n");
}

llvm::Value* Statement::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitStatement(this);
}

/*=================================Integer==================================*/
Integer::Integer(int64_t value) {
	this->value = value;
}

void Integer::describe() const {
	printf("---Found Literal Integer: %i\n", (int)value);
}

llvm::Value* Integer::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitInteger(this);
}

/*==================================Float===================================*/
Float::Float(double value) {
	this->value = value;
}

void Float::describe() const {
	printf("---Found Float: %f\n", value);
}

llvm::Value* Float::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitFloat(this);
}

/*================================Identifier================================*/
Identifier::Identifier(char* name) {
	size_t length = strlen(name)+1;
	this->name = (char*)GC_MALLOC_ATOMIC(length);
	memcpy(this->name,name,length);
	//Its good practice to keep our own copy
}

void Identifier::describe() const {
	printf("---Found Identifier: %s\n",name);
}

llvm::Value* Identifier::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitIdentifier(this);
}

/*=============================NullaryOperator==============================*/
NullaryOperator::NullaryOperator(char* op) {
	size_t length = strlen(op)+1;
        this->op = (char*)GC_MALLOC_ATOMIC(length);
        memcpy(this->op,op,length);
}

void NullaryOperator::describe() const {
	printf("---Found Nullary Operator\n");
}

llvm::Value* NullaryOperator::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitNullaryOperator(this);
}

/*==============================UnaryOperator===============================*/
UnaryOperator::UnaryOperator(char* op, Expression* exp) {
        size_t length = strlen(op)+1;
        this->op = (char*)GC_MALLOC_ATOMIC(length);
        memcpy(this->op,op,length);
	this->exp = exp;
}

void UnaryOperator::describe() const {
	printf("---Found Unary Operator\n");
}

llvm::Value* UnaryOperator::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitUnaryOperator(this);
}

/*==============================BinaryOperator==============================*/
BinaryOperator::BinaryOperator(Expression* left, char* op, Expression* right) {
	size_t length = strlen(op)+1;
        this->op = (char*)GC_MALLOC_ATOMIC(length);
        memcpy(this->op,op,length);
	this->left = left;
	this->right = right;
}

void BinaryOperator::describe() const {
	printf("---Found Binary Operator %s\n",this->op);
}

llvm::Value* BinaryOperator::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitBinaryOperator(this);
}

/*================================Assignment================================*/
Assignment::Assignment(Identifier* left, Expression* right) {
	this->left = left;
	this->right = right;
}

void Assignment::describe() const {
	printf("---Found Assignment: %s\n",left->name);
}

llvm::Value* Assignment::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitAssignment(this);
}

/*==================================Block===================================*/
Block::Block(vector<Statement*, gc_allocator<Statement*>>* statements) {
	this->statements = statements;
}

void Block::describe() const {
	printf("---Found Block\n");
}

Block::Block() {
	this->statements = NULL;
}

llvm::Value* Block::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitBlock(this);
}

/*===============================FunctionCall===============================*/
FunctionCall::FunctionCall(Identifier* ident, vector<Expression*, gc_allocator<Expression*>>* args) {
	this->ident = ident;
	this->args = args;
}

void FunctionCall::describe() const {
	printf("---Found Function Call: %s\n",ident->name);
}

llvm::Value* FunctionCall::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitFunctionCall(this);
}

/*=================================Keyword==================================*/
Keyword::Keyword(char* name) {
	size_t length = strlen(name)+1;
	this->name = (char*)GC_MALLOC_ATOMIC(length);
	memcpy(this->name,name,length);
}

void Keyword::describe() const {
	printf("---Found Keyword: %s\n",name);
}

llvm::Value* Keyword::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitKeyword(this);
}

/*============================VariableDefinition============================*/
VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
}

void VariableDefinition::describe() const {
	printf("---Found Variable Declaration: type='%s' identifier='%s'\n",
		type->name,ident->name);
}

llvm::Value* VariableDefinition::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitVariableDefinition(this);
}

/*===========================StructureDefinition============================*/
StructureDefinition::StructureDefinition(Identifier* ident,Block* block) {
	this->ident = ident;
	this->block = block;
}

void StructureDefinition::describe() const {
	printf("---Found Structure Definition: %s\n",ident->name);
}

llvm::Value* StructureDefinition::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitStructureDefinition(this);
}


/*============================FunctionDefinition============================*/
FunctionDefinition::FunctionDefinition(Keyword* type, Identifier* ident,
	 vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	 Block* block) {
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->block = block;
}

void FunctionDefinition::describe() const {
	printf("---Found Function Definition: %s\n",ident->name);
}

llvm::Value* FunctionDefinition::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitFunctionDefinition(this);
}

/*==========================StructureDeclaration============================*/
StructureDeclaration::StructureDeclaration(Identifier* type,Identifier* ident) {
	this->type = type;
	this->ident = ident;
}

void StructureDeclaration::describe() const {
	printf("---Found Structure Declaration: type='%s' identifier='%s'\n",
		type->name,ident->name);
}

llvm::Value* StructureDeclaration::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitStructureDeclaration(this);
}

/*===========================ExpressionStatement============================*/
ExpressionStatement::ExpressionStatement(Expression* exp) {
	this->exp = exp;
}

void ExpressionStatement::describe() const {
	printf("---Expression(s) converted into statements\n");
}

llvm::Value* ExpressionStatement::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitExpressionStatement(this);
}

/*=============================ReturnStatement==============================*/
ReturnStatement::ReturnStatement(Expression* exp) {
	this->exp = exp;
}

void ReturnStatement::describe() const {
	if (exp) {
	  printf("---Found return statement with expression\n");
	} else {
	  printf("---Found return statement, statement returns void\n");
	}
}

llvm::Value* ReturnStatement::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitReturnStatement(this);
}

/*=============================AssignStatement==============================*/
AssignStatement::AssignStatement(Expression* target,Expression* valxp) {
	this->target = target;
	this->valxp = valxp;
}

void AssignStatement::describe() const {
	printf("---Found Assignment Statement\n");
}

llvm::Value* AssignStatement::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitAssignStatement(this);
}

/*===============================IfStatement================================*/
IfStatement::IfStatement(Expression* exp,Block* block) {
	this->exp = exp;
	this->block = block;
}

void IfStatement::describe() const {
	printf("---Found If Statement\n");
}

llvm::Value* IfStatement::acceptCodeGenVisitor(CodeGenVisitor c) {
	return c.visitIfStatement(this);
}
