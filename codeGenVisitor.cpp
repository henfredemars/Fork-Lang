#include "codeGenVisitor.h"
#include <iostream>

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
	switchMap.insert(std::make_pair("||", BOP_OR));
	switchMap.insert(std::make_pair("&&", BOP_AND));
}

llvm::Value* CodeGenVisitor::castIntToFloat(llvm::Value* val) {
	return builder->CreateSIToFP(val, builder->getDoubleTy());
}

llvm::Value* CodeGenVisitor::castIntToBoolean(llvm::Value* val) {
	return builder->CreateICmpNE(val, llvm::ConstantInt::get(*context, llvm::APInt(64, 0, true)));
}

llvm::Value* CodeGenVisitor::castFloatToBoolean(llvm::Value* val) {
	return builder->CreateFCmpONE(val, llvm::ConstantFP::get(*context, llvm::APFloat(0.0)));
}

llvm::Value* CodeGenVisitor::castBooleantoInt(llvm::Value* val) {
	return builder->CreateZExtOrBitCast(val, builder->getInt64Ty());
}

llvm::Value* CodeGenVisitor::castPointerToInt(llvm::Value* val) {
	return builder->CreatePtrToInt(val, builder->getInt64Ty());
}

llvm::Value* CodeGenVisitor::castIntToPointer(llvm::Value* val) {
	return builder->CreateIntToPtr(val, llvm::Type::getInt64PtrTy(*context));
}

llvm::Type* CodeGenVisitor::getValType(llvm::Value* val) {
	return val->getType();
}

llvm::Type* CodeGenVisitor::getPointedType(llvm::Value* val) {
	return val->getType()->getContainedType(0);
}

llvm::Type* CodeGenVisitor::getFuncRetType(llvm::Function* func) {
	return func->getReturnType();
}

llvm::Type* CodeGenVisitor::getAllocaType(llvm::AllocaInst* alloca) {
	return alloca->getAllocatedType();
}

llvm::Function* CodeGenVisitor::generateFunction(bool hasPointerType, std::string returnType, std::string name, std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* arguments) {
	llvm::FunctionType* funcType = nullptr;
	llvm::Function* func = nullptr;
	std::vector<llvm::Type*> inputArgs; //set input args as float or int
	for(size_t i = 0, end = arguments->size(); i < end; ++i) {
		std::string type = arguments->at(i)->type->name;
		if(arguments->at(i)->hasPointerType) {
			if(type == "float") {
				inputArgs.push_back(llvm::Type::getDoublePtrTy(*context));
			}
			else if(type == "int") {
				inputArgs.push_back(llvm::Type::getInt64PtrTy(*context));
			}
			else {
				return (llvm::Function*) ErrorV("Invalid argument pointer type for function definition");
			}
		}
		else {
			if(type == "float") {
				inputArgs.push_back(builder->getDoubleTy());
			}
			else if(type == "int") {
				inputArgs.push_back(builder->getInt64Ty());
			}
			else {
				return (llvm::Function*) ErrorV("Invalid argument type for function definition");
			}
		}
	}
	if(hasPointerType) { //set function return type
		if(returnType == "float") {
			funcType = llvm::FunctionType::get(llvm::Type::getDoublePtrTy(*context), inputArgs, false);
		}

		else if(returnType == "int") {
			funcType = llvm::FunctionType::get(llvm::Type::getInt64PtrTy(*context), inputArgs, false);
		}
		else if(returnType == "void") {
			return (llvm::Function*) ErrorV("Invalid return type of void* for function definition");
		}
		else {
			return (llvm::Function*) ErrorV("Invalid return pointer type for function definition");
		}
	}
	else {
		if(returnType == "float") {
			funcType = llvm::FunctionType::get(builder->getDoubleTy(), inputArgs, false);
		}

		else if(returnType == "int") {
			funcType = llvm::FunctionType::get(builder->getInt64Ty(), inputArgs, false);
		}
		else if(returnType == "void") {
			funcType = llvm::FunctionType::get(builder->getVoidTy(), inputArgs, false); 
		}
		else {
			return (llvm::Function*) ErrorV("Invalid return type for function definition");
		}
	}
	func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, theModule.get());
	{ //set names for func args
	size_t i = 0;
	for (auto &arg : func->args())
	  arg.setName(arguments->at(i++)->ident->name);
	}
	return func;
}

llvm::AllocaInst* CodeGenVisitor::createAlloca(llvm::Function* func, llvm::Type* type, const std::string &name) {
	llvm::IRBuilder<true, llvm::NoFolder> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
	if(type->isDoubleTy()) {
		return tempBuilder.CreateAlloca(builder->getDoubleTy(), 0, name);
	}
	else if(type->isIntegerTy()) {
		return tempBuilder.CreateAlloca(builder->getInt64Ty(), 0, name);
	}
	else if(type->isPointerTy()) {
		if(type->getContainedType(0)->isIntegerTy()) {
			return tempBuilder.CreateAlloca(llvm::Type::getInt64PtrTy(*context), 0, name);
		}
		else if(type->getContainedType(0)->isDoubleTy()) {
			return tempBuilder.CreateAlloca(llvm::Type::getDoublePtrTy(*context), 0, name);
		}
	}
	return nullptr;
}

CodeGenVisitor::CodeGenVisitor(std::string name) {
	error = false;
	justReturned = false;
	populateSwitchMap();
	context = &llvm::getGlobalContext();
	forkJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
	theModule = llvm::make_unique<llvm::Module>(name, *context);
	theModule->setDataLayout(forkJIT->getTargetMachine().createDataLayout());
	builder = llvm::make_unique<llvm::IRBuilder<true, llvm::NoFolder>>(*context);
	voidValue = llvm::ReturnInst::Create(*context);
	floatNullPointer = llvm::Constant::getNullValue(llvm::Type::getDoublePtrTy(*context));
	intNullPointer = llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(*context));
}

void CodeGenVisitor::executeMain() {
	if(!error) {
		auto handle = forkJIT->addModule(std::move(theModule));
		auto mainSymbol = forkJIT->findSymbol("main");
		assert(mainSymbol && "No code to execute, include a main function");
		void (*func)() = (void (*)())(intptr_t)mainSymbol.getAddress();
	    printf("\nExecuting main function...\n");
	    func();
	    printf("---> main() returns: void\n");
	    delete voidValue;
	    forkJIT->removeModule(handle);
	}
	else {
		printf("Main execution halted due to errors\n");
	}
}

void CodeGenVisitor::printModule() const {
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
		//commit action //TODO
		return voidValue;
	}
	return ErrorV("Invalid nullary operator");
}

/*==============================UnaryOperator===============================*/
llvm::Value* CodeGenVisitor::visitUnaryOperator(UnaryOperator* u) {
	llvm::Value* expr = u->exp->acceptVisitor(this);
	if(!expr) {
		expr = intNullPointer;
	}
	if(getValType(expr)->isVoidTy()) {
		return ErrorV("Unary operator applied to void type");
	}
	else if(getValType(expr)->isPointerTy()) {
		expr = castPointerToInt(expr);
		switch(*u->op) {
			case '-':
			return castIntToPointer(builder->CreateMul(llvm::ConstantInt::get(*context, llvm::APInt(64, -1, true)), expr));
			case '!':
			return castIntToPointer(castBooleantoInt(builder->CreateNot(castIntToBoolean(expr))));
			default:
			return ErrorV("Invalid unary operator found");
		}	
	}
	else if(getValType(expr)->isDoubleTy()) {
		switch(*u->op) {
			case '-':
			return builder->CreateFMul(llvm::ConstantFP::get(*context, llvm::APFloat(-1.0)), expr);
			case '!':
			return castBooleantoInt(builder->CreateNot(castFloatToBoolean(expr)));
			default:
			return ErrorV("Invalid unary operator found");
		}
	}
	else if (getValType(expr)->isIntegerTy()) {
		switch(*u->op) {
			case '-':
			return builder->CreateMul(llvm::ConstantInt::get(*context, llvm::APInt(64, -1, true)), expr);
			case '!':
			return castBooleantoInt(builder->CreateNot(castIntToBoolean(expr)));
			default:
			return ErrorV("Invalid unary operator found");
		}	
	}
	return ErrorV("Unexpected type found for evaluated expression applied to unary operator");
}

/*==============================BinaryOperator==============================*/
llvm::Value* CodeGenVisitor::visitBinaryOperator(BinaryOperator* b) {
	llvm::Value* left = b->left->acceptVisitor(this);
 	llvm::Value* right = b->right->acceptVisitor(this);
	if (!left) {
		left = intNullPointer;
	}
	if(!right) {
		right = intNullPointer;
	}
	if(getValType(left)->isVoidTy() || getValType(right)->isVoidTy()) {
		return ErrorV("Binary operator applied to void type");
	}
	if(getValType(left)->isIntegerTy() && getValType(right)->isDoubleTy()) {
		left = castIntToFloat(left);
	}
	else if(getValType(left)->isDoubleTy() && getValType(right)->isIntegerTy()) {
		right = castIntToFloat(right);
	}
	if(getValType(left)->isPointerTy() || getValType(right)->isPointerTy()) { //at least one operand is a pointer
		if(getValType(left)->isPointerTy()) {
			left = castPointerToInt(left);
		}
		if(getValType(right)->isPointerTy()) {
			right = castPointerToInt(right);
		}
		if(getValType(left)->getTypeID() != getValType(right)->getTypeID()) { //if both operands are not now integers
			return ErrorV("Binary operator applied to pointer and incorrect non-integer type");
		}
		switch (switchMap.find(b->op)->second) {
			case BOP_PLUS:
			return castIntToPointer(builder->CreateAdd(left, right));
			case BOP_MINUS:
			return castIntToPointer(builder->CreateSub(left, right));
			case BOP_MULT:
			return castIntToPointer(builder->CreateMul(left, right));
			case BOP_DIV:
			return castIntToPointer(builder->CreateSDiv(left, right));
			case BOP_NEQ:
			return castIntToPointer(castBooleantoInt(builder->CreateICmpNE(left, right)));
			case BOP_EQ:
			return castIntToPointer(castBooleantoInt(builder->CreateICmpEQ(left, right)));
			case BOP_GTE:
			return castIntToPointer(castBooleantoInt(builder->CreateICmpSGE(left, right)));
			case BOP_LTE:
			return castIntToPointer(castBooleantoInt(builder->CreateICmpSLE(left, right)));
			case BOP_GT:
			return castIntToPointer(castBooleantoInt(builder->CreateICmpSGT(left, right)));
			case BOP_LT:
			return castIntToPointer(castBooleantoInt(builder->CreateICmpSLT(left, right)));
			case BOP_OR:
			return castIntToPointer(builder->CreateOr(castIntToBoolean(left), castIntToBoolean(right)));
			case BOP_AND:
			return castIntToPointer(builder->CreateAnd(castIntToBoolean(left), castIntToBoolean(right)));
			return ErrorV("Attempt to generate code for not yet implemented dot binary operator");
			default:
			return ErrorV("Invalid binary operator found");
		}
	}
	else if(getValType(left)->isDoubleTy()) { //both operands are floats
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
			return castBooleantoInt(builder->CreateFCmpONE(left, right));
			case BOP_EQ:
			return castBooleantoInt(builder->CreateFCmpOEQ(left, right));
			case BOP_GTE:
			return castBooleantoInt(builder->CreateFCmpOGE(left, right));
			case BOP_LTE:
			return castBooleantoInt(builder->CreateFCmpOLE(left, right));
			case BOP_GT:
			return castBooleantoInt(builder->CreateFCmpOGT(left, right));
			case BOP_LT:
			return castBooleantoInt(builder->CreateFCmpOLT(left, right));
			case BOP_OR:
			return castBooleantoInt(builder->CreateOr(castFloatToBoolean(left), castFloatToBoolean(right)));
			case BOP_AND:
			return castBooleantoInt(builder->CreateAnd(castFloatToBoolean(left), castFloatToBoolean(right)));
			return ErrorV("Attempt to generate code for not yet implemented dot binary operator");
			default:
			return ErrorV("Invalid binary operator found");
		}
	}
	else if(getValType(left)->isIntegerTy()) { //both operands are ints
		switch (switchMap.find(b->op)->second) {
			case BOP_PLUS:
			return builder->CreateAdd(left, right);
			case BOP_MINUS:
			return builder->CreateSub(left, right);
			case BOP_MULT:
			return builder->CreateMul(left, right);
			case BOP_DIV:
			return builder->CreateSDiv(left, right);
			case BOP_NEQ:
			return castBooleantoInt(builder->CreateICmpNE(left, right));
			case BOP_EQ:
			return castBooleantoInt(builder->CreateICmpEQ(left, right));
			case BOP_GTE:
			return castBooleantoInt(builder->CreateICmpSGE(left, right));
			case BOP_LTE:
			return castBooleantoInt(builder->CreateICmpSLE(left, right));
			case BOP_GT:
			return castBooleantoInt(builder->CreateICmpSGT(left, right));
			case BOP_LT:
			return castBooleantoInt(builder->CreateICmpSLT(left, right));
			case BOP_OR:
			return builder->CreateOr(castIntToBoolean(left), castIntToBoolean(right));
			case BOP_AND:
			return builder->CreateAnd(castIntToBoolean(left), castIntToBoolean(right));
			return ErrorV("Attempt to generate code for not yet implemented dot binary operator");
			default:
			return ErrorV("Invalid binary operator found");
		}
	}
	return ErrorV("Unexpected type found for evaluated operand applied to binary operator");
}

/*==================================Block===================================*/
llvm::Value* CodeGenVisitor::visitBlock(Block* b) {
	llvm::Value* lastVisited = nullptr;
	if(b->statements) {
		for(size_t i = 0, end = b->statements->size(); i != end; ++i) {
			lastVisited = b->statements->at(i)->acceptVisitor(this);
		}
	}
	return lastVisited;
}

/*===============================FunctionCall===============================*/
llvm::Value* CodeGenVisitor::visitFunctionCall(FunctionCall* f) {
	llvm::Function* func = theModule->getFunction(f->ident->name); //search func name in module
	if(!func) {
		return ErrorV("Unknown function reference");
	}
	if(func->arg_size() != f->args->size()) {
		return ErrorV("Wrong number of arguments passed to function");
	}
	std::vector<llvm::Value*> argVector;
	auto funcArgs = func->arg_begin();
	for(size_t i = 0, end = f->args->size(); i != end; ++i) {
		llvm::Value* argument = f->args->at(i)->acceptVisitor(this);
		llvm::Argument* funcArgument = funcArgs++;
		if(!argument) { //input NULL to functions
			if(getValType(funcArgument)->isPointerTy()) {
				if(getPointedType(funcArgument)->isIntegerTy()) {
					argument = intNullPointer;
				}
				else if(getPointedType(funcArgument)->isDoubleTy()) {
					argument = floatNullPointer;
				}
				else {
					return ErrorV("Attempt to input pointer type to function argument of incorrect type");
				}
			}
			else {
				return ErrorV("Attempt to input NULL to function argument of incorrect type");
			}
		}
		if(getValType(argument) != getValType(funcArgument)) {
			if(getValType(argument)->isIntegerTy() && getValType(funcArgument)->isDoubleTy()) {
				argument = castIntToFloat(argument);
			}
			else {
				return ErrorV("Invalid type as input for function args");
			}
		}
		argVector.push_back(argument);
	}
	return builder->CreateCall(func, argVector);
}

/*===============================NullLiteral===============================*/
llvm::Value* CodeGenVisitor::visitNullLiteral(NullLiteral* n) {
	return nullptr;
}

/*=================================Keyword==================================*/
llvm::Value* CodeGenVisitor::visitKeyword(Keyword* k) {
	return ErrorV("Attempt to generate code for dangling keyword");
}

/*============================VariableDefinition============================*/
llvm::Value* CodeGenVisitor::visitVariableDefinition(VariableDefinition* v) {
	std::string name = v->ident->name;
	if(namedValues.count(name) != 0) {
		return ErrorV("Attempt to redefine variable");
	}
	llvm::Function* func = builder->GetInsertBlock()->getParent(); //future optimizations include memgen
	std::string type = v->type->name;
	llvm::Value* val = nullptr;
	if(v->hasPointerType) {
		if(v->exp) { //custom value
			val = v->exp->acceptVisitor(this);
			if(!val) { //Assign Variable to NULL
				if(type == "int") {
					val = intNullPointer;
				}
				else if(type == "float") {
					val = floatNullPointer;
				}	
				else {
					return ErrorV("Attempt to create variable of incorrect pointer type");
				}
			}
			else {
				if(!getValType(val)->isPointerTy()) {
					return ErrorV("Attempt to assign non-pointer type to pointer type");
				}
				else {
					if(getPointedType(val)->isIntegerTy() && type != "int") {
						return ErrorV("Attempt to assign incorrect pointer type to int*");
					}
					else if(getPointedType(val)->isDoubleTy() && type != "float") {
						return ErrorV("Attempt to assign incorrect pointer type to float*");
					}
				}
			}
		}
		else { //default value	
			if(type == "int") {
				val = intNullPointer;
			}
			else if(type == "float") {
				val = floatNullPointer;
			}
			else {
				return ErrorV("Attempt to create variable of incorrect pointer type");
			}
		}
	}
	else {
		if(v->exp) {
			val = v->exp->acceptVisitor(this);
			if(!val) {
				return ErrorV("Unable to assign expression evaluated to null");
			}
			if(getValType(val)->isPointerTy()) {
				return ErrorV("Attempt to assign pointer type to non-pointer type");
			}
			if(type == "int" && getValType(val)->isDoubleTy()) {
				return ErrorV("Attempt to assign float to int type");
			}
			else if(type == "float" && getValType(val)->isIntegerTy()) {
				val = castIntToFloat(val);
			}
		}
		else { // default value
			if(type == "int") {
				val = llvm::ConstantInt::get(*context, llvm::APInt(64, 0, true));
			}
			else if(type == "float") {
				val = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
			}
			else {
				return ErrorV("Attempt to create variable of an incorrect type");
			}
		}
	}
	if(val) {//add to map
		llvm::AllocaInst* alloca = createAlloca(func, val->getType(), v->ident->name);
		if(!alloca) {
			return ErrorV("Unable to generate stack variable with type or function parameters");
		}
		namedValues.insert(std::make_pair(v->ident->name, alloca));
  		builder->CreateStore(val, alloca);
  		return v->ident->acceptVisitor(this);
	}
	return ErrorV("Unable to generate value for variable");
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> vars = s->getVariables();
	llvm::StructType* currStruct = llvm::StructType::create(*context, s->ident->name);
	llvm::Type** types = new llvm::Type*[vars.size()];
	for(size_t i = 0, end = vars.size(); i != end; ++i) {
		std::string type = vars.at(i)->type->name;
		if(type == "int") {
			types[i] = builder->getInt64Ty();
		}
		else if(type == "float") {
			types[i] = builder->getDoubleTy();
		}
		else if(type == s->ident->name) { //if struct is inside itself
			if(!vars.at(i)->hasPointerType) {
				return ErrorV("Attempt to evaluate recursive struct with no pointer");
			}
			types[i] = llvm::PointerType::getUnqual(currStruct);
		}
		else if(llvm::StructType* tempStruct = structTypes[type]) { //getStructType retrieves struct from struct map
			if(!vars.at(i)->hasPointerType) {
				types[i] = tempStruct;
			}
			else {
				types[i] = llvm::PointerType::getUnqual(tempStruct);
			}
		}
		else {
			return ErrorV("Attempt to create type within struct that is not previously declared");
		}
	}
	currStruct->setBody(llvm::ArrayRef<llvm::Type*>(types, vars.size()));
	structTypes.insert(std::make_pair(s->ident->name, currStruct)); //add struct to struct list
	return voidValue;
}

/*============================FunctionDefinition============================*/
llvm::Value* CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* f) {
	llvm::Function* func = theModule->getFunction(f->ident->name);
	if(!func) {
		func = generateFunction(f->hasPointerType, f->type->name, f->ident->name, f->args); //create function object with type|ident|args
	}
	if(!func) {//generateFunction returned nullptr
		return ErrorV("Invalid function signature");
	}
	if(!func->empty()) {
		return ErrorV("Function is already defined");
	}
	llvm::BasicBlock* block = llvm::BasicBlock::Create(*context, "function begin", func);
	builder->SetInsertPoint(block);
	namedValues.clear();
	for (auto &arg : func->args()) {
		llvm::AllocaInst* alloca = createAlloca(func, arg.getType(), arg.getName());
    	builder->CreateStore(&arg, alloca); // Store init value into alloca
		namedValues[arg.getName()] = alloca; //setup map
	} //create alloca for each argument
	llvm::Value* retVal = f->block->acceptVisitor(this);
	return retVal;
}

/*==========================StructureDeclaration============================*/
llvm::Value* CodeGenVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	//llvm::Function* func = builder->GetInsertBlock()->getParent();
	//llvm::ConstantAggregateZero* structInit = llvm::ConstantAggregateZero::get(structTypes[s->ident->name]);
	//llvm::AllocaInst* alloca = createAlloca(func, structDec->getType(), s->ident->name);
	//llvm::Value* structDec = structInit;
	//return builder->CreateStore(structDec, alloca);
	return ErrorV("Attempt to evaluate not yet implemented structure declaration");//TODO
}


/*===========================ExpressionStatement============================*/
llvm::Value* CodeGenVisitor::visitExpressionStatement(ExpressionStatement* e) {
	return e->exp->acceptVisitor(this);	//evaluated but value discarded
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	llvm::Function* func = builder->GetInsertBlock()->getParent();
	justReturned = true;
	if(r->exp) { 
		if(llvm::Value* retVal = r->exp->acceptVisitor(this)) {
			if(getValType(retVal)->isVoidTy() && getFuncRetType(func)->isVoidTy()) {
				if(getFuncRetType(func)->isVoidTy()) {
					retVal = builder->CreateRetVoid();
					verifyFunction(*func);
					return retVal;
				}
				else {
					return ErrorV("Unable to return bad void type from function");
				}
			}
			else if(getFuncRetType(func)->getTypeID() == getValType(retVal)->getTypeID()) {
				if(getFuncRetType(func)->isPointerTy()) {
					if(getFuncRetType(func)->getContainedType(0)->getTypeID() != getPointedType(retVal)->getTypeID()) {
						return ErrorV("Unable to return bad pointer type from function");
					}
				}
				builder->CreateRet(retVal);
				verifyFunction(*func);
				return retVal;
			}
			else if(getValType(retVal)->isIntegerTy() && getFuncRetType(func)->isDoubleTy()) {
				builder->CreateRet(castIntToFloat(retVal));
				verifyFunction(*func);
				return retVal;
			}
			else {
				return ErrorV("Unable to return bad type from function");
			}
		}
	}
	else {
		if(getFuncRetType(func)->isVoidTy()) {
			llvm::Value* retVal = builder->CreateRetVoid();
			verifyFunction(*func);
			return retVal;
		}
		else {
			return ErrorV("Unable to return bad void type from function");
		}
	}
	//return NULL
	if(getFuncRetType(func)->isPointerTy()) {
		llvm::Value* retVal = nullptr;
		if(getFuncRetType(func)->getContainedType(0)->isIntegerTy()) {
			retVal = builder->CreateRet(intNullPointer);
		}
		else if(getFuncRetType(func)->getContainedType(0)->isDoubleTy()) {
			retVal = builder->CreateRet(floatNullPointer);
		}
		else {
			return ErrorV("Unable to return bad pointer type from function");
		}
		verifyFunction(*func);
		return retVal;
	}
	return ErrorV("Unable to return incorrect NULL type from function");
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	ReferenceExpression* left = a->target;
	if(left->addressOfThis) {
		return ErrorV("Unable to assign memory address of left operand");
	}
	llvm::AllocaInst* var = namedValues[left->ident->name];
	if(!var) {
		return ErrorV("Unable to evaluate left operand in assignment statement");
	}
	llvm::Value* right = a->valxp->acceptVisitor(this);
	if(!right) {
		if(getAllocaType(var)->isPointerTy()) { //assign to NULL
			if(getAllocaType(var)->getContainedType(0)->isDoubleTy()) {
				right = floatNullPointer;
			}
			else if(getAllocaType(var)->getContainedType(0)->isIntegerTy()) {
				right = intNullPointer;
			}
			else {
				return ErrorV("Unable to assign to unknown pointer type");
			}
			builder->CreateStore(right, var);
			return right;
		}
		else {
			return ErrorV("Unable to assign evaluated null right operand to non pointer type");
		}
	}
	if(left->hasPointerType) {
		llvm::Value* offset = left->offsetExpression->acceptVisitor(this);
		//dereference left
		auto varPtr = builder->CreateLoad(var)->getPointerOperand();
		if(getValType(right) != getPointedType(varPtr)->getContainedType(0)) {
			if(getValType(right)->isIntegerTy() && getPointedType(varPtr)->getContainedType(0)->isDoubleTy()) {
				right = castIntToFloat(right);
			}
			else {
				return ErrorV("Dereferenced left operand is assigned to right operand of incorrect type");
			}
		}
		llvm::LoadInst* derefVar = builder->CreateLoad(builder->CreateGEP(varPtr, offset));
		//store value for right in dereferenced left
		builder->CreateStore(right, derefVar);
		return right;	
	}
	if(getAllocaType(var)->isDoubleTy() && getValType(right)->isIntegerTy()) {
		right = castIntToFloat(right);
	}
	else if(getAllocaType(var)->getTypeID() != getValType(right)->getTypeID()) {
		return ErrorV("Unable to assign evaluated right operand of bad type to left operand");
	}
	else if(getAllocaType(var)->isPointerTy()) {
		if(getAllocaType(var)->getContainedType(0)->getTypeID() != getPointedType(right)->getTypeID()) {
			return ErrorV("Unable to assign evaluated right operand of bad pointer type to left operand");
		}
	}
	builder->CreateStore(right, var); //store value for right in var
	return right; //allows chained assignment X = ( Y = 4 + 1);
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	llvm::Value* condition = i->exp->acceptVisitor(this);
	if(!condition) { //input NULL
		condition = castPointerToInt(intNullPointer);
	}
	else if(getValType(condition)->isPointerTy()) { //input address
		condition = castPointerToInt(condition);
	}
	if(getValType(condition)->isDoubleTy()) {
		condition = castFloatToBoolean(condition);
	}
	else if (getValType(condition)->isIntegerTy()) {
		condition = castIntToBoolean(condition);
	}
	else {
		return ErrorV("Unable to determine condition type");
	}
	llvm::Function* func = builder->GetInsertBlock()->getParent();
	llvm::BasicBlock* thenIf = llvm::BasicBlock::Create(*context, "then", func);
	llvm::BasicBlock* elseIf = llvm::BasicBlock::Create(*context, "else");
	llvm::BasicBlock* mergeIf = llvm::BasicBlock::Create(*context, "if");
	builder->CreateCondBr(condition, thenIf, elseIf); //branch between blocks
	//insert into then
	builder->SetInsertPoint(thenIf);
	llvm::Value* ifEval = nullptr;
	if(i->block) {
		justReturned = false;
		ifEval = i->block->acceptVisitor(this);
	}
        if (!justReturned) { //Ugly but at least it's ugly just here
	  builder->CreateBr(mergeIf);
        } else {
	  justReturned = false;
	}
	thenIf = builder->GetInsertBlock();
	func->getBasicBlockList().push_back(elseIf);
	//insert into else
	builder->SetInsertPoint(elseIf);
	llvm::Value* elseEval = nullptr;
	if(i->else_block) {
		justReturned = false;
		elseEval = i->else_block->acceptVisitor(this);
	}
	if (!justReturned) { //Ugly again, but visitor pattern limits
	  builder->CreateBr(mergeIf);
	} else {
	  justReturned = false;
	}
	elseIf = builder->GetInsertBlock();
	func->getBasicBlockList().push_back(mergeIf);
	//resolve then, else in mergeIf
	builder->SetInsertPoint(mergeIf);
	//llvm::PHINode* phi = nullptr;
	//evaluate phi node
	//if(getValType(ifEval)->isDoubleTy()) {
	//	phi = builder->CreatePHI(llvm::Type::getDoubleTy(*context), 2);
	//	elseEval = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
	//}
	//else if(getValType(ifEval)->isIntegerTy()) {
	//	phi = builder->CreatePHI(llvm::Type::getInt64Ty(*context), 2);
	//	elseEval = llvm::ConstantInt::get(*context, llvm::APInt(64, 0, true));
	//}
	//else if(getValType(ifEval)->isVoidTy()) {
	//	phi = builder->CreatePHI(llvm::Type::getVoidTy(*context), 2);
	//	elseEval = ifEval;
	//}
	//else if(getValType(ifEval)->isPointerTy()) {
	//	if(getPointedType(ifEval)->isIntegerTy()) {
	//		phi = builder->CreatePHI(llvm::Type::getInt64PtrTy(*context), 2);
	//		elseEval = llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(*context));
	//	}
	//	if(getPointedType(ifEval)->isDoubleTy()) {
	//		phi = builder->CreatePHI(llvm::Type::getDoublePtrTy(*context), 2); 
 	//		elseEval = llvm::Constant::getNullValue(llvm::Type::getDoublePtrTy(*context));
	//	}
	//}
	//else {
	//	return ErrorV("If block could not be evaluated to an acceptable type");
	//}
	//phi->addIncoming(thenValue, thenIf);
	//phi->addIncoming(elseValue, elseIf);
	return voidValue;
}

/*===============================ReferenceExpression================================*/
llvm::Value* CodeGenVisitor::visitReferenceExpression(ReferenceExpression* r) {
	if(!r)
		return ErrorV("Unable to evaluate reference Expression");
	llvm::AllocaInst* var = namedValues[r->ident->name];
	if(!var) {
		return ErrorV("Unable to evaluate variable");
	}
	if(!r->hasPointerType) { 
		if(r->addressOfThis) {
			return builder->CreateLoad(builder->CreateConstGEP1_64(builder->CreateLoad(var)->getPointerOperand(), 0))->getPointerOperand();
		}
		return r->ident->acceptVisitor(this);
	}
	//dereference using [] or *, assume already of GEP type
	llvm::Value* offset = r->offsetExpression->acceptVisitor(this);
	if(!getValType(offset)->isIntegerTy()) {
		return ErrorV("Unable to access relative address as a non-integer type");
	}
	auto varPtr = builder->CreateLoad(var)->getPointerOperand();
	if(r->addressOfThis) { 
		return builder->CreateLoad(builder->CreateGEP(varPtr, offset))->getPointerOperand();
	}
	return builder->CreateLoad(builder->CreateLoad(builder->CreateGEP(varPtr, offset)));
}

/*===============================ExternStatement================================*/
llvm::Value* CodeGenVisitor::visitExternStatement(ExternStatement* e) {
	llvm::Function* func = theModule->getFunction(e->ident->name);
	if(!func) {
		func = generateFunction(e->hasPointerType, e->type->name, e->ident->name, e->args); //create function object with type|ident|args
	}
	if(!func) {
		return ErrorV("Invalid function signature");
	}
	return voidValue;
}
