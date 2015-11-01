
//Implementations of the AST objects
//Much of the logic of creating these comes from bison grammar

#include "node.h"

//Tables
SymbolTable sym_table;
TypeTable user_type_table;

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

llvm::Value* Node::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* Expression::acceptVisitor(ASTVisitor* v) {
	return v->visitExpression(this);
}

/*================================Statement=================================*/
void Statement::describe() const {
	printf("---Found generic statement object with no fields\n");
}

llvm::Value* Statement::acceptVisitor(ASTVisitor* v) {
	return v->visitStatement(this);
}

void Statement::acceptVisitor(StatementVisitor* v) {
	v->visitStatement(this);
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

llvm::Value* Integer::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* Float::acceptVisitor(ASTVisitor* v) {
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
        IdentType type = sym_table.at(name);
	if (type == INVALID) {
                yyerror("Identifier does not exist in symbol table");
                return false;
        } else if (type != VARIABLE) {
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

llvm::Value* Identifier::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* NullaryOperator::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* UnaryOperator::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* BinaryOperator::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* Block::acceptVisitor(ASTVisitor* v) {
	return v->visitBlock(this);
}

void Block::acceptVisitor(StatementVisitor* v) {
	v->visitBlock(this);
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

llvm::Value* FunctionCall::acceptVisitor(ASTVisitor* v) {
	return v->visitFunctionCall(this);
}

/*=================================Keyword==================================*/
Keyword::Keyword(char* name) {
	this->name = dup_char(name);
}

void Keyword::describe() const {
	printf("---Found Keyword: %s\n",name);
}

llvm::Value* Keyword::acceptVisitor(ASTVisitor* v) {
	return v->visitKeyword(this);
}

/*============================VariableDefinition============================*/
VariableDefinition::VariableDefinition(Keyword* type, Identifier* ident, Expression* exp,
	bool isPointer) {
	this->type = type;
	this->ident = ident;
	this->exp = exp;
	this->hasPointerType = isPointer;
}

void VariableDefinition::insertIntoSymbolTable() {
        sym_table.insert(ident->name,VARIABLE);
}

bool VariableDefinition::alreadyExistsInSymbolTable() const {
	return sym_table.check(ident->name,VARIABLE);
}

bool VariableDefinition::alreadyExistsInLocalSymbolTable() const {
	return sym_table.checkLocal(ident->name,VARIABLE);
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

llvm::Value* VariableDefinition::acceptVisitor(ASTVisitor* v) {
	return v->visitVariableDefinition(this);
}

void VariableDefinition::acceptVisitor(StatementVisitor* v) {
	v->visitVariableDefinition(this);
}

/*===========================StructureDefinition============================*/
StructureDefinition::StructureDefinition(Identifier* ident,Block* block) {
	this->ident = ident;
	this->block = block;
}

std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> StructureDefinition::getVariables() const {
	StatementVisitor* sv = new StatementVisitor();
	block->acceptVisitor(sv);
	return sv->varList;
}

bool StructureDefinition::validate() {
	if (user_type_table.check(ident->name)) {
	  printf("User type already exists!\n");
	}
	user_type_table.insert(ident->name);
}

void StructureDefinition::describe() const {
	printf("---Found Structure Definition: %s\n",ident->name);
}

llvm::Value* StructureDefinition::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* FunctionDefinition::acceptVisitor(ASTVisitor* v) {
	return v->visitFunctionDefinition(this);
}

/*==========================StructureDeclaration============================*/
StructureDeclaration::StructureDeclaration(Identifier* type,Identifier* ident) {
	this->type = type;
	this->ident = ident;
}

bool StructureDeclaration::validate() {
        if (sym_table.check(ident->name)) {
                printf("Variable name already exists in the symbol table\n");
		return false;
        } else if (!user_type_table.check(type->name)) {
                yyerror("Type was not a declared in the structure table");
		return false;
	}
	sym_table.insert(ident->name,VARIABLE);
	return true;
}

void StructureDeclaration::describe() const {
	printf("---Found Structure Declaration: type='%s' identifier='%s'\n",
		type->name,ident->name);
}

llvm::Value* StructureDeclaration::acceptVisitor(ASTVisitor* v) {
	return v->visitStructureDeclaration(this);
}

/*===========================ExpressionStatement============================*/
ExpressionStatement::ExpressionStatement(Expression* exp) {
	this->exp = exp;
}

void ExpressionStatement::describe() const {
	printf("---Expression(s) converted into statements\n");
}

llvm::Value* ExpressionStatement::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* ReturnStatement::acceptVisitor(ASTVisitor* v) {
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

llvm::Value* AssignStatement::acceptVisitor(ASTVisitor* v) {
	return v->visitAssignStatement(this);
}

/*===============================IfStatement================================*/
IfStatement::IfStatement(Expression* exp,Block* block,Block* else_block) {
	this->exp = exp;
	this->block = block;
	this->else_block = else_block;
}

void IfStatement::describe() const {
	printf("---Found If Statement\n");
}

llvm::Value* IfStatement::acceptVisitor(ASTVisitor* v) {
	return v->visitIfStatement(this);
}

/*===============================ExternStatement================================*/
ExternStatement::ExternStatement(Keyword* type,Identifier* ident,
          std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* args, bool hasPointerType)
{
	this->type = type;
	this->ident = ident;
	this->args = args;
	this->hasPointerType = hasPointerType;
}

void ExternStatement::describe() const {
	printf("---Found extern: %s\n",ident->name);
}

llvm::Value* ExternStatement::acceptVisitor(ASTVisitor* v) {
        return v->visitExternStatement(this);
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

IdentType SymbolTable::at(const char* ident) const {
	int frame_size = frames.size();
        assert(frame_size);
        for (int i = 0; i<frame_size;i++) {
          if (frames.at(i).count(std::string(ident))) {
            return frames.at(i).at(std::string(ident));
          }
        }
	return INVALID;
}

bool SymbolTable::check(const char* ident) const {
	int frame_size = frames.size();
        assert(frame_size);
        for (int i = 0; i<frame_size;i++) {
          if (frames.at(i).count(std::string(ident))) {
	    return true;
          }
        }
        return false;
}

bool SymbolTable::checkLocal(const char* ident) const {
	int frame_size = frames.size();
	assert(frame_size);
	if (frames.at(frame_size-1).count(std::string(ident))) {
	  return true;
	}
	return false;
}

bool SymbolTable::check(const char* ident,IdentType type) const {
	int frame_size = frames.size();
        assert(frame_size);
        for (int i = 0; i<frame_size;i++) {
          if (frames.at(i).count(std::string(ident))) {
            if (frames.at(i).at(std::string(ident))==type) {
              return true;
            }
          }
        }
        return false;
}

bool SymbolTable::checkLocal(const char* ident,IdentType type) const {
	int frame_size = frames.size();
        assert(frame_size);
        if (frames.at(frame_size-1).count(std::string(ident))) {
          if (frames.at(frame_size-1).at(std::string(ident))==type) {
            return true;
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

/*===============================TypeTable================================*/
TypeTable::TypeTable() {
	//Nothing to do
}

void TypeTable::insert(const char* ident) {
	types.push_back(std::string(ident));
}

bool TypeTable::check(const char* ident) const {
	return std::find(types.begin(), types.end(), std::string(ident)) != types.end();
}


/*===============================ReferenceExpression================================*/
ReferenceExpression::ReferenceExpression(Identifier* ident,Expression* offsetExpression,
	bool hasPointerType, bool addressOfThis, bool hasStructureType) {
	this->ident = ident;
	this->offsetExpression = offsetExpression;
	this->hasPointerType = hasPointerType;
	this->addressOfThis = addressOfThis;
	this->hasStructureType = hasStructureType;
}

bool ReferenceExpression::usesDirectValue() const {
	return (offsetExpression == nullptr);
}

bool ReferenceExpression::identsDeclared() const {
	return (ident->declaredAsVar() && (!offsetExpression || hasStructureType ||
		offsetExpression->identsDeclared()));
}

void ReferenceExpression::describe() const {
	printf("---Found ReferenceExpression for ident: %s\n",ident->name);
}

llvm::Value* ReferenceExpression::acceptVisitor(ASTVisitor* v) {
	return v->visitReferenceExpression(this);
}

