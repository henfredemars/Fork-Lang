
//Implementations of the AST objects
//Much of the logic of creating these comes from bison grammar

#include "node.h"


void Node::describe() {
	printf("Found generic node object with no fields\n");
}

void Expression::describe() {
	printf("Found generic Expression object with no fields\n");
}

void Statement::describe() {
	printf("Found generic statement object with no fields\n");
}

Integer::Integer(int64_t value) {
	this->value = value;
}

void Integer::describe() {
	printf("Found Integer: %i\n", (int)value);
}

Float::Float(double value) {
	this->value = value;
}

void Float::describe() {
	printf("Found Float: %f\n", value);
}

Identifier::Identifier(char* name) {
	size_t length = strlen(name);
	this->name = (char*)GC_MALLOC(length+1);
	memcpy(this->name,name,length);
	//It's good practice to keep our own copy
}

void Identifier::describe() {
	printf("Found Identifier: %s\n",name);
}

NullaryOperator::NullaryOperator(int64_t op) {
	this->op = op;
}

void NullaryOperator::describe() {
	printf("Found Nullary Operator\n");
}

UnaryOperator::UnaryOperator(int64_t op, Expression* exp) {
	this->op = op;
	this->exp = exp;
}

void UnaryOperator::describe() {
	printf("Found Unary Operator\n");
}

BinaryOperator::BinaryOperator(Expression* left, int64_t op, Expression* right) {
	this->op = op;
	this->left = left;
	this->right = right;
}

void BinaryOperator::describe() {
	printf("Found Binary Operator %d\n",(int)this->op);
}

Assignment::Assignment(Identifier* left, Expression* right) {
	this->left = left;
	this->right = right;
}

void Assignment::describe() {
	printf("Found Assignment: %s\n",left->name);
}

Block::Block(vector<Statement*, gc_allocator<Statement*>>* statements) {
	this->statements = statements;
}

void Block::describe() {
	printf("Found Block\n");
}

Block::Block() {
	this->statements = NULL;
}

FunctionCall::FunctionCall(Identifier* ident, vector<Expression*, gc_allocator<Expression*>>* args) {
	this->ident = ident;
	this->args = args;
}

void FunctionCall::describe() {
	printf("Found Function Call: %s\n",ident->name);
}

Keyword::Keyword(char* name) {
	size_t length = strlen(name);
	this->name = (char*)GC_MALLOC(length+1);
	memcpy(this->name,name,length);
}

void Keyword::describe() {
	printf("Found Keyword: %s\n",name);
}

VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
}

void VariableDefinition::describe() {
	printf("Found Variable Declaration: %s\n",type->name);
}

FunctionDefinition::FunctionDefinition(Keyword* type, Identifier* ident, vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	 Block* block) {
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->block = block;
}

void FunctionDefinition::describe() {
	printf("Found Function Definition: %s\n",ident->name);
}

ExpressionStatement::ExpressionStatement(Expression* exp) {
	this->exp = exp;
}

void ExpressionStatement::describe() {
	printf("Expression(s) converted into statements.\n");
}
