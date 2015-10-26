#include "codeGenVisitor.h"

llvm::Value* CodeGenVisitor::ErrorV(const char* str) {
  fprintf(stderr, "Error: %s\n", str);
  error = true;
  return nullptr;
}

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

bool CodeGenVisitor::isIntegerType(llvm::Value* val) {
	return val->getType()->getTypeID() == llvm::Type::IntegerTyID;
}
bool CodeGenVisitor::isFloatType(llvm::Value* val) {
	return val->getType()->getTypeID() == llvm::Type::DoubleTyID;
}
bool CodeGenVisitor::isVoidType(llvm::Value* val) {
	return val->getType()->getTypeID() == llvm::Type::VoidTyID;
}
llvm::Value* CodeGenVisitor::castIntToFloat(llvm::Value* val) {
	return builder->CreateSIToFP(val, llvm::Type::getDoubleTy(*context));
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

llvm::AllocaInst* CodeGenVisitor::createAlloca(llvm::Function* func, llvm::Type* type, const std::string &name) {
	llvm::IRBuilder<true, llvm::NoFolder> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
	if(type->getTypeID() == llvm::Type::DoubleTyID) {
		return tempBuilder.CreateAlloca(llvm::Type::getDoubleTy(*context), 0, name);
	}
	return tempBuilder.CreateAlloca(llvm::Type::getInt64Ty(*context), 0, name);
}

CodeGenVisitor::CodeGenVisitor(std::string name) {
	error = false;
	populateSwitchMap();
	currFunc = nullptr;
	context = &llvm::getGlobalContext();
	forkJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
	theModule = llvm::make_unique<llvm::Module>(name, *context);
	theModule->setDataLayout(forkJIT->getTargetMachine().createDataLayout());
	builder = llvm::make_unique<llvm::IRBuilder<true, llvm::NoFolder>>(*context);
}

void CodeGenVisitor::executeMain() {
	if(!error) {
		auto handle = forkJIT->addModule(std::move(theModule));
		auto mainSymbol = forkJIT->findSymbol("main");
		assert(mainSymbol && "No code to execute, include a main function");
		void (*func)() = (void (*)())(intptr_t)mainSymbol.getAddress();
	    func();
	    printf("main() returns: void\n");
	    forkJIT->removeModule(handle);
	}
	else {
		printf("Main execution halted due to errors\n");
	}

}

void CodeGenVisitor::printModule() {
	if(!error) {
		theModule->dump();
	}
	else {
		printf("IR dump prevented due to errors\n");
	}
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
	return llvm::ConstantInt::get(*context, llvm::APInt(64, i->value, true));
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
  return builder->CreateLoad(val, i->name);
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
		if(isFloatType(expr)) {
			return builder->CreateFMul(llvm::ConstantFP::get(*context, llvm::APFloat(-1.0)), expr);
		}
		else if(isIntegerType(expr)) {
			return builder->CreateMul(llvm::ConstantInt::get(*context, llvm::APInt(64, -1, true)), expr);
		}
		else {
			return ErrorV("Unary operator applied to void type");
		}
		case '!'://TODO
		case '*'://TODO
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
	//use flag for check if both operands are integers
	bool isInteger = false;
	if((left->getType()->getTypeID() != right->getType()->getTypeID())) {
		//cast values in IR
		if(isIntegerType(left)) {
			left = castIntToFloat(left);
		}
		else {
			right = castIntToFloat(right);
		}
	}
	if(isIntegerType(left)) {
		isInteger = true;
	}
	//binary op map
	switch (switchMap.find(b->op)->second) {
		case BOP_PLUS:
		return isInteger ? builder->CreateAdd(left, right) : builder->CreateFAdd(left, right);
		case BOP_MINUS:
		return isInteger ? builder->CreateSub(left, right) : builder->CreateFSub(left, right);
		case BOP_MULT:
		return isInteger ? builder->CreateMul(left, right) : builder->CreateFMul(left, right);
		case BOP_DIV:
		return isInteger ? builder->CreateSDiv(left, right) : builder->CreateFDiv(left, right);
		case BOP_NEQ:
		return isInteger ? builder->CreateICmpNE(left, right) : builder->CreateFCmpONE(left, right);		
		case BOP_EQ:
		return isInteger ? builder->CreateICmpEQ(left, right) : builder->CreateFCmpOEQ(left, right);
		case BOP_GTE:
		return isInteger ? builder->CreateICmpSGE(left, right) : builder->CreateFCmpOGE(left, right);
		case BOP_LTE:
		return isInteger ? builder->CreateICmpSLE(left, right) : builder->CreateFCmpOLE(left, right);
		case BOP_GT:
		return isInteger ? builder->CreateICmpSGT(left, right) : builder->CreateFCmpOGT(left, right);
		case BOP_LT:
		return isInteger ? builder->CreateICmpSLT(left, right) : builder->CreateFCmpOLT(left, right);
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
		llvm::Value* argument = f->args->at(i)->acceptVisitor(this);
		//if(argument->getType()->getTypeID() == llvm::Type::IntegerTyID && func->args()->getType()->getTypeID == llvm::Type::DoubleTyID) {
		//	argument = builder->CreateSIToFP(argVector.back(), llvm::Type::getDoubleTy(*context));
		//}
		argVector.push_back(argument);
	}
	return builder->CreateCall(func, argVector);
}

/*=================================Keyword==================================*/
llvm::Value* CodeGenVisitor::visitKeyword(Keyword* k) {
	return ErrorV("Attempt to generate code for dangling keyword");
}

/*============================VariableDefinition============================*/
llvm::Value* CodeGenVisitor::visitVariableDefinition(VariableDefinition* v) {
	llvm::Function* func = builder->GetInsertBlock()->getParent(); //future optimizations include memgen
	std::string type = v->type->name;
	llvm::Value* val = nullptr;
	if(v->exp) {
		val = v->exp->acceptVisitor(this);
		if(type == "int" && isFloatType(val)) {
			return ErrorV("Attempt to return float to int type");
		}
		else if(type == "float" && isIntegerType(val)) {
			val = castIntToFloat(val); //cast int to float type
		}
	}
	else { // add default
		if(type == "int") {
			val = llvm::ConstantInt::get(*context, llvm::APInt(64, 0, true));
		}
		else if(type == "float")
			val = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
		else
			return ErrorV("Attempt to create variable of an incorrect type");
	}
	if(val) {//add to map
		llvm::AllocaInst* alloca = createAlloca(func, val->getType(), v->ident->name);
		namedValues.insert(std::make_pair(v->ident->name, alloca));
  		builder->CreateStore(val, alloca);
  		return v->ident->acceptVisitor(this);
	}
	return ErrorV("Unable to generate value for variable");
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	//llvm::ArrayRef<llvm::Type*> types;
	//llvm::StructType* structDef = llvm::StructType::create(*context, types, s->ident->name, true);
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
	currFunc = func; //store current function for access by other classes
	llvm::BasicBlock* block = llvm::BasicBlock::Create(*context, "function start", func);
	builder->SetInsertPoint(block);
	namedValues.clear();
	for (auto &arg : func->args()) {
		llvm::AllocaInst* alloca = createAlloca(func, arg.getType(), arg.getName());
    	builder->CreateStore(&arg, alloca); // Store init value into alloca
		namedValues[arg.getName()] = alloca; //setup map
	} //create alloca for each argument
	llvm::Value* retVal =  f->block->acceptVisitor(this);
	currFunc = nullptr;
	return retVal;
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
	if(r->exp) {
		if(llvm::Value* retVal = r->exp->acceptVisitor(this)) {
			if(currFunc->getReturnType()->getTypeID() == retVal->getType()->getTypeID()) {
				builder->CreateRet(retVal);
				verifyFunction(*currFunc);
				return retVal;
			}
		}
		else {
			if(currFunc->getReturnType()->getTypeID() == llvm::Type::VoidTyID) {
				builder->CreateRetVoid();
				verifyFunction(*currFunc);
				return nullptr;
			}
		}
	}
	else {
		if(currFunc->getReturnType()->getTypeID() == llvm::Type::VoidTyID) {
			builder->CreateRetVoid();
			verifyFunction(*currFunc);
			return nullptr;
		}
	}
	currFunc->eraseFromParent();
	return ErrorV("Function deleted for erroneous return type or function body complications");
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	Identifier* left = (Identifier*)a->target;
	llvm::Value* right = a->valxp->acceptVisitor(this);
	if(!right)
		return ErrorV("Unable to evaluate right operand in assignment statement");
	llvm::AllocaInst* var = namedValues[left->name];
	if(!var)
		return ErrorV("Unable to evaluate left operand in assignment statement");
	if((var->getAllocatedType()->getTypeID() == llvm::Type::DoubleTyID) && isIntegerType(right)) {
		right = castIntToFloat(right);
	}
	else if(var->getAllocatedType()->getTypeID() != right->getType()->getTypeID()) {
		return ErrorV("Unable to assign evaluated right operand of bad type to left operand");
	}
	builder->CreateStore(right, var); //store value for right in var
	return right; //allows chained assignment X = ( Y = 4 + 1);
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	llvm::Value* condition = i->exp->acceptVisitor(this);
	if(!condition)
		return ErrorV("Unable to evaluate if statement condition");
	if(isFloatType(condition)) {
		condition = builder->CreateFCmpONE(condition, llvm::ConstantFP::get(*context, llvm::APFloat(0.0)));
	}
	else if (isIntegerType(condition)){
		condition = builder->CreateICmpNE(condition, llvm::ConstantInt::get(*context, llvm::APInt(64, 0, true)));
	}
	else {
		return ErrorV("Unable to determine condition type");
	}
	llvm::Function* func = builder->GetInsertBlock()->getParent();
	llvm::BasicBlock* thenIf = llvm::BasicBlock::Create(*context, "then", func);
	llvm::BasicBlock* elseIf = llvm::BasicBlock::Create(*context, "else");
	llvm::BasicBlock* mergeIf = llvm::BasicBlock::Create(*context, "if");
	builder->CreateCondBr(condition, thenIf, elseIf);
	builder->SetInsertPoint(thenIf);
	llvm::Value* ifEval = i->block->acceptVisitor(this);
	if(!ifEval)
		return ErrorV("If block could not be evaluated");
	builder->CreateBr(mergeIf);
	thenIf = builder->GetInsertBlock();
	func->getBasicBlockList().push_back(elseIf);
	builder->SetInsertPoint(elseIf);
	builder->CreateBr(mergeIf);
	elseIf = builder->GetInsertBlock();
	func->getBasicBlockList().push_back(mergeIf);
	builder->SetInsertPoint(mergeIf);
	llvm::Value* elseEval = nullptr;
	llvm::PHINode* phi = nullptr;
	if(isFloatType(ifEval)) {
		phi = builder->CreatePHI(llvm::Type::getDoubleTy(*context), 2);
		elseEval = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
	}
	else if(isIntegerType(ifEval)) {
		phi = builder->CreatePHI(llvm::Type::getInt64Ty(*context), 2);
		elseEval = llvm::ConstantInt::get(*context, llvm::APInt(64, 0, true));
	}
	else if(isVoidType(ifEval)) {
		phi = builder->CreatePHI(llvm::Type::getVoidTy(*context), 2);
		elseEval = ifEval;
	}
	else {
		return ErrorV("If block could not be evaluated to an acceptable type");
	}
	phi->addIncoming(ifEval, thenIf);
	phi->addIncoming(elseEval, elseIf);
	return phi;
}
