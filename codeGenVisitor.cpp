#include "codeGenVisitor.h"

static std::unique_ptr<llvm::Module> *TheModule;
static llvm::IRBuilder<> Builder(llvm::getGlobalContext());
static std::map<std::string, llvm::Value*> NamedValues;

llvm::Value* CodeGenVisitor::ErrorV(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

/*================================Expression================================*/
llvm::Value* CodeGenVisitor::visitExpression(Expression* e) {
	return nullptr;
}

/*================================Statement=================================*/
llvm::Value* CodeGenVisitor::visitStatement(Statement* s) {
	return nullptr;
}

/*=================================Integer==================================*/
llvm::Value* CodeGenVisitor::visitInteger(Integer* i) {
	return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(value));
}

/*==================================Float===================================*/
llvm::Value* CodeGenVisitor::visitFloat(Float* f) {
	return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(value));
}

/*================================Identifier================================*/
llvm::Value* CodeGenVisitor::visitIdentifier(Identifier* i) {
	// retrieve variable from the map
	// still have to insert code for variable checking during usage
  Value *V = NamedValues[Name];
  if (!V)
    ErrorV("Unknown variable name");
  return V;
}

/*=============================NullaryOperator==============================*/
llvm::Value* CodeGenVisitor::visitNullaryOperator(NullaryOperator* n) {
	return nullptr;
}

/*==============================UnaryOperator===============================*/
llvm::Value* CodeGenVisitor::visitUnaryOperator(UnaryOperator* u) {
	return nullptr;
}

/*==============================BinaryOperator==============================*/
llvm::Value* CodeGenVisitor::visitBinaryOperator(BinaryOperator* b) {
	Value* L = left->acceptCodeGenVisitor(*this);
 	Value* R = right->acceptCodeGenVisitor(*this);
	if (!L || !R)
		return nullptr;
	switch (op) {
		case '+':
		return Builder.CreateFAdd(L, R, "addtmp");
		case '-':
		return Builder.CreateFSub(L, R, "subtmp");
		case '*':
		return Builder.CreateFMul(L, R, "multmp");
		// case '=': not implemented yet
		default:
		return ErrorV("invalid binary operator");
	}
}

/*================================Assignment================================*/
llvm::Value* CodeGenVisitor::visitAssignment(Assignment* a) {
	return nullptr;
}

/*==================================Block===================================*/
llvm::Value* CodeGenVisitor::visitBlock(Block* b) {
	return nullptr;
}

/*===============================FunctionCall===============================*/
llvm::Value* CodeGenVisitor::visitFunctionCall(FunctionCall* f) {
	return nullptr;
}

/*=================================Keyword==================================*/
llvm::Value* CodeGenVisitor::visitKeyword(Keyword* k) {
	return nullptr;
}

/*============================VariableDefinition============================*/
llvm::Value* CodeGenVisitor::visitVariableDefinition(VariableDefinition* v) {
	return nullptr;
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	return nullptr;
}

/*============================FunctionDefinition============================*/
llvm::Value* CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* f) {
	return nullptr;
}

/*==========================StructureDeclaration============================*/
llvm::Value* CodeGenVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	return nullptr;
}


/*===========================ExpressionStatement============================*/
llvm::Value* CodeGenVisitor::visitExpressionStatement(ExpressionStatement* e) {
	return nullptr;
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	return nullptr;
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	return nullptr;
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	return nullptr;
}
