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
llvm::Constant* CodeGenVisitor::getNullPointer(std::string typeName) {
	if(structTypes.find(typeName) != structTypes.end()) {
		llvm::StructType* tempStruct = std::get<0>(structTypes.find(typeName)->second); //recover struct type
		return llvm::Constant::getNullValue(llvm::PointerType::getUnqual(tempStruct)); //grab unique struct null
	}
	return (llvm::Constant*)ErrorV("Attempt to create null pointer of undeclared struct type");
}

llvm::LoadInst* CodeGenVisitor::getStructField(std::string typeString, std::string fieldName, llvm::Value* var) {
	if(structTypes.find(typeString) == structTypes.end()) {
		return (llvm::LoadInst*)ErrorV("Unable to access undeclared struct with dot operator");
	}
	auto structTuple = structTypes.find(typeString)->second;
	bool found = false;
	size_t index = 0;
	std::vector<std::string> varList = std::get<1>(structTuple); //get fields associated with struct
	for(size_t end = varList.size(); index != end; ++index) {
		if(fieldName == varList.at(index)) {
			found = true;
			break; //exit once field is found
		}
	}
	if(!found) {
		return (llvm::LoadInst*)ErrorV("Unable to evaluate field that does not belong to struct");
	}
	llvm::StructType* currStruct = std::get<0>(structTuple); 
	auto varptr = builder->CreateLoad(var)->getPointerOperand(); //get pointer to struct variable
	return builder->CreateLoad(builder->CreateStructGEP(currStruct, varptr, index)); //return struct field
}

llvm::Type* CodeGenVisitor::getTypeFromString(std::string typeName, bool isPointer, bool allowsVoid) {
	if(isPointer) { 
		if(typeName == "float") {
			return llvm::Type::getDoublePtrTy(*context);
		}
		else if(typeName == "int") {
			return llvm::Type::getInt64PtrTy(*context);
		}
		else if(structTypes.find(typeName) != structTypes.end()) {
			llvm::StructType* tempStruct = std::get<0>(structTypes.find(typeName)->second); //get struct type
			return llvm::PointerType::getUnqual(tempStruct); //return struct pointer type matching string
		}
		else {
			return (llvm::Type*) ErrorV("Invalid pointer type detected");
		}
	}
	else {
		if(typeName == "float") {
			return builder->getDoubleTy(); 
		}
		else if(typeName == "int") {
			return builder->getInt64Ty();
		}
		else if(typeName == "void") {
			if(allowsVoid) {
				return builder->getVoidTy();
			}
			return (llvm::Type*) ErrorV("Invalid type void detected");
		}
		else if(structTypes.find(typeName) != structTypes.end()) {
			return std::get<0>(structTypes.find(typeName)->second); //return struct type
		}
	}
	return (llvm::Type*) ErrorV("Invalid type detected");
}

llvm::Function* CodeGenVisitor::generateFunction(bool hasPointerType, std::string returnType, std::string name, std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* arguments) {
	llvm::FunctionType* funcType = nullptr;
	llvm::Function* func = nullptr;
	std::vector<llvm::Type*> inputArgs;
	for(auto it = arguments->begin(), end = arguments->end(); it < end; ++it) {
		auto argument = *it;
		llvm::Type* type = getTypeFromString(argument->stringType(), argument->hasPointerType, false); //grab int, float, int*, float*, struct, struct* type
		if(!type) {
			return (llvm::Function*) ErrorV("Invalid argument for function definition");
		}
		inputArgs.push_back(type); //place type in input arg vector
	}
	llvm::Type* type = getTypeFromString(returnType, hasPointerType, true); //grab void, int, float, int*, float*, struct, struct* type for return
	if(!type) {
		return (llvm::Function*) ErrorV("Invalid return for function definition");
	}
	funcType = llvm::FunctionType::get(type, inputArgs, false); //create funcType
	func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, theModule.get()); //create function with functype and use external
	{ //set names for func args
	size_t i = 0;
	for (auto &arg : func->args())
	  arg.setName(arguments->at(i++)->ident->name);
	}
	return func;
}

llvm::AllocaInst* CodeGenVisitor::createAlloca(llvm::Function* func, llvm::Type* type, const std::string &name) {
	llvm::IRBuilder<true, llvm::NoFolder> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
	if(llvm::AllocaInst* alloca = tempBuilder.CreateAlloca(type, 0, name)) { //use tempBuilder to push alloca into the function block beginning
		return alloca;
	}
	return (llvm::AllocaInst*)ErrorV("Unable to create alloca of incorrect type");
}

CodeGenVisitor::CodeGenVisitor(std::string name) {
	error = false;
	lambdaNum = 0; //lambda
	insideLambda = false; //lambda
	justReturned = false;
	populateSwitchMap();
	context = &llvm::getGlobalContext(); //set context for vars
	forkJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>(); //set jit
	theModule = llvm::make_unique<llvm::Module>(name, *context);
	theModule->setDataLayout(forkJIT->getTargetMachine().createDataLayout()); //set module for tracking and execution
	builder = llvm::make_unique<llvm::IRBuilder<true, llvm::NoFolder>>(*context); //set builder for IR insertion
	voidValue = llvm::ReturnInst::Create(*context); 
	floatNullPointer = llvm::Constant::getNullValue(llvm::Type::getDoublePtrTy(*context));
	intNullPointer = llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(*context)); // set default void and nullptr values, struct has to be retrieved
}

void CodeGenVisitor::executeMain() {
	if(!error) {
		auto handle = forkJIT->addModule(std::move(theModule)); // JIT the module
		auto mainSymbol = forkJIT->findSymbol("main"); 
		assert(mainSymbol && "No code to execute, include a main function");
		void (*func)() = (void (*)())(intptr_t)mainSymbol.getAddress(); //grab address of main
	    printf("\nExecuting main function...\n");
	    func(); //execute main
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
		theModule->dump(); //dump IR to stderr that is used
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
  llvm::AllocaInst* val = namedValues[i->name]; //find alloca in map
  if (!val)
    return ErrorV("Attempt to generate code for not previously defined variable");
  return builder->CreateLoad(val, i->name); //load alloca value from map
}

/*=============================NullaryOperator==============================
llvm::Value* CodeGenVisitor::visitNullaryOperator(NullaryOperator* n) {
	if(*n->op == ';') {
		//commit action //TODO
		return voidValue;
	}
	return ErrorV("Invalid nullary operator");
}*/

/*==============================UnaryOperator===============================*/
llvm::Value* CodeGenVisitor::visitUnaryOperator(UnaryOperator* u) {
	llvm::Value* expr = u->exp->acceptVisitor(this);
	if(!expr) { //NULL applied to unary operator
		expr = intNullPointer;
	}
	if(getValType(expr)->isVoidTy()) {
		return ErrorV("Unary operator applied to void type");
	}
	else if(getValType(expr)->isPointerTy()) { //pointer applied to unary op
		expr = castPointerToInt(expr);
		switch(*u->op) { //pointer acts as int then is returned as pointer
			case '-':
			return castIntToPointer(builder->CreateMul(llvm::ConstantInt::get(*context, llvm::APInt(64, -1, true)), expr));
			case '!':
			return castIntToPointer(castBooleantoInt(builder->CreateNot(castIntToBoolean(expr)))); //casted to boolean returned as pointer
			default:
			return ErrorV("Invalid unary operator found applied to pointer type");
		}	
	}
	else if(getValType(expr)->isDoubleTy()) { //float applied to unary op
		switch(*u->op) {
			case '-':
			return builder->CreateFMul(llvm::ConstantFP::get(*context, llvm::APFloat(-1.0)), expr);
			case '!':
			return castBooleantoInt(builder->CreateNot(castFloatToBoolean(expr)));
			default:
			return ErrorV("Invalid unary operator found applied to float type");
		}
	}
	else if (getValType(expr)->isIntegerTy()) { //integer applied to unary op
		switch(*u->op) {
			case '-':
			return builder->CreateMul(llvm::ConstantInt::get(*context, llvm::APInt(64, -1, true)), expr);
			case '!':
			return castBooleantoInt(builder->CreateNot(castIntToBoolean(expr)));
			default:
			return ErrorV("Invalid unary operator found applied to integer type");
		}	
	}
	else if(getValType(expr)->isStructTy()) { //operand is struct
		return ErrorV("Unable to evaluate unary operator applied to struct type");
	}
	return ErrorV("Unexpected type found for evaluated expression applied to unary operator");
}

/*==============================BinaryOperator==============================*/
llvm::Value* CodeGenVisitor::visitBinaryOperator(BinaryOperator* b) {
	llvm::Value* left = b->left->acceptVisitor(this);
 	llvm::Value* right = b->right->acceptVisitor(this);
	if (!left) { //NULL found
		left = intNullPointer;
	}
	if(!right) {
		right = intNullPointer;
	}
	if(getValType(left)->isVoidTy() || getValType(right)->isVoidTy()) { //void found
		return ErrorV("Binary operator applied to void type");
	}
	if(getValType(left)->isIntegerTy() && getValType(right)->isDoubleTy()) { //double and int cast both to double
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
		if(getValType(left) != getValType(right)) { //if both operands are not now integers
			return ErrorV("Binary operator applied to pointer and incorrect non-integer type");
		}
		switch (switchMap.find(b->op)->second) { //expect pointer return
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
			return castIntToPointer(builder->CreateOr(castIntToBoolean(left), castIntToBoolean(right))); //cast to boolean as input
			case BOP_AND:
			return castIntToPointer(builder->CreateAnd(castIntToBoolean(left), castIntToBoolean(right)));
			return ErrorV("Attempt to generate code for not yet implemented dot binary operator");
			default:
			return ErrorV("Invalid binary operator applied to pointer and integer or pointer type");
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
			default:
			return ErrorV("Invalid binary operator applied to float types");
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
			default:
			return ErrorV("Invalid binary operator applied to integer types");
		}
	}
	else if(getValType(left)->isStructTy() || getValType(right)->isStructTy()) { //either operand is a struct
		return ErrorV("Unable to evaluate binary operator applied to struct type");
	}
	return ErrorV("Unexpected type found for evaluated operand applied to binary operator");
}

/*==================================Block===================================*/
llvm::Value* CodeGenVisitor::visitBlock(Block* b) {
	llvm::Value* lastVisited = nullptr;
	if(b->statements) {
		std::vector<bool> commitVector;
		if(!insideLambda) {
			for(auto it = b->statements->begin(), end = b->statements->end(); it != end; ++it) { //create vector of statement commits
				auto statement = *it;
				commitVector.push_back(statement->statementCommits());
			}
			bool prev = true;
			for(auto it = commitVector.begin(), end = commitVector.end(); it != end; ++it) { //modify vector of statement commits such that the last commit is concurrent as well
				auto commit = *it;
				if(!commit) { //if not commiting, return prev value
					prev = commit;
				}
				else { //if commiting and not prev value, set current to false and record to true
					if(!prev) { 
						*it = false;
						prev = true;
					}
				}
			}
		}
		for(size_t i = 0, end = b->statements->size(); i != end; ++i) {
			auto statement = b->statements->at(i);
			bool commits = true;
			if(!insideLambda) { //if outside lambda, check if lambda must be created
				commits = commitVector.at(i);
			}
			if(!commits) { //if must create lambda
				insideLambda = true;
				statement->setCommit(true);
				//create env struct type
				llvm::StructType* currStruct = llvm::StructType::create(*context, "env"); //create env struct type
				std::vector<std::string> stringVec;
				std::vector<llvm::Type*> types;
				std::vector<llvm::Value*> vals;
				for(auto it = namedValues.begin(), end = namedValues.end(); it != end; ++it) {
					stringVec.push_back(it->first);
					types.push_back(getAllocaType(it->second));
					vals.push_back(builder->CreateLoad(it->second, it->first));
				}
				currStruct->setBody(types); //insert type list into env
				structTypes.insert(std::make_pair("env", std::make_tuple(currStruct, stringVec))); //add env and fields to struct list
				//create env
				llvm::AllocaInst* alloca = createAlloca(builder->GetInsertBlock()->getParent(), currStruct, "e0");
				llvm::Constant* structDec = llvm::ConstantAggregateZero::get(currStruct);
				builder->CreateStore(structDec, alloca);
				namedValues.insert(std::make_pair("e0", alloca));
				for(size_t i = 0, end = vals.size(); i != end; ++i) {
					auto structFieldRef = getStructField("env", stringVec.at(i), alloca)->getPointerOperand();
					builder->CreateStore(vals.at(i), structFieldRef);
				}
				auto copyValues = namedValues; //clone map
				auto ip = builder->saveAndClearIP(); //store block insertion point
				auto lambdaStatements = new std::vector<Statement*,gc_allocator<Statement*>>();
				lambdaStatements->push_back(statement);
				lambdaStatements->push_back(new ReturnStatement(nullptr));
				char* keyword = (char *)GC_MALLOC_ATOMIC(6); 
				strcpy(keyword, "void");
				char* envType = (char *)GC_MALLOC_ATOMIC(4); 
				strcpy(envType, "env");
				char* envName = (char *)GC_MALLOC_ATOMIC(3); 
				strcpy(envName, "e0");
				char* identifier = (char *)GC_MALLOC_ATOMIC(32);
				std::ostringstream ss;
				ss << "lambda" << lambdaNum++;
				strcpy(identifier, (ss.str()).c_str()); // name mangle the lambda
				//pass struct to function def
				auto envArg = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
				envArg->push_back(new StructureDeclaration(new Identifier(envType), new Identifier(envName), true)); //add env* e argument
				FunctionDefinition* fd = new FunctionDefinition(new Keyword(keyword), new Identifier(identifier), 
					envArg, new Block(lambdaStatements), false);
				fd->acceptVisitor(this);
				builder->restoreIP(ip); //restore block insertion point
				namedValues = copyValues;
				//pass env as pointer
				auto envVal = new std::vector<Expression*, gc_allocator<Expression*>>();
				envVal->push_back(new AddressOfExpression(new Identifier(envName), nullptr));
				FunctionCall* lambdaCall = new FunctionCall(new Identifier(identifier), envVal);
				lastVisited = lambdaCall->acceptVisitor(this); //create lambda call
				structTypes.erase("env");
				namedValues.erase("e0");
				insideLambda = false;
			}
			else {
				lastVisited = statement->acceptVisitor(this);
			}
		}
	}
	return lastVisited;
}

/*===============================FunctionCall===============================*/
llvm::Value* CodeGenVisitor::visitFunctionCall(FunctionCall* f) {
	llvm::Function* func = theModule->getFunction(f->ident->name); //search func name in module
	if(!func) { //func name does not exist
		return ErrorV("Unknown function reference");
	}
	if(func->arg_size() != f->args->size()) { //func name exists but wrong args
		return ErrorV("Wrong number of arguments passed to function");
	}
	std::vector<llvm::Value*> argVector;
	auto funcArgs = func->arg_begin();
	for(size_t i = 0, end = f->args->size(); i != end; ++i) { //evaluate vector of args and type check
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
				else if(getPointedType(funcArgument)->isStructTy()) {
					argument = getNullPointer(getPointedType(funcArgument)->getStructName());
				}
				else {
					return ErrorV("Attempt to input NULL to function argument of incorrect pointer type");
				}
			}
			else {
				return ErrorV("Attempt to input NULL to function argument of incorrect type");
			}
		}
		if(getValType(argument) != getValType(funcArgument)) { //if int found instead of double, cast
			if(getValType(argument)->isIntegerTy() && getValType(funcArgument)->isDoubleTy()) {
				argument = castIntToFloat(argument);
			}
			else {
				return ErrorV("Invalid type as input for function args");
			}
		}
		argVector.push_back(argument); //push the arg into the vector
	}
	return builder->CreateCall(func, argVector); //establish function call with name and args
}

/*===============================NullLiteral===============================*/
llvm::Value* CodeGenVisitor::visitNullLiteral(NullLiteral* n) {
	return nullptr; //checked as nullptr and evaluated for individual value based on expected value in other nodes
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
	llvm::Function* func = builder->GetInsertBlock()->getParent();
	std::string type = v->stringType();
	llvm::Value* val = nullptr;
	if(v->hasPointerType) {
		if(v->exp) { //instantiated value
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
		if(v->exp) { //instantiated value
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
				return ErrorV("Attempt to declare variable of an incorrect type");
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
  		return val;
	}
	return ErrorV("Unable to generate value for variable");
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> vars = s->getVariables();
	std::vector<StructureDeclaration*,gc_allocator<StructureDeclaration*>> structs = s->getStructs();
	llvm::StructType* currStruct = llvm::StructType::create(*context, s->ident->name); //forward declare struct type
	std::vector<llvm::Type*> types;
	std::vector<std::string> stringVec;
	for(auto it = vars.begin(), end = vars.end(); it != end; ++it) { //iterate over var list
		VariableDefinition* var = *it;
		std::string typeName = var->stringType();
		if(var->exp) {
			return ErrorV("Attempt to instantiate types within structs that can only be declared");
		}
		llvm::Type* type = getTypeFromString(typeName, var->hasPointerType, false);
		if(!type) {
			return ErrorV("Invalid type for struct definition");
		}
		types.push_back(type); //put type in type vector
		stringVec.push_back(var->ident->name); //put string in string vector
	}
	for(auto it = structs.begin(), end = structs.end(); it != end; ++it) { //iterate over struct list (structs are at the end of a struct but the order is inconseuquential due to var mapping)
		StructureDeclaration* strct = *it;
		std::string typeName = strct->stringType();
		if(!strct->hasPointerType) {	
			if(typeName == s->ident->name) { //same struct type inside itself
				return ErrorV("Attempt to evaluate recursive struct with no pointer");
			}
			else if(structTypes.find(typeName) != structTypes.end()) { //
				llvm::StructType* tempStruct = std::get<0>(structTypes.find(typeName)->second);
				types.push_back(tempStruct); //store struct type
			}
			else {
				return ErrorV("Attempt to create struct pointer type within struct that is not previously declared");
			}
		} 
		else {
			if(typeName == s->ident->name) {
				types.push_back(llvm::PointerType::getUnqual(currStruct));
			}
			else if(structTypes.find(typeName) != structTypes.end()) {
				llvm::StructType* tempStruct = std::get<0>(structTypes.find(typeName)->second);
				types.push_back(llvm::PointerType::getUnqual(tempStruct)); //store pointer to struct type
			}
			else {
				return ErrorV("Attempt to create struct type within struct that is not previously declared");
			}
		}
		stringVec.push_back(strct->ident->name); //store string name for struct
	}
	currStruct->setBody(types); //insert type list into struct type
	structTypes.insert(std::make_pair(s->ident->name, std::make_tuple(currStruct, stringVec))); //add struct type and fields to struct list
	return voidValue;
}

/*============================FunctionDefinition============================*/
llvm::Value* CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* f) {
	llvm::Function* func = theModule->getFunction(f->ident->name);
	if(!func) {
		if(!f->type) {
			func = generateFunction(f->hasPointerType, f->user_type->name, f->ident->name, f->args); //struct return type
		} 
		else {
			func = generateFunction(f->hasPointerType, f->type->name, f->ident->name, f->args); //keyword return type
		}
	}
	if(!func) {//generateFunction returned nullptr
		return ErrorV("Invalid function signature");
	}
	if(!func->empty()) {
		return ErrorV("Function is already defined");
	}
	llvm::BasicBlock* block = llvm::BasicBlock::Create(*context, "func", func);
	builder->SetInsertPoint(block);
	namedValues.clear();
	if(!insideLambda) { //keep variables to allow access to current scope
		for (auto &arg : func->args()) {
			llvm::AllocaInst* alloca = createAlloca(func, arg.getType(), arg.getName());
			if(!alloca) {
				return ErrorV("Unable to create stack variable inside function body for function argument");
			}
	    	builder->CreateStore(&arg, alloca); // Store init value into alloca
			namedValues.insert(std::make_pair(arg.getName(), alloca));
		} //create alloca for each argument
	}
	else {
		auto env = func->arg_begin();
		llvm::AllocaInst* envAlloca = createAlloca(func, env->getType(), env->getName());
		if(!envAlloca) {
			return ErrorV("Unable to create stack variable inside lambda body for environment");
		}
		builder->CreateStore(env, envAlloca);
		namedValues.insert(std::make_pair(env->getName(), envAlloca));
		auto varList = std::get<1>(structTypes.find("env")->second);
		for(size_t i = 0, end = varList.size(); i != end; ++i) {
			llvm::Value* val = getStructField("env", varList.at(i), builder->CreateLoad(envAlloca));
			llvm::AllocaInst* alloca = createAlloca(func, getValType(val), varList.at(i));
			builder->CreateStore(val, alloca);
			namedValues.insert(std::make_pair(varList.at(i), alloca));
		}
	}
	llvm::Value* retVal = f->block->acceptVisitor(this);
	return retVal;
}

/*==========================StructureDeclaration============================*/
llvm::Value* CodeGenVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	llvm::Function* func = builder->GetInsertBlock()->getParent();
	std::string type = s->stringType();
	std::string name = s->ident->name;
	if(structTypes.find(type) == structTypes.end()) { 
		return ErrorV("Unable to instantiate struct that is not previously declared");
	}
	auto structTuple = structTypes.find(type)->second;
	if(s->hasPointerType) {
		llvm::Constant* structPtrDec = getNullPointer(type); //make nullptr
		llvm::AllocaInst* alloca = createAlloca(func, structPtrDec->getType(), name); //make structptr alloca
		if(!alloca) {
			return ErrorV("Unable to create alloca of pointer to struct type");
		}
		namedValues.insert(std::make_pair(name, alloca)); 
		return builder->CreateStore(structPtrDec, alloca); //store structptr in alloca
	}
	llvm::StructType* currStruct = std::get<0>(structTuple);
	llvm::AllocaInst* alloca = createAlloca(func, currStruct, name); //make struct alloca
	if(!alloca) {
		return ErrorV("Unable to create alloca of struct type");
	}
	llvm::Constant* structDec = llvm::ConstantAggregateZero::get(currStruct); //zero all values
	namedValues.insert(std::make_pair(name, alloca));
	return builder->CreateStore(structDec, alloca); //store struct in alloca
}


/*===========================ExpressionStatement============================*/
llvm::Value* CodeGenVisitor::visitExpressionStatement(ExpressionStatement* e) {
	return e->exp->acceptVisitor(this);	//evaluated but value discarded
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	llvm::Function* func = builder->GetInsertBlock()->getParent();
	justReturned = true;
	if(r->exp) { //return exp
		if(llvm::Value* retVal = r->exp->acceptVisitor(this)) { 
			if(getValType(retVal)->isVoidTy() && getFuncRetType(func)->isVoidTy()) { //void func returned
				if(getFuncRetType(func)->isVoidTy()) {
					retVal = builder->CreateRetVoid();
					verifyFunction(*func); //check for any func errors
					return retVal;
				}
				else {
					return ErrorV("Unable to return bad void type from function");
				}
			}
			else if(getFuncRetType(func) == getValType(retVal)) { //ret val is the func ret val
				if(getFuncRetType(func)->isPointerTy()) { //pointer ret val the same
					if(getFuncRetType(func)->getContainedType(0) != getPointedType(retVal)) {
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
		if(getFuncRetType(func)->isPointerTy()) { //return NULL
			llvm::Value* retVal = nullptr;
			if(getFuncRetType(func)->getContainedType(0)->isIntegerTy()) {
				retVal = builder->CreateRet(intNullPointer);
			}
			else if(getFuncRetType(func)->getContainedType(0)->isDoubleTy()) {
				retVal = builder->CreateRet(floatNullPointer);
			}
			else if(getFuncRetType(func)->getContainedType(0)->isStructTy()) {
				retVal = builder->CreateRet(getNullPointer(getFuncRetType(func)->getContainedType(0)->getStructName()));
			}
			else {
				return ErrorV("Unable to return bad pointer type from function");
			}
			verifyFunction(*func);
			return retVal;
		}
		else {
			return ErrorV("Unable to return incorrect NULL type from function");
		}	
	}
	if(getFuncRetType(func)->isVoidTy()) {//return or return;
		llvm::Value* retVal = builder->CreateRetVoid();
		verifyFunction(*func);
		return retVal;
	}
	return ErrorV("Unable to return bad void type from function");
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	llvm::Value* right = a->valxp->acceptVisitor(this); //visit RHS
	AssignmentLHSVisitor* leftVisitor = new AssignmentLHSVisitor(this, right); //pass codegenvisitor and RHS to LHS visitor 
	llvm::Value* retVal = a->target->acceptVisitor(leftVisitor); //visit LHS
	if(!retVal) {
		return ErrorV("Unable to evaluate assignment statement");
	}
	return retVal;
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	llvm::Value* condition = i->exp->acceptVisitor(this);
	if(!condition) { //input NULL
		condition = castPointerToInt(intNullPointer);
	}
	else if(getValType(condition)->isPointerTy()) {
		condition = castPointerToInt(condition);
	}
	if(getValType(condition)->isDoubleTy()) { 
		condition = castFloatToBoolean(condition);
	}
	else if (getValType(condition)->isIntegerTy()) {
		condition = castIntToBoolean(condition);
	}
	else { //error if struct, otherwise convert to boolean
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
	return voidValue; //void val never used
}

/*===============================PointerExpression================================*/
llvm::Value* CodeGenVisitor::visitPointerExpression(PointerExpression* e) {
	if(!e) {
		return ErrorV("Unable to evaluate Pointer Expression");
	}
	llvm::AllocaInst* var = namedValues[e->ident->name];
	if(!var) {
		return ErrorV("Unable to evaluate variable");
	}
	if(e->usesDirectValue()) {
		return e->ident->acceptVisitor(this); //pointer expression not dereferenced
	}
	llvm::Value* offset = e->offsetExpression->acceptVisitor(this);
	if(!offset) {
		return ErrorV("Unable to evaluate offset for Pointer Expression");
	}
	if(!getValType(offset)->isIntegerTy()) {
		return ErrorV("Unable to access relative address as a non-integer type");
	}
	auto varPtr = builder->CreateLoad(var);
	llvm::LoadInst* derefVar = builder->CreateLoad(builder->CreateGEP(varPtr, offset)); //offset the ptr
	if(e->field) { //deref struct type and resolving field
		llvm::Type* type = getPointedType(derefVar->getPointerOperand());
		if(type->isStructTy()) {
			std::string typeString = type->getStructName();
			std::string fieldName = e->field->name;
			return getStructField(typeString, fieldName, derefVar->getPointerOperand());
		}
		else {
			return ErrorV("Unable to use dot operator on dereferenced non-struct type");
		}
	}
	return derefVar; //pointer expression dereferenced
}

/*===============================AddressOfExpression================================*/
llvm::Value* CodeGenVisitor::visitAddressOfExpression(AddressOfExpression* e) {
	if(!e)
		return ErrorV("Unable to evaluate Address Expression");
	llvm::AllocaInst* var = namedValues[e->ident->name];
	if(!var) {
		return ErrorV("Unable to evaluate variable");
	}
	if(!e->offsetExpression) { //if no offset return the stack pointer
		return var;
	}
	llvm::Value* offset = e->offsetExpression->acceptVisitor(this);
	if(!offset) {
		return ErrorV("Unable to evaluate offset for Address Expression");
	}
	if(!getValType(offset)->isIntegerTy()) {
		return ErrorV("Unable to access relative address as a non-integer type");
	}
	auto varPtr = builder->CreateLoad(var);
	return builder->CreateGEP(varPtr, offset); //dereference, offset, and get address
}

/*===============================StructureExpression================================*/
llvm::Value* CodeGenVisitor::visitStructureExpression(StructureExpression* e) {
	if(!e)
		return ErrorV("Unable to evaluate Structure Expression");
	llvm::AllocaInst* var = namedValues[e->ident->name]; //grab alloca
	if(!var) {
		return ErrorV("Unable to evaluate variable");
	}
	if(!getAllocaType(var)->isStructTy()) {
		return ErrorV("Unable to use dot operator on non-struct type");
	}
	std::string fieldName = e->field->name;
	std::string typeString = getAllocaType(var)->getStructName(); //get struct type string 
	return getStructField(typeString, fieldName, var); //get struct field
}

/*===============================ExternStatement================================*/
llvm::Value* CodeGenVisitor::visitExternStatement(ExternStatement* e) {
	llvm::Function* func = theModule->getFunction(e->ident->name);
	if(!func) { //func doesnt exist
		func = generateFunction(e->hasPointerType, e->type->name, e->ident->name, e->args); //define func with no body
	}
	if(!func) {
		return ErrorV("Invalid extern function signature");
	}
	return voidValue;
}

/*=================================AssignmentLHSVisitor================================*/

CodeGenVisitor::AssignmentLHSVisitor::AssignmentLHSVisitor(CodeGenVisitor* c, llvm::Value* right) {
	this->c = c;
	this->right = right;
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitExpression(Expression* e) {
	return c->ErrorV("Unable to assign to left operand as general expression");
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitIdentifier(Identifier* i) {
	llvm::AllocaInst* var = c->namedValues[i->name];
	if(!var) {
		return c->ErrorV("Unable to evaluate identifier left operand in assignment statement");
	}
	if(!right) { //right is NULL
		if(c->getAllocaType(var)->isPointerTy()) { //left is a pointer
			if(c->getAllocaType(var)->getContainedType(0)->isDoubleTy()) {
				right = c->floatNullPointer;
			}
			else if(c->getAllocaType(var)->getContainedType(0)->isIntegerTy()) {
				right = c->intNullPointer;
			}
			else if(c->getAllocaType(var)->getContainedType(0)->isStructTy()) {
				right = c->getNullPointer(c->getAllocaType(var)->getContainedType(0)->getStructName());
			}
			else {
				return c->ErrorV("Unable to assign to unknown pointer type");
			}
			c->builder->CreateStore(right, var);
			return right;
		}
		else {
			return c->ErrorV("Unable to assign evaluated null right operand to non pointer type");
		}
	}
	if(c->getAllocaType(var)->isDoubleTy() && c->getValType(right)->isIntegerTy()) {
		right = c->castIntToFloat(right);
	}
	else if(c->getAllocaType(var) != c->getValType(right)) {
		return c->ErrorV("Unable to assign evaluated right operand of bad type to left operand");
	}
	else if(c->getAllocaType(var)->isPointerTy()) {
		if(c->getAllocaType(var)->getContainedType(0) != c->getPointedType(right)) {
			return c->ErrorV("Unable to assign evaluated right operand of bad pointer type to left operand");
		}
	}
	c->builder->CreateStore(right, var); //store RHS into LHS var
	return right;
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitPointerExpression(PointerExpression* e) { 
	llvm::AllocaInst* var = c->namedValues[e->ident->name];
	if(!var) {
		return c->ErrorV("Unable to evaluate dereferenced identifier left operand in assignment statement");
	}
	llvm::Value* offset = e->offsetExpression->acceptVisitor(c);
	auto varPtr = c->builder->CreateLoad(var);
	llvm::Value* refVar = c->builder->CreateLoad(c->builder->CreateGEP(varPtr, offset))->getPointerOperand(); //deref offset LHS pointer
	if(e->field) {
		llvm::Type* type = c->getPointedType(refVar);
		if(type->isStructTy()) {
			std::string typeString = type->getStructName();
			std::string fieldName = e->field->name;
			refVar = c->getStructField(typeString, fieldName, refVar)->getPointerOperand(); //grab struct field
			if(!right) { //right is NULL
				if(c->getPointedType(refVar)->isPointerTy()) { //LHS field is a pointer
					if(c->getPointedType(refVar)->getContainedType(0)->isDoubleTy()) {
						right = c->floatNullPointer;
					}
					else if(c->getPointedType(refVar)->getContainedType(0)->isIntegerTy()) {
						right = c->intNullPointer;
					}
					else if(c->getPointedType(refVar)->getContainedType(0)->isStructTy()) {
						right = c->getNullPointer(c->getPointedType(refVar)->getContainedType(0)->getStructName());
					}
					else {
						return c->ErrorV("Unable to assign to unknown pointer type");
					}
				}
				else {
					return c->ErrorV("Unable to assign evaluated null right operand to non pointer type");
				}
			}
			else if(c->getValType(right) != c->getPointedType(refVar)) { //LHS field type does not match RHS type
				if(c->getValType(right)->isIntegerTy() && c->getPointedType(refVar)->isDoubleTy()) {
					right = c->castIntToFloat(right);
				}
				else {
					return c->ErrorV("Dereferenced left operand field is assigned to right operand of incorrect type");
				}
			}	
		}
		else {
			return c->ErrorV("Unable to use dot operator on dereferenced non-struct type");
		}
	}
	else {
		if(!right) { //LHS is non pointer and RHS is pointer
			return c->ErrorV("Unable to assign NULL pointer to left operand of non-pointer type");
		}
		if(c->getValType(right) != c->getPointedType(refVar)) { //LHS deref type does not match RHS type
			if(c->getValType(right)->isIntegerTy() && c->getPointedType(refVar)->isDoubleTy()) {
				right = c->castIntToFloat(right);
			}
			else {
				return c->ErrorV("Dereferenced left operand is assigned to right operand of incorrect type");
			}
		}
	}
	c->builder->CreateStore(right, refVar); //store RHS into deref LHS type/field
	return right;	
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitAddressOfExpression(AddressOfExpression* e) {
	return c->ErrorV("Unable to assign a value to the reference of an identifier");
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitStructureExpression(StructureExpression* e) {
	llvm::AllocaInst* var = c->namedValues[e->ident->name];
	if(!var) {
		return c->ErrorV("Unable to evaluate accessed field of struct for left operand in assignment statement");
	}
	if(!c->getAllocaType(var)->isStructTy()) {
		return c->ErrorV("Unable to use dot operator on non-struct type");
	}
	std::string typeString = c->getAllocaType(var)->getStructName();
	std::string fieldName = e->field->name;
	llvm::Value* structFieldRef = c->getStructField(typeString, fieldName, var)->getPointerOperand(); //get LHS field
	if(!right) {
		if(c->getPointedType(structFieldRef)->isPointerTy()) { //LHS field is a pointer
			if(c->getPointedType(structFieldRef)->getContainedType(0)->isDoubleTy()) {
				right = c->floatNullPointer;
			}
			else if(c->getPointedType(structFieldRef)->getContainedType(0)->isIntegerTy()) {
				right = c->intNullPointer;
			}
			else if(c->getPointedType(structFieldRef)->getContainedType(0)->isStructTy()) {
				right = c->getNullPointer(c->getPointedType(structFieldRef)->getContainedType(0)->getStructName());
			}
			else {
				return c->ErrorV("Unable to assign to unknown pointer type");
			}
		}
		else {
			return c->ErrorV("Unable to assign evaluated null right operand to non pointer type");
		}
	}
	else if(c->getValType(right) != c->getPointedType(structFieldRef)) { //LHS field does not match RHS type
		if(c->getValType(right)->isIntegerTy() && c->getPointedType(structFieldRef)->isDoubleTy()) {
			right = c->castIntToFloat(right);
		}
		else {
			return c->ErrorV("Dereferenced left operand is assigned to right operand of incorrect type");
		}
	}
	c->builder->CreateStore(right, structFieldRef); //store RHS into LHS field
	return right;
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitNode(Node* n) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitStatement(Statement* s) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitInteger(Integer* i) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitFloat(Float* f) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitUnaryOperator(UnaryOperator* u) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitBinaryOperator(BinaryOperator* b) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitBlock(Block* b) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitFunctionCall(FunctionCall* f) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitKeyword(Keyword* k) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitVariableDefinition(VariableDefinition* v) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitStructureDefinition(StructureDefinition* s) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitFunctionDefinition(FunctionDefinition* f) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitStructureDeclaration(StructureDeclaration* s) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitExpressionStatement(ExpressionStatement* e) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitReturnStatement(ReturnStatement* r) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitAssignStatement(AssignStatement* a) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitIfStatement(IfStatement* i) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitExternStatement(ExternStatement* e) {return nullptr;}
llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitNullLiteral(NullLiteral* n) {return nullptr;}
