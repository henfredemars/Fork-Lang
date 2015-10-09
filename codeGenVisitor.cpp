#include "codeGenVisitor.h"

llvm::LLVMContext* CodeGenVisitor::getLLVMContext() {
	return l;
}

llvm::Value* CodeGenVisitor::ErrorV(const char* str) {
  fprintf(stderr, "Error: %s\n", str);
  return nullptr;
}

llvm::IRBuilder<> Builder(llvm::getGlobalContext());

void CodeGenVisitor::initModule(std::string name) {
	theModule = llvm::make_unique<llvm::Module>(name, *getLLVMContext());
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
	return llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(i->value, 64));
}

/*==================================Float===================================*/
llvm::Value* CodeGenVisitor::visitFloat(Float* f) {
	return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(f->value));
}

/*================================Identifier================================*/
llvm::Value* CodeGenVisitor::visitIdentifier(Identifier* i) {
	// retrieve variable from the map
	// still have to insert code for variable checking during usage
  llvm::Value *V = namedValues[i->name];
  if (!V)
    return ErrorV("Unknown variable name");
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
	llvm::Value* L = b->left->acceptCodeGenVisitor(this);
 	llvm::Value* R = b->right->acceptCodeGenVisitor(this);
	if (!L || !R)
		return nullptr;
	switch (*b->op) {
		case '+':
		return Builder.CreateFAdd(L, R, "addtmp");
		case '-':
		return Builder.CreateFSub(L, R, "subtmp");
		case '*':
		return Builder.CreateFMul(L, R, "multmp");
		case '/':
		return Builder.CreateFDiv(L, R, "divtmp");
		default:
		return ErrorV("Invalid binary operator");
	}
}

/*==================================Block===================================*/
llvm::Value* CodeGenVisitor::visitBlock(Block* b) {
	//iterate through vector
	return nullptr;
}

/*===============================FunctionCall===============================*/
llvm::Value* CodeGenVisitor::visitFunctionCall(FunctionCall* f) {
	return nullptr;
}

/*=================================Keyword==================================*/
llvm::Value* CodeGenVisitor::visitKeyword(Keyword* k) {
	return ErrorV("Attempt to generate code for dangling keyword");
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
	//evaluated but value discarded
	return nullptr;
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	return nullptr;
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	//separate from bin op
	return nullptr;
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	return nullptr;
}
