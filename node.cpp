
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

/*================================Expression================================*/
void Expression::describe() const {
	printf("---Found generic Expression object with no fields\n");
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

void Identifier::describe() const {
	printf("---Found Identifier: %s\n",name);
}

bool Identifier::assertDeclared() const {
	if (!sym_table.check(name)) {
                yyerror("Identifier does not exist in symbol table");
                return true;
        } else if (!sym_table.check(name,VARIABLE)) {
                yyerror("Identifier was not a variable");
		return true;
        }
	return false;
}

llvm::Value* Identifier::acceptVisitor(Visitor* v) {
	return v->visitIdentifier(this);
}

/*=============================NullaryOperator==============================*/
NullaryOperator::NullaryOperator(char* op) {
        printf("%s",op);
	this->op = dup_char(op);
}

void NullaryOperator::describe() const {
	printf("---Found Nullary Operator\n");
}

llvm::Value* NullaryOperator::acceptVisitor(Visitor* v) {
	return v->visitNullaryOperator(this);
}

/*==============================UnaryOperator===============================*/
UnaryOperator::UnaryOperator(char* op, Expression* exp) {
	this->op = dup_char(op);
	this->exp = exp;
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

void BinaryOperator::describe() const {
	printf("---Found Binary Operator %s\n",this->op);
}

llvm::Value* BinaryOperator::acceptVisitor(Visitor* v) {
	return v->visitBinaryOperator(this);
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

llvm::Value* Block::acceptVisitor(Visitor* v) {
	return v->visitBlock(this);
}

/*===============================FunctionCall===============================*/
FunctionCall::FunctionCall(Identifier* ident, vector<Expression*, gc_allocator<Expression*>>* args) {
	this->ident = ident;
	this->args = args;
        if (!sym_table.check(ident->name)) {
		yyerror("Called function does not exist in symbol table");
	} else if (!sym_table.check(ident->name,FUNCTION)) {
		yyerror("Identifier was not a function");
	}
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
VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
        sym_table.insert(ident->name,VARIABLE);
}

void VariableDefinition::describe() const {
	printf("---Found Variable Declaration: type='%s' identifier='%s'\n",
		type->name,ident->name);
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
	 vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	 Block* block) {
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->block = block;
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
AssignStatement::AssignStatement(Expression* target,Expression* valxp) {
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
	this->push();
}

void SymbolTable::insert(char* ident,IdentType type) {
	assert(frames.size());
	map<std::string,IdentType>& lframe = frames.back();
        if (this->check(ident))
          yyerror("Identifier already exists");
        lframe.insert(std::make_pair(std::string(ident),type));
}

bool SymbolTable::check(char* ident) {
        assert(frames.size());
        for (int i = 0; i<frames.size();i++) {
          if (frames.at(i).count(std::string(ident))) {
            printf("Found\n");
	    return true;
          }
        }
        return false;
}

bool SymbolTable::check(char* ident,IdentType type) {
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

