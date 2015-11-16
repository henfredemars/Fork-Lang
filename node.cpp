
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
Statement::Statement() {
	this->commit = false;
}

void Statement::setCommit(const bool& commit) {
	this->commit = commit;
}

bool Statement::statementCommits() const {
	return commit;
}

bool Statement::lambdable() const {
	return false;
}

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
	#ifdef YYDEBUG
	printf("---Found Literal Integer: %i\n", (int)value);
	#endif
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
	#ifdef YYDEBUG
	printf("---Found Float: %f\n", value);
	#endif
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
	#ifdef YYDEBUG
	printf("---Found Identifier: %s\n",name);
	#endif
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

/*==============================UnaryOperator===============================*/
UnaryOperator::UnaryOperator(char* op, Expression* exp) {
	this->op = dup_char(op);
	this->exp = exp;
}

bool UnaryOperator::identsDeclared() const {
	return exp->identsDeclared();
}

void UnaryOperator::describe() const {
	#ifdef YYDEBUG
	printf("---Found Unary Operator\n");
	#endif
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
	#ifdef YYDEBUG
	printf("---Found Binary Operator %s\n",this->op);
	#endif
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
	#ifdef YYDEBUG
	printf("---Found Block\n");
	#endif
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
	#ifdef YYDEBUG
	printf("---Found Function Call: %s\n",ident->name);
	#endif
}

llvm::Value* FunctionCall::acceptVisitor(ASTVisitor* v) {
	return v->visitFunctionCall(this);
}

/*=================================NullLiteral==================================*/
NullLiteral::NullLiteral() { }

void NullLiteral::describe() const {
	#ifdef YYDEBUG
	printf("---Found Null\n");
	#endif
}

bool NullLiteral::identsDeclared() const {
	return true;
}

llvm::Value* NullLiteral::acceptVisitor(ASTVisitor* v) {
	return v->visitNullLiteral(this);
}

/*=================================Keyword==================================*/
Keyword::Keyword(char* name) {
	this->name = dup_char(name);
}

void Keyword::describe() const {
	#ifdef YYDEBUG
	printf("---Found Keyword: %s\n",name);
	#endif
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
	assert(ident);
}

VariableDefinition::VariableDefinition() {
	this->type = nullptr;
	this->ident = nullptr;
	this->exp = nullptr;
	this->hasPointerType = false;
}

bool VariableDefinition::statementCommits() const {
	return (!exp || commit);
}

bool VariableDefinition::lambdable() const {
	return !(!exp);
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

const char* VariableDefinition::stringType() const {
	return type->name;
}

void VariableDefinition::describe() const {
	#ifdef YYDEBUG
	if (hasPointerType) {
		printf("---Found Variable Declaration: type='%s*' identifier='%s'\n",
			type->name,ident->name);
	} else {
		printf("---Found Variable Declaration: type='%s' identifier='%s'\n",
                        type->name,ident->name);
	}
	#endif
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

void StructureDefinition::setCommit(const bool& commit) {
	//Do nothing
}

bool StructureDefinition::statementCommits() const {
	return true; //Structure always commits
}

bool StructureDefinition::lambdable() const {
	return false;
}

std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> StructureDefinition::getVariables() const {
	StatementVisitor* sv = new StatementVisitor();
	block->acceptVisitor(sv);
	return sv->varList;
}

std::vector<StructureDeclaration*,gc_allocator<StructureDeclaration*>> StructureDefinition::getStructs() const {
	StatementVisitor* sv = new StatementVisitor();
	block->acceptVisitor(sv);
	return sv->structList;
}

bool StructureDefinition::validate() const {
	if (user_type_table.check(ident->name)) {
	  printf("User type already exists!\n");
	  return false;
	}
	user_type_table.insert(ident->name);
	return true;
}

void StructureDefinition::describe() const {
	#ifdef YYDEBUG
	printf("---Found Structure Definition: %s\n",ident->name);
	#endif
}

llvm::Value* StructureDefinition::acceptVisitor(ASTVisitor* v) {
	return v->visitStructureDefinition(this);
}

void StructureDefinition::acceptVisitor(StatementVisitor* v) {
	return v->visitStructureDefinition(this);
}


/*============================FunctionDefinition============================*/
FunctionDefinition::FunctionDefinition(Keyword* type, Identifier* ident,
	std::vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	Block* block, bool hasPointerType) {
	this->type = type;
	this->user_type = nullptr;
	this->ident = ident;
	this->args = args;
	this->block = block;
	this->hasPointerType = hasPointerType;
	assert(type && "Missing return type");
        sym_table.insert(ident->name,FUNCTION);
}

FunctionDefinition::FunctionDefinition(Identifier* user_type, Identifier* ident,
	std::vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	Block* block, bool hasPointerType) {
	this->type = nullptr;
	this->user_type = user_type;
	this->ident = ident;
	this->args = args;
	this->block = block;
	this->hasPointerType = hasPointerType;
	assert(user_type && "Missing user-defined return type");
        sym_table.insert(ident->name,FUNCTION);
}

void FunctionDefinition::setCommit(const bool& commit) {
	//Do nothing
}

bool FunctionDefinition::statementCommits() const {
	return true; //Always
}

bool FunctionDefinition::lambdable() const {
	return false;
}

void FunctionDefinition::describe() const {
	#ifdef YYDEBUG
	printf("---Found Function Definition: %s\n",ident->name);
	#endif
}

bool FunctionDefinition::validate() const {
	return (!user_type || user_type_table.check(user_type->name));
}

llvm::Value* FunctionDefinition::acceptVisitor(ASTVisitor* v) {
	return v->visitFunctionDefinition(this);
}

/*==========================StructureDeclaration============================*/
StructureDeclaration::StructureDeclaration(Identifier* user_type,Identifier* ident,bool hasPointerType) {
	this->user_type = user_type;
	this->type = nullptr;
	this->ident = ident;
	this->hasPointerType = hasPointerType;
}

void StructureDeclaration::setCommit(const bool& commit) {
	//Do nothing
}

bool StructureDeclaration::statementCommits() const {
	return true; //Always
}

bool StructureDeclaration::lambdable() const {
	return false;
}

bool StructureDeclaration::validate() const {
        if (sym_table.check(ident->name)) {
                printf("Variable name '%s' already exists in the symbol table\n",ident->name);
		return false;
        } else if (!user_type_table.check(user_type->name)) {
                yyerror("Type was not a declared in the structure table");
		return false;
	}
	return true;
}

const char* StructureDeclaration::stringType() const {
	return user_type->name;
}

void StructureDeclaration::describe() const {
	#ifdef YYDEBUG
	printf("---Found Structure Declaration: type='%s' identifier='%s'\n",
		user_type->name,ident->name);
	#endif
}

llvm::Value* StructureDeclaration::acceptVisitor(ASTVisitor* v) {
	return v->visitStructureDeclaration(this);
}

void StructureDeclaration::acceptVisitor(StatementVisitor* v) {
	v->visitStructureDeclaration(this);
}

/*===========================ExpressionStatement============================*/
ExpressionStatement::ExpressionStatement(Expression* exp) {
	this->exp = exp;
	assert(exp && "Empty expression statement");
}

bool ExpressionStatement::lambdable() const {
	return true;
}

void ExpressionStatement::describe() const {
	#ifdef YYDEBUG
	printf("---Expression(s) converted into statements\n");
	#endif
}

llvm::Value* ExpressionStatement::acceptVisitor(ASTVisitor* v) {
	return v->visitExpressionStatement(this);
}

/*=============================ReturnStatement==============================*/
ReturnStatement::ReturnStatement(Expression* exp) {
	this->exp = exp;
}

void ReturnStatement::setCommit(const bool& commit) {
	//Do nothing
}

bool ReturnStatement::statementCommits() const {
	return true; //Always
}

bool ReturnStatement::lambdable() const {
	return false;
}

void ReturnStatement::describe() const {
	#ifdef YYDEBUG
	if (exp) {
	  printf("---Found return statement with expression\n");
	} else {
	  printf("---Found return statement, statement returns void\n");
	}
	#endif
}

llvm::Value* ReturnStatement::acceptVisitor(ASTVisitor* v) {
	return v->visitReturnStatement(this);
}

/*=============================AssignStatement==============================*/
AssignStatement::AssignStatement(Expression* target,Expression* valxp) {
	this->target = target;
	this->valxp = valxp;
}

void AssignStatement::describe() const {
	#ifdef YYDEBUG
	printf("---Found Assignment Statement\n");
	#endif
}

bool AssignStatement::lambdable() const {
	return true;
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

void IfStatement::setCommit(const bool& commit) {
	//Do nothing
}

bool IfStatement::statementCommits() const {
	return true; //Always
}

bool IfStatement::lambdable() const {
	return false;
}

void IfStatement::describe() const {
	#ifdef YYDEBUG
	printf("---Found If Statement\n");
	#endif
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
        sym_table.insert(ident->name,FUNCTION);
}

void ExternStatement::setCommit(const bool& commit) {
	//Do nothing
}

bool ExternStatement::statementCommits() const {
	return true; //Always
}

bool ExternStatement::lambdable() const {
	return false;
}

void ExternStatement::describe() const {
	#ifdef YYDEBUG
	printf("---Found extern: %s\n",ident->name);
	#endif
}

llvm::Value* ExternStatement::acceptVisitor(ASTVisitor* v) {
        return v->visitExternStatement(this);
}

/*===============================SymbolTable================================*/
SymbolTable::SymbolTable() {
	//Push global scope
	this->push();
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

/*===============================PointerExpression================================*/
PointerExpression::PointerExpression(Identifier* ident,Expression* offsetExpression, Identifier* field) {
	this->ident = ident;
	this->offsetExpression = offsetExpression;
	this->field = field;
	assert(ident && "No identifier given for PointerExpression");
}

bool PointerExpression::usesDirectValue() const {
	return offsetExpression == nullptr;
}

bool PointerExpression::referencingStruct() const {
	return !(!field);
}

void PointerExpression::describe() const {
	#ifdef YYDEBUG
	printf("---Found PointerExpression for: %s\n",ident->name);
	#endif
}

bool PointerExpression::identsDeclared() const {
	return sym_table.check(ident->name,VARIABLE) && offsetExpression->identsDeclared();
}

llvm::Value* PointerExpression::acceptVisitor(ASTVisitor* v) {
	return v->visitPointerExpression(this);
}

/*===============================AddressOfExpression================================*/
AddressOfExpression::AddressOfExpression(Identifier* ident,Expression* offsetExpression) {
	this->ident = ident;
	this->offsetExpression = offsetExpression;
	assert(ident && "No identifier given for AddressOfExpression");
}

void AddressOfExpression::describe() const {
	#ifdef YYDEBUG
	printf("---Found AddressOfExpression for: %s\n",ident->name);
	#endif
}

bool AddressOfExpression::identsDeclared() const {
	return sym_table.check(ident->name,VARIABLE) && (!offsetExpression || offsetExpression->identsDeclared());
}

llvm::Value* AddressOfExpression::acceptVisitor(ASTVisitor* v) {
	return v->visitAddressOfExpression(this);
}

/*===============================StructureExpression================================*/
StructureExpression::StructureExpression(Identifier* ident,Identifier* field) {
	this->ident = ident;
	this->field = field;
	assert(ident && "No structure identifier given for StructureExpression");
	assert(field && "No field given for StructureExpression");
}

void StructureExpression::describe() const {
	#ifdef YYDEBUG
	printf("---Found StructureExpression for: %s, field: %s\n",ident->name,field->name);
	#endif
}

bool StructureExpression::identsDeclared() const {
	//TODO Check if field is valid field name?
	return sym_table.check(ident->name,VARIABLE);
}

llvm::Value* StructureExpression::acceptVisitor(ASTVisitor* v) {
	return v->visitStructureExpression(this);
}
