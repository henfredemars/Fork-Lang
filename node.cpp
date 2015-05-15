
//Implementations of the AST objects
//Much of the logic of creating these comes from bison grammar

#include "node.h"

Integer::Integer(int64_t value) {
	this->value = value;
}

Float::Float(double value) {
	this->value = value;
}

Identifier::Identifier(char* name) {
	size_t length = strlen(name);
	this->name = (char*)GC_MALLOC(length+1);
	memcopy(this->name,name,length);
	//It's good practice to keep our own copy
}

NullaryOperator::NullaryOperator(int64_t op) {
	this->op = op;
}

UnaryOperator::UnaryOperator(int64_t op, Expression* exp) {
	this->op = op;
	this->exp = exp;
}

BinaryOperator::BinaryOperator(int64_t op, Expression* left, Expression* right) {
	this->op = op;
	this->left = left;
	this->right = right;
}

Assignment::Assignment(Identifier* left, Expression* right) {
	this->left = left;
	this->right = right;
}

Block::Block(vector<Statement*, gc_alloc> statements) {
	this->statements = statements;
}

FunctionCall::FunctionCall(Identifier* ident, vector<Expression*, gc_alloc> args) {
	this->ident = ident;
	this->args = args;
}

Keyword::Keyword(char* name) {
	size_t length = strlen(name);
	this->name = (char*)GC_MALLOC(length+1);
	memcopy(this->name,name,length);
}

VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
}

FunctionDefenition::FunctionDefenition(Keyword* type, Identifier* ident, vector<VariableDefinition*, gc_alloc> args,
	 Block* block) {
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->block = block;
}