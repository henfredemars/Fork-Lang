#include "codeGenVisitor.h"

void CodeGenVisitor::populateSwitchMap() {
	switchMap.insert( std::make_pair("+", BOP_PLUS) );
	switchMap.insert( std::make_pair("-", BOP_MINUS) );
	switchMap.insert( std::make_pair("*", BOP_MULT) );
	switchMap.insert( std::make_pair("/", BOP_DIV) );
	switchMap.insert( std::make_pair(">=", BOP_GTE) );
	switchMap.insert( std::make_pair("<=", BOP_LTE) );
	switchMap.insert( std::make_pair(">", BOP_GT) );
	switchMap.insert( std::make_pair("<", BOP_LT) );
	switchMap.insert( std::make_pair("!=", BOP_NEQ) );
	switchMap.insert( std::make_pair("==", BOP_EQ) );
	switchMap.insert( std::make_pair(".", BOP_DOT) );
} 

llvm::Value* CodeGenVisitor::ErrorV(const char* str) {
  fprintf(stderr, "Error: %s\n", str);
  return nullptr;
}

CodeGenVisitor::CodeGenVisitor(std::string name) {
	context = &llvm::getGlobalContext();
	populateSwitchMap();
	theModule = llvm::make_unique<llvm::Module>(name, *context);
	builder = llvm::make_unique<llvm::IRBuilder<>>(*context);
}

/*===================================Node===================================*/
llvm::Value* CodeGenVisitor::visitNode(Node* n) {
	return ErrorV("Attempt to generate code for generic Node");
}

/*================================Expression================================*/
llvm::Value* CodeGenVisitor::visitExpression(Expression* e) {
	return ErrorV("Attempt to generate code for generic Expression");
}

/*================================Statement=================================*/
llvm::Value* CodeGenVisitor::visitStatement(Statement* s) {
	return ErrorV("Attempt to generate code for generic Statement");
}

/*=================================Integer==================================*/
llvm::Value* CodeGenVisitor::visitInteger(Integer* i) {
	return llvm::ConstantInt::get(*context, llvm::APInt(i->value, 64));
}

/*==================================Float===================================*/
llvm::Value* CodeGenVisitor::visitFloat(Float* f) {
	return llvm::ConstantFP::get(*context, llvm::APFloat(f->value));
}

/*================================Identifier================================*/
llvm::Value* CodeGenVisitor::visitIdentifier(Identifier* i) {
	// retrieve variable from the map
	// TODO: insert code for variable checking during usage
  llvm::Value *V = namedValues[i->name];
  if (!V)
    return ErrorV("Unknown variable name");
  return V;
}

/*=============================NullaryOperator==============================*/
llvm::Value* CodeGenVisitor::visitNullaryOperator(NullaryOperator* n) {
	if(*n->op == ';') {
		//commit action
		return nullptr;
	}
	return ErrorV("Invalid nullary operator");
}

/*==============================UnaryOperator===============================*/
llvm::Value* CodeGenVisitor::visitUnaryOperator(UnaryOperator* u) {
	llvm::Value* expr = u->exp->acceptVisitor(this);
	switch(*u->op) {
		case '-':
		return builder->CreateFMul(llvm::ConstantInt::get(*context, llvm::APInt(-1, 64)), expr, "multmp");
		case '!':
		case '*':
		return ErrorV("Not yet specified unary operator");
		default:
		return ErrorV("Invalid unary operator");
	}
}

/*==============================BinaryOperator==============================*/
llvm::Value* CodeGenVisitor::visitBinaryOperator(BinaryOperator* b) {
	llvm::Value* left = b->left->acceptVisitor(this);
 	llvm::Value* right = b->right->acceptVisitor(this);
 	//grab left and right
	if (!left || !right)
		return ErrorV("Could not evaluate binary operator");
	//use map for binary operators
	switch (switchMap.find(b->op)->second) {
		case BOP_PLUS:
		return builder->CreateFAdd(left, right, "addtmp");
		case BOP_MINUS:
		return builder->CreateFSub(left, right, "subtmp");
		case BOP_MULT:
		return builder->CreateFMul(left, right, "multmp");
		case BOP_DIV:
		return builder->CreateFDiv(left, right, "divtmp");
		case BOP_NEQ:
		case BOP_EQ:
		case BOP_GTE:
		case BOP_LTE:
		case BOP_GT:
		case BOP_LT:
		case BOP_DOT:
		return ErrorV("Attempt to generate code for not yet implemented binary operator");
		//assignment operator is separate
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
	//grab function and evaluate with arguments
	llvm::Function* func = theModule->getFunction(f->ident->name);
	if(!func)
		return ErrorV("Unknown function reference");
	if(func->arg_size() != f->args->size())
		return ErrorV("Wrong number of arguments passed");

	std::vector<llvm::Value*> argVector;
	for(size_t i = 0, end = f->args->size(); i != end; ++i) {
		argVector.push_back(f->args[i][0]->acceptVisitor(this));
		if(!argVector.back())
			return nullptr;
	}
	return builder->CreateCall(func, argVector, "calltmp");
}

/*=================================Keyword==================================*/
llvm::Value* CodeGenVisitor::visitKeyword(Keyword* k) {
	return ErrorV("Attempt to generate code for dangling keyword");
}

/*============================VariableDefinition============================*/
llvm::Value* CodeGenVisitor::visitVariableDefinition(VariableDefinition* v) {
	//add identifier with a default value
	std::string type = v->type->name; //int float or void
	llvm::Value* val = nullptr;
	if(type == "int") {
		int64_t i = 0;
		val = llvm::ConstantInt::get(*context, llvm::APInt(i, 64));
	}
	else if(type == "float")
		val = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
	if(val != nullptr)
		namedValues.insert(std::make_pair(v->ident->name, val));
	return val;
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	return ErrorV("Attempt to evaluate not yet implemented structure definition");
}

/*============================FunctionDefinition============================*/
llvm::Value* CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* f) {
	//add function to be called
	return nullptr;
}

/*==========================StructureDeclaration============================*/
llvm::Value* CodeGenVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	return ErrorV("Attempt to evaluate not yet implemented structure declaration");
}


/*===========================ExpressionStatement============================*/
llvm::Value* CodeGenVisitor::visitExpressionStatement(ExpressionStatement* e) {
	//evaluated but value discarded
	return nullptr;
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	//exit the current block
	return nullptr;
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	//map a value to an exisiting identifier
	return nullptr;
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	return ErrorV("Attempt to evluate not yet implemented if statement");
}
