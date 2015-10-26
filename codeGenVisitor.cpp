#include "codeGenVisitor.h"

void CodeGenVisitor::populateSwitchMap() {
	switchMap.insert(std::make_pair("+", BOP_PLUS));
	switchMap.insert(std::make_pair("-", BOP_MINUS));
	switchMap.insert(std::make_pair("*", BOP_MULT));
	switchMap.insert(std::make_pair("/", BOP_DIV));
	switchMap.insert(std::make_pair(">=", BOP_GTE));
	switchMap.insert(std::make_pair("<=", BOP_LTE));
	switchMap.insert(std::make_pair(">", BOP_GT));
	switchMap.insert(std::make_pair("<", BOP_LT));
	switchMap.insert(std::make_pair("!=", BOP_NEQ));
	switchMap.insert(std::make_pair("==", BOP_EQ));
	switchMap.insert(std::make_pair(".", BOP_DOT));
}

llvm::Value* CodeGenVisitor::ErrorV(const char* str) {
  assert(true && fprintf(stderr, "Error: %s\n", str));
  return nullptr;
}

llvm::Function* CodeGenVisitor::generateFunction(FunctionDefinition* f) {
	std::string type = f->type->name;
	llvm::FunctionType* funcType = nullptr;
	llvm::Function* func = nullptr;
	std::vector<llvm::Type*> inputArgs; //set input args as float or int
	for(size_t i = 0, end = f->args->size(); i < end; ++i) {
		std::string type = f->args->at(i)->type->name;
		if(type == "float") {
			inputArgs.push_back(llvm::Type::getDoubleTy(*context));
		}
		else if(type == "int") {
			inputArgs.push_back(llvm::Type::getInt64Ty(*context));
		}
		else
			return (llvm::Function*) ErrorV("Invalid type - malformed function definition");
	}
	if(type == "void") { //set function return type
		funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), inputArgs, false); 
	}
	else if(type == "float") {
		funcType = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), inputArgs, false);
	}

	else if(type == "int") {
		funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context), inputArgs, false);
	}//add pointer types
	func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, f->ident->name, theModule.get()); //pass unique ptr to function

	{
	size_t i = 0;
	for (auto &arg : func->args())
	  arg.setName(f->args->at(i++)->ident->name);
	}
	return func;
}

CodeGenVisitor::CodeGenVisitor(std::string name) {
	populateSwitchMap();
	context = &llvm::getGlobalContext();
	forkJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
	theModule = llvm::make_unique<llvm::Module>(name, *context);
	theModule->setDataLayout(forkJIT->getTargetMachine().createDataLayout());
	builder = llvm::make_unique<llvm::IRBuilder<true, llvm::NoFolder>>(*context);
}

void CodeGenVisitor::executeMain() {
	auto handle = forkJIT->addModule(std::move(theModule));
	auto mainSymbol = forkJIT->findSymbol("main");
	assert(mainSymbol && "No code to be execute, include a main function");
	void (*func)() = (void (*)())(intptr_t)mainSymbol.getAddress();
    func();
    printf("main() returns: void\n");
    forkJIT->removeModule(handle);
}

void CodeGenVisitor::printModule() {
	theModule->dump();
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
  llvm::Value* val = namedValues[i->name];
  if (!val)
    return ErrorV("Attempt to generate code for not previously defined variable");
  return val;
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
		switch(expr->getType()->getTypeID()) { //switch by type to apply -1 to expr
			case llvm::Type::DoubleTyID:
				return builder->CreateFMul(llvm::ConstantFP::get(*context, llvm::APFloat(-1.0)), expr);
			case llvm::Type::IntegerTyID:
				return builder->CreateFMul(llvm::ConstantInt::get(*context, llvm::APInt(-1, 64)), expr);
		}
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
		return builder->CreateFAdd(left, right);
		case BOP_MINUS:
		return builder->CreateFSub(left, right);
		case BOP_MULT:
		return builder->CreateFMul(left, right);
		case BOP_DIV:
		return builder->CreateFDiv(left, right);
		case BOP_NEQ:
		case BOP_EQ:
		case BOP_GTE:
		case BOP_LTE:
		case BOP_GT:
		case BOP_LT:
		case BOP_DOT:
		return ErrorV("Attempt to generate code for not yet implemented binary operator");
		//assignment op separate
		default:
		return ErrorV("Invalid binary operator");
	}
}

/*==================================Block===================================*/
llvm::Value* CodeGenVisitor::visitBlock(Block* b) {
	llvm::Value* lastVisited = nullptr;
	for(size_t i = 0, end = b->statements->size(); i != end; ++i) {
		lastVisited = b->statements->at(i)->acceptVisitor(this);
	}
	return lastVisited;
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
		argVector.push_back(f->args->at(i)->acceptVisitor(this));
	}
	return builder->CreateCall(func, argVector);
}

/*=================================Keyword==================================*/
llvm::Value* CodeGenVisitor::visitKeyword(Keyword* k) {
	return ErrorV("Attempt to generate code for dangling keyword");
}

/*============================VariableDefinition============================*/
llvm::Value* CodeGenVisitor::visitVariableDefinition(VariableDefinition* v) {
	std::string type = v->type->name; //int float or void
	llvm::Value* val = nullptr;
	if(type == "int") {
		int64_t i = 0; //TODO: Figure out why variable necessary, attempted casting already
		val = llvm::ConstantInt::get(*context, llvm::APInt(i, 64));
	}
	else if(type == "float")
		val = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
	if(!val) //add default value variable to map
		namedValues.insert(std::make_pair(v->ident->name, val));
	return val;
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	return ErrorV("Attempt to evaluate not yet implemented structure definition");
}

/*============================FunctionDefinition============================*/
llvm::Value* CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* f) {
	llvm::Function* func = theModule->getFunction(f->ident->name);
	if(!func)
		func = generateFunction(f); //create function object with type|ident|args
	if(!func) //generateFunction returned nullptr
		return ErrorV("Invalid function signature");
	if(!func->empty())
		return ErrorV("Function is already defined");
	llvm::BasicBlock* block = llvm::BasicBlock::Create(*context, "function begin", func);
	builder->SetInsertPoint(block);
	namedValues.clear();
	for (auto &arg : func->args())
		namedValues[arg.getName()] = &arg;
	llvm::Value* retVal = f->block->acceptVisitor(this);
	if(!retVal && (func->getReturnType()->getTypeID() == llvm::Type::VoidTyID)) {//void return nullptr
		builder->CreateRetVoid();
		verifyFunction(*func);
		return nullptr;
	}
	if(retVal->getType()->getTypeID() == func->getReturnType()->getTypeID()) {//check typing for int/float
		builder->CreateRet(retVal);
		verifyFunction(*func);
		return retVal;
	}
	func->eraseFromParent();//erase function
	return ErrorV("Function deleted for erroneous return type or function body complications"); 
}

/*==========================StructureDeclaration============================*/
llvm::Value* CodeGenVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	return ErrorV("Attempt to evaluate not yet implemented structure declaration");
}


/*===========================ExpressionStatement============================*/
llvm::Value* CodeGenVisitor::visitExpressionStatement(ExpressionStatement* e) {
	return e->exp->acceptVisitor(this);	//evaluated but value discarded
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	return r->exp->acceptVisitor(this);
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	//TODO: map a value to an exisiting identifier
	//look for identifier
	//map target to value
	return nullptr;
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	return ErrorV("Attempt to evaluate not yet implemented if statement");
}
