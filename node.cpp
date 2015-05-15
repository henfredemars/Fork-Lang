
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



