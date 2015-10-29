
//Implementations of the AST objects
//Much of the logic of creating these comes from bison grammar

#include "node.h"

//Symbol table
SymbolTable sym_table;

//Copy identifier
char* dup_char(char* name) {
  size_t length = strlen(name)+1;
  char* ident = (char*)GC_MALLOC_ATOMIC(length);
  memcpy(ident,name,length);
  return ident;
}

/*===================================Node===================================*/
void Node::describe() const {
	printf("---Found generic node object with no fields.");
	printf("---This SHOULD BE AN ERROR.");
}

llvm::Value* Node::acceptVisitor(Visitor* v) {
	return v->visitNode(this);
}

/*================================Expression================================*/
void Expression::describe() const {
	printf("---Found generic Expression object with no fields\n");
}

bool Expression::identsDeclared() const {
        printf("Attempt to check idents on generic expression\n");
        assert(false);
}

llvm::Value* Expression::acceptVisitor(Visitor* v) {
	return v->visitExpression(this);
}

/*================================Statement=================================*/
void Statement::describe() const {
	printf("---Found generic statement object with no fields\n");
}

llvm::Value* Statement::acceptVisitor(Visitor* v) {
	return v->visitStatement(this);
}

/*=================================Integer==================================*/
Integer::Integer(int64_t value) {
	this->value = value;
}

bool Integer::identsDeclared() const {
	return true;
}

void Integer::describe() const {
	printf("---Found Literal Integer: %i\n", (int)value);
}

llvm::Value* Integer::acceptVisitor(Visitor* v) {
	return v->visitInteger(this);
}

/*==================================Float===================================*/
Float::Float(double value) {
	this->value = value;
}

bool Float::identsDeclared() const {
	return true;
}

void Float::describe() const {
	printf("---Found Float: %f\n", value);
}

llvm::Value* Float::acceptVisitor(Visitor* v) {
	return v->visitFloat(this);
}

/*================================Identifier================================*/
Identifier::Identifier(char* name) {
	this->name = dup_char(name);
}

bool Identifier::identsDeclared() const {
	return declaredAtAll();
}

void Identifier::describe() const {
	printf("---Found Identifier: %s\n",name);
}

bool Identifier::declaredAsVar() const {
	if (!sym_table.check(name)) {
                yyerror("Identifier does not exist in symbol table");
                return false;
        } else if (!sym_table.check(name,VARIABLE)) {
                yyerror("Identifier was not a variable");
		return false;
        }
	return true;
}

bool Identifier::declaredAsFunc() const {
	if (!sym_table.check(name)) {
                yyerror("Identifier does not exist in symbol table");
                return false;
        } else if (!sym_table.check(name,FUNCTION)) {
                yyerror("Identifier was not a function");
		return false;
        }
	return true;
}

bool Identifier::declaredAtAll() const {
	if (!sym_table.check(name)) {
                yyerror("Identifier does not exist in symbol table");
                return false;
        }
	return true;
}

llvm::Value* Identifier::acceptVisitor(Visitor* v) {
	return v->visitIdentifier(this);
}

/*=============================NullaryOperator==============================*/
NullaryOperator::NullaryOperator(char* op) {
	this->op = dup_char(op);
}

bool NullaryOperator::identsDeclared() const {
	return true;
}

void NullaryOperator::describe() const {
	printf("---Found Nullary Operator\n");
}

llvm::Value* NullaryOperator::acceptVisitor(Visitor* v) {
	return v->visitNullaryOperator(this);
}

/*==============================UnaryOperator===============================*/
UnaryOperator::UnaryOperator(char* op, Expression* exp) {
        printf("UO: %s\n",op);
	this->op = dup_char(op);
	this->exp = exp;
}

bool UnaryOperator::identsDeclared() const {
	return exp->identsDeclared();
}

void UnaryOperator::describe() const {
	printf("---Found Unary Operator\n");
}

llvm::Value* UnaryOperator::acceptVisitor(Visitor* v) {
	return v->visitUnaryOperator(this);
}

/*==============================BinaryOperator==============================*/
BinaryOperator::BinaryOperator(Expression* left, char* op, Expression* right) {
    this->op = dup_char(op);
	this->left = left;
	this->right = right;
}

bool BinaryOperator::identsDeclared() const {
	return (left->identsDeclared() && right->identsDeclared());
}

void BinaryOperator::describe() const {
	printf("---Found Binary Operator %s\n",this->op);
}

llvm::Value* BinaryOperator::acceptVisitor(Visitor* v) {
	return v->visitBinaryOperator(this);
}

/*==================================Block===================================*/
Block::Block(std::vector<Statement*, gc_allocator<Statement*>>* statements) {
	this->statements = statements;
}

bool Block::identsDeclared() const {
	printf("Attempt to check idents on Block\n");
	assert(false);
	return false;
}

void Block::describe() const {
	printf("---Found Block\n");
}

Block::Block() {
	this->statements = nullptr;
}

llvm::Value* Block::acceptVisitor(Visitor* v) {
	return v->visitBlock(this);
}

/*===============================FunctionCall===============================*/
FunctionCall::FunctionCall(Identifier* ident, std::vector<Expression*, gc_allocator<Expression*>>* args) {
	this->ident = ident;
	this->args = args;
        if (!sym_table.check(ident->name)) {
		yyerror("Called function does not exist in symbol table");
	} else if (!sym_table.check(ident->name,FUNCTION)) {
		yyerror("Identifier was not a function");
	}
}

bool FunctionCall::identsDeclared() const {
	return ident->declaredAsFunc();
}

void FunctionCall::describe() const {
	printf("---Found Function Call: %s\n",ident->name);
}

llvm::Value* FunctionCall::acceptVisitor(Visitor* v) {
	return v->visitFunctionCall(this);
}

/*=================================Keyword==================================*/
Keyword::Keyword(char* name) {
	this->name = dup_char(name);
}

void Keyword::describe() const {
	printf("---Found Keyword: %s\n",name);
}

llvm::Value* Keyword::acceptVisitor(Visitor* v) {
	return v->visitKeyword(this);
}

/*============================VariableDefinition============================*/
VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp,
	bool isPointer) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
	this->hasPointerType = isPointer;
        sym_table.insert(ident->name,VARIABLE);
}

void VariableDefinition::describe() const {
	if (hasPointerType) {
		printf("---Found Variable Declaration: type='%s*' identifier='%s'\n",
			type->name,ident->name);
	} else {
		printf("---Found Variable Declaration: type='%s' identifier='%s'\n",
                        type->name,ident->name);
	}
}

llvm::Value* VariableDefinition::acceptVisitor(Visitor* v) {
	return v->visitVariableDefinition(this);
}

/*===========================StructureDefinition============================*/
StructureDefinition::StructureDefinition(Identifier* ident,Block* block) {
	this->ident = ident;
	this->block = block;
        sym_table.insert(ident->name,STRUCTURE);
}

void StructureDefinition::describe() const {
	printf("---Found Structure Definition: %s\n",ident->name);
}

llvm::Value* StructureDefinition::acceptVisitor(Visitor* v) {
	return v->visitStructureDefinition(this);
}


/*============================FunctionDefinition============================*/
FunctionDefinition::FunctionDefinition(Keyword* type, Identifier* ident,
	std::vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	Block* block, bool hasPointerType) {
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->block = block;
	this->hasPointerType = hasPointerType;
        sym_table.insert(ident->name,FUNCTION);
}

void FunctionDefinition::describe() const {
	printf("---Found Function Definition: %s\n",ident->name);
}

llvm::Value* FunctionDefinition::acceptVisitor(Visitor* v) {
	return v->visitFunctionDefinition(this);
}

/*==========================StructureDeclaration============================*/
StructureDeclaration::StructureDeclaration(Identifier* type,Identifier* ident) {
	this->type = type;
	this->ident = ident;
        if (!sym_table.check(type->name)) {
                yyerror("Structure definition does not exist in symbol table");
        } else if (!sym_table.check(type->name,STRUCTURE)) {
                yyerror("Type was not a declared structure type");
        }

}

void StructureDeclaration::describe() const {
	printf("---Found Structure Declaration: type='%s' identifier='%s'\n",
		type->name,ident->name);
}

llvm::Value* StructureDeclaration::acceptVisitor(Visitor* v) {
	return v->visitStructureDeclaration(this);
}

/*===========================ExpressionStatement============================*/
ExpressionStatement::ExpressionStatement(Expression* exp) {
	this->exp = exp;
}

void ExpressionStatement::describe() const {
	printf("---Expression(s) converted into statements\n");
}

llvm::Value* ExpressionStatement::acceptVisitor(Visitor* v) {
	return v->visitExpressionStatement(this);
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

llvm::Value* ReturnStatement::acceptVisitor(Visitor* v) {
	return v->visitReturnStatement(this);
}

/*=============================AssignStatement==============================*/
AssignStatement::AssignStatement(ReferenceExpression* target,Expression* valxp) {
	this->target = target;
	this->valxp = valxp;
}

void AssignStatement::describe() const {
	printf("---Found Assignment Statement\n");
}

llvm::Value* AssignStatement::acceptVisitor(Visitor* v) {
	return v->visitAssignStatement(this);
}

/*===============================IfStatement================================*/
IfStatement::IfStatement(Expression* exp,Block* block) {
	this->exp = exp;
	this->block = block;
}

void IfStatement::describe() const {
	printf("---Found If Statement\n");
}

llvm::Value* IfStatement::acceptVisitor(Visitor* v) {
	return v->visitIfStatement(this);
}

/*===============================SymbolTable================================*/
SymbolTable::SymbolTable() {
	//Push global scope
	const char* print_int = "print_int";
	const char* print_float = "print_float";
        const char* malloc_int = "malloc_int";
	const char* malloc_float = "malloc_float";
	this->push();
	this->insert(print_int,FUNCTION);
	this->insert(print_float,FUNCTION);
	this->insert(malloc_int,FUNCTION);
	this->insert(malloc_float,FUNCTION);
}

void SymbolTable::insert(const char* ident,IdentType type) {
	assert(frames.size());
	std::map<std::string,IdentType>& lframe = frames.back();
        if (this->check(ident))
          yyerror("Identifier already exists");
        lframe.insert(std::make_pair(std::string(ident),type));
}

bool SymbolTable::check(const char* ident) {
        assert(frames.size());
        for (int i = 0; i<frames.size();i++) {
          if (frames.at(i).count(std::string(ident))) {
	    return true;
          }
        }
        return false;
}

bool SymbolTable::check(const char* ident,IdentType type) {
        assert(frames.size());
        for (int i = 0; i<frames.size();i++) {
          if (frames.at(i).count(std::string(ident))) {
            if (frames.at(i).at(std::string(ident))==type) {
              return true;
            }
          }
        }
        return false;
}

void SymbolTable::push() {
	//Push new scope
	this->frames.push_back(std::map<std::string,IdentType>());
}

void SymbolTable::pop() {
	//Pop current scope
	assert(frames.size());
	frames.pop_back();
}

/*===============================LeftExpression================================*/
ReferenceExpression::ReferenceExpression(Identifier* ident,Expression* offsetExpression,
	bool hasPointerType, bool addressOfThis) {
	this->ident = ident;
	this->offsetExpression = offsetExpression;
	this->hasPointerType = hasPointerType;
	this->addressOfThis = addressOfThis;
}

bool ReferenceExpression::usesDirectValue() const {
	return (offsetExpression == nullptr);
}

bool ReferenceExpression::identsDeclared() const {
	return (ident->declaredAsVar() && (!offsetExpression || offsetExpression->identsDeclared()));
}

void ReferenceExpression::describe() const {
	printf("---Found ReferenceExpression for ident: %s\n",ident->name);
}

llvm::Value* ReferenceExpression::acceptVisitor(Visitor* v) {
	return v->visitReferenceExpression(this);
}

