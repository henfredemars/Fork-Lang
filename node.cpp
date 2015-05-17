
//Implementations of the AST objects
//Much of the logic of creating these comes from bison grammar

#include "node.h"

Integer::Integer(int64_t value) {
	this->value = value;
}

virtual void Integer::describe() {
	printf("Found Integer: %i\n",value);
}

Float::Float(double value) {
	this->value = value;
}

virtual void Float::describe() {
	printf("Found Float: %f\n",value);
}

Identifier::Identifier(char* name) {
	size_t length = strlen(name);
	this->name = (char*)GC_MALLOC(length+1);
	memcopy(this->name,name,length);
	//It's good practice to keep our own copy
}

virtual void Identifier::describe() {
	printf("Found Identifier: %s\n",name);
}

NullaryOperator::NullaryOperator(int64_t op) {
	this->op = op;
}

virtual void NullaryOperator::describe() {
	printf("Found Nullary Operator\n");
}

UnaryOperator::UnaryOperator(int64_t op, Expression* exp) {
	this->op = op;
	this->exp = exp;
}

virtual void UnaryOperator::describe() {
	printf("Found Unary Operator\n");
}

BinaryOperator::BinaryOperator(int64_t op, Expression* left, Expression* right) {
	this->op = op;
	this->left = left;
	this->right = right;
}

virtual void BinaryOperator::describe() {
	printf("Found Binary Operator\n");
}

Assignment::Assignment(Identifier* left, Expression* right) {
	this->left = left;
	this->right = right;
}

virtual void Assignment::describe() {
	printf("Found Assignment: %s\n",left->name);
}

Block::Block(vector<Statement*, gc_alloc> statements) {
	this->statements = statements;
}

virtual void Block::describe() {
	printf("Found Block\n");
}

Block::Block() {
	this->statements = NULL;
}

FunctionCall::FunctionCall(Identifier* ident, vector<Expression*, gc_alloc> args) {
	this->ident = ident;
	this->args = args;
}

virtual void FunctionCall::describe() {
	printf("Found Function Call: %s\n",ident->name);
}

Keyword::Keyword(char* name) {
	size_t length = strlen(name);
	this->name = (char*)GC_MALLOC(length+1);
	memcopy(this->name,name,length);
}

virtual void Keyword::describe() {
	printf("Found Keyword: %s\n",name);
}

VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
}

virtual void VariableDefinition::describe() {
	printf("Found Variable Declaration: %s\n",type->name);
}

FunctionDefinition::FunctionDefinition(Keyword* type, Identifier* ident, vector<VariableDefinition*, gc_alloc> args,
	 Block* block) {
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->block = block;
}

virtual void FunctionDefinition::describe() {
	printf("Found Function Definition: %s\n",ident->name);
}

ExpressionStatement::ExpressionStatement(Expression* exp) {
	this->exp = exp;
}

virtual void ExpressionStatement::describe() {
	printf("Expression(s) converted into statements.\n");
}