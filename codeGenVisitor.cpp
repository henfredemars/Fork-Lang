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
	return getBuilder()->CreateSIToFP(val, getBuilder()->getDoubleTy());
}

llvm::Value* CodeGenVisitor::castIntToBoolean(llvm::Value* val) {
	return getBuilder()->CreateICmpNE(val, llvm::ConstantInt::get(*getContext(), llvm::APInt(64, 0, true)));
}

llvm::Value* CodeGenVisitor::castFloatToBoolean(llvm::Value* val) {
	return getBuilder()->CreateFCmpONE(val, llvm::ConstantFP::get(*getContext(), llvm::APFloat(0.0)));
}

llvm::Value* CodeGenVisitor::castBooleantoInt(llvm::Value* val) {
	return getBuilder()->CreateZExtOrBitCast(val, getBuilder()->getInt64Ty());
}

llvm::Value* CodeGenVisitor::castPointerToInt(llvm::Value* val) {
	return getBuilder()->CreatePtrToInt(val, getBuilder()->getInt64Ty());
}

llvm::Value* CodeGenVisitor::castIntToPointer(llvm::Value* val) {
	return getBuilder()->CreateIntToPtr(val, llvm::Type::getInt64PtrTy(*getContext()));
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
	auto varptr = getBuilder()->CreateLoad(var)->getPointerOperand(); //get pointer to struct variable
	return getBuilder()->CreateLoad(getBuilder()->CreateStructGEP(currStruct, varptr, index)); //return struct field
}

llvm::Type* CodeGenVisitor::getTypeFromString(std::string typeName, bool isPointer, bool allowsVoid) {
	if(isPointer) { 
		if(typeName == "float") {
			return llvm::Type::getDoublePtrTy(*getContext());
		}
		else if(typeName == "int") {
			return llvm::Type::getInt64PtrTy(*getContext());
		}
		else if(typeName == "void") {
			return llvm::PointerType::get(llvm::IntegerType::get(*getContext(), 64), 0); //i64* pointer called void* for extern purposes
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
			return getBuilder()->getDoubleTy(); 
		}
		else if(typeName == "int") {
			return getBuilder()->getInt64Ty();
		}
		else if(typeName == "void") {
			if(allowsVoid) {
				return getBuilder()->getVoidTy();
			}
			return (llvm::Type*) ErrorV("Invalid type void detected");
		}
		else if(structTypes.find(typeName) != structTypes.end()) {
			return std::get<0>(structTypes.find(typeName)->second); //return struct type
		}
	}
	return (llvm::Type*) ErrorV("Invalid type detected");
}

llvm::LLVMContext* CodeGenVisitor::getContext() {
	if(insideLambda) {
		return lambdaContext;
	}
	return mainContext;
}

llvm::IRBuilder<true, llvm::NoFolder>* CodeGenVisitor::getBuilder() {
	if(insideLambda) {
		return lambdaBuilder.get();
	}
	return mainBuilder.get();
}

llvm::Module* CodeGenVisitor::getModule() {
	if(insideLambda) {
		return lambdaModule.get();
	}
	return mainModule.get();
}

llvm::Value* CodeGenVisitor::getVoidValue() {
	if(insideLambda) {
		return lambdaVoidValue;
	}
	return mainVoidValue;
}

llvm::Constant* CodeGenVisitor::getIntNullPointer() {
	if(insideLambda) {
		return lambdaIntNullPointer;
	}
	return mainIntNullPointer;
}

llvm::Constant* CodeGenVisitor::getFloatNullPointer() {
	if(insideLambda) {
		return lambdaFloatNullPointer;
	}
	return mainFloatNullPointer;
}

llvm::Value* CodeGenVisitor::makeSched(llvm::Type* type) {
	if(type->isIntegerTy()) {
		strcpy(lambdaKeyword, "int");
	}
	else if(type->isDoubleTy()) {
		strcpy(lambdaKeyword, "float");
	}
	else {
		return ErrorV("Not yet implemented closure assignment to pointer or struct types");
	}
	//store recon assignment TODO
	return nullptr;
}

llvm::Function* CodeGenVisitor::generateFunction(bool hasPointerType, std::string returnType, std::string name, std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* arguments) {
	llvm::FunctionType* funcType = nullptr;
	llvm::Function* func = nullptr;
	std::vector<llvm::Type*> inputArgs;
	for(auto it = arguments->begin(), end = arguments->end(); it < end; ++it) {
		auto argument = *it;
		llvm::Type* type = getTypeFromString(argument->stringType(), argument->hasPointerType, false); //grab int, float, int*, float*, struct, struct*, void* type
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
	func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, getModule()); //create function with functype and use external
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
	recon = false; //lambda
	executeCommit = true;
	populateSwitchMap();
	mainContext = llvm::unwrap(LLVMContextCreate());
	lambdaContext = llvm::unwrap(LLVMContextCreate());
	mainJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
	lambdaJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
	mainModule = llvm::make_unique<llvm::Module>(name, *mainContext);
	mainModule->setDataLayout(mainJIT->getTargetMachine().createDataLayout()); //set module for tracking and execution
	mainBuilder = llvm::make_unique<llvm::IRBuilder<true, llvm::NoFolder>>(*mainContext); //set builder for IR insertion
	mainVoidValue = llvm::ReturnInst::Create(*mainContext); 
	lambdaVoidValue = llvm::ReturnInst::Create(*lambdaContext); 
	mainFloatNullPointer = llvm::Constant::getNullValue(llvm::Type::getDoublePtrTy(*mainContext));
	lambdaFloatNullPointer = llvm::Constant::getNullValue(llvm::Type::getDoublePtrTy(*lambdaContext));
	mainIntNullPointer = llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(*mainContext));
	lambdaIntNullPointer = llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(*lambdaContext)); // set default void and nullptr values, struct has to be retrieved
}

void CodeGenVisitor::executeMain() {
	if(!error) {
		auto handle = mainJIT->addModule(std::move(mainModule)); // JIT the module
		auto mainSymbol = mainJIT->findSymbol("main"); 
		assert(mainSymbol && "No code to execute, include a main function");
		void (*func)() = (void (*)())(intptr_t)mainSymbol.getAddress(); //grab address of main
	    printf("\nExecuting main function...\n");
	    func(); //execute main
	    printf("---> main() returns: void\n");
	    delete mainVoidValue;
	    delete lambdaVoidValue;
	    LLVMContextDispose(llvm::wrap(lambdaContext));
	    LLVMContextDispose(llvm::wrap(mainContext));
	    mainJIT->removeModule(handle);
	}
	else {
		printf("Main execution halted due to errors\n");
	}
}

void CodeGenVisitor::printModule() const {
	if(!error) {
		mainModule->dump(); //dump IR to stderr that is used
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
	return llvm::ConstantInt::get(*getContext(), llvm::APInt(64, i->value, true));
}

/*==================================Float===================================*/
llvm::Value* CodeGenVisitor::visitFloat(Float* f) {
	return llvm::ConstantFP::get(*getContext(), llvm::APFloat(f->value));
}

/*================================Identifier================================*/
llvm::Value* CodeGenVisitor::visitIdentifier(Identifier* i) {
  llvm::AllocaInst* val = namedValues[i->name]; //find alloca in map
  if (!val)
    return ErrorV("Attempt to generate code for not previously defined variable");
  return getBuilder()->CreateLoad(val, i->name); //load alloca value from map
}

/*=============================NullaryOperator==============================
llvm::Value* CodeGenVisitor::visitNullaryOperator(NullaryOperator* n) {
	if(*n->op == ';') {
		//commit action 
		return getVoidValue();
	}
	return ErrorV("Invalid nullary operator");
}*/

/*==============================UnaryOperator===============================*/
llvm::Value* CodeGenVisitor::visitUnaryOperator(UnaryOperator* u) {
	llvm::Value* expr = u->exp->acceptVisitor(this);
	if(!expr) { //NULL applied to unary operator
		expr = getIntNullPointer();
	}
	if(getValType(expr)->isVoidTy()) {
		return ErrorV("Unary operator applied to void type");
	}
	else if(getValType(expr)->isPointerTy()) { //pointer applied to unary op
		expr = castPointerToInt(expr);
		switch(*u->op) { //pointer acts as int then is returned as pointer
			case '-':
			return castIntToPointer(getBuilder()->CreateMul(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, -1, true)), expr));
			case '!':
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateNot(castIntToBoolean(expr)))); //casted to boolean returned as pointer
			default:
			return ErrorV("Invalid unary operator found applied to pointer type");
		}	
	}
	else if(getValType(expr)->isDoubleTy()) { //float applied to unary op
		switch(*u->op) {
			case '-':
			return getBuilder()->CreateFMul(llvm::ConstantFP::get(*getContext(), llvm::APFloat(-1.0)), expr);
			case '!':
			return castBooleantoInt(getBuilder()->CreateNot(castFloatToBoolean(expr)));
			default:
			return ErrorV("Invalid unary operator found applied to float type");
		}
	}
	else if (getValType(expr)->isIntegerTy()) { //integer applied to unary op
		switch(*u->op) {
			case '-':
			return getBuilder()->CreateMul(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, -1, true)), expr);
			case '!':
			return castBooleantoInt(getBuilder()->CreateNot(castIntToBoolean(expr)));
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
		left = getIntNullPointer();
	}
	if(!right) {
		right = getIntNullPointer();
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
			return castIntToPointer(getBuilder()->CreateAdd(left, right));
			case BOP_MINUS:
			return castIntToPointer(getBuilder()->CreateSub(left, right));
			case BOP_MULT:
			return castIntToPointer(getBuilder()->CreateMul(left, right));
			case BOP_DIV:
			return castIntToPointer(getBuilder()->CreateSDiv(left, right));
			case BOP_NEQ:
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateICmpNE(left, right)));
			case BOP_EQ:
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateICmpEQ(left, right)));
			case BOP_GTE:
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateICmpSGE(left, right)));
			case BOP_LTE:
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateICmpSLE(left, right)));
			case BOP_GT:
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateICmpSGT(left, right)));
			case BOP_LT:
			return castIntToPointer(castBooleantoInt(getBuilder()->CreateICmpSLT(left, right)));
			case BOP_OR:
			return castIntToPointer(getBuilder()->CreateOr(castIntToBoolean(left), castIntToBoolean(right))); //cast to boolean as input
			case BOP_AND:
			return castIntToPointer(getBuilder()->CreateAnd(castIntToBoolean(left), castIntToBoolean(right)));
			return ErrorV("Attempt to generate code for not yet implemented dot binary operator");
			default:
			return ErrorV("Invalid binary operator applied to pointer and integer or pointer type");
		}
	}
	else if(getValType(left)->isDoubleTy()) { //both operands are floats
		switch (switchMap.find(b->op)->second) {
			case BOP_PLUS:
			return getBuilder()->CreateFAdd(left, right);
			case BOP_MINUS:
			return getBuilder()->CreateFSub(left, right);
			case BOP_MULT:
			return getBuilder()->CreateFMul(left, right);
			case BOP_DIV:
			return getBuilder()->CreateFDiv(left, right);
			case BOP_NEQ:
			return castBooleantoInt(getBuilder()->CreateFCmpONE(left, right));
			case BOP_EQ:
			return castBooleantoInt(getBuilder()->CreateFCmpOEQ(left, right));
			case BOP_GTE:
			return castBooleantoInt(getBuilder()->CreateFCmpOGE(left, right));
			case BOP_LTE:
			return castBooleantoInt(getBuilder()->CreateFCmpOLE(left, right));
			case BOP_GT:
			return castBooleantoInt(getBuilder()->CreateFCmpOGT(left, right));
			case BOP_LT:
			return castBooleantoInt(getBuilder()->CreateFCmpOLT(left, right));
			case BOP_OR:
			return castBooleantoInt(getBuilder()->CreateOr(castFloatToBoolean(left), castFloatToBoolean(right)));
			case BOP_AND:
			return castBooleantoInt(getBuilder()->CreateAnd(castFloatToBoolean(left), castFloatToBoolean(right)));
			default:
			return ErrorV("Invalid binary operator applied to float types");
		}
	}
	else if(getValType(left)->isIntegerTy()) { //both operands are ints
		switch (switchMap.find(b->op)->second) {
			case BOP_PLUS:
			return getBuilder()->CreateAdd(left, right);
			case BOP_MINUS:
			return getBuilder()->CreateSub(left, right);
			case BOP_MULT:
			return getBuilder()->CreateMul(left, right);
			case BOP_DIV:
			return getBuilder()->CreateSDiv(left, right);
			case BOP_NEQ:
			return castBooleantoInt(getBuilder()->CreateICmpNE(left, right));
			case BOP_EQ:
			return castBooleantoInt(getBuilder()->CreateICmpEQ(left, right));
			case BOP_GTE:
			return castBooleantoInt(getBuilder()->CreateICmpSGE(left, right));
			case BOP_LTE:
			return castBooleantoInt(getBuilder()->CreateICmpSLE(left, right));
			case BOP_GT:
			return castBooleantoInt(getBuilder()->CreateICmpSGT(left, right));
			case BOP_LT:
			return castBooleantoInt(getBuilder()->CreateICmpSLT(left, right));
			case BOP_OR:
			return getBuilder()->CreateOr(castIntToBoolean(left), castIntToBoolean(right));
			case BOP_AND:
			return getBuilder()->CreateAnd(castIntToBoolean(left), castIntToBoolean(right));
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
			executeCommit = true;
		}
		for(size_t i = 0, end = b->statements->size(); i != end; ++i) {
			auto statement = b->statements->at(i);
			bool commits = true;
			if(!insideLambda) { //if outside lambda, check if lambda must be created
				commits = commitVector.at(i);
			}
			if(!commits) { //if must create lambda
				if(executeCommit) {
					executeCommit = false;
					char* cid = (char *)GC_MALLOC_ATOMIC(4); 
					strcpy(cid, "cid");
					char* makeContextName = (char *)GC_MALLOC_ATOMIC(15); 
					strcpy(makeContextName, "__make_context");
					char* keyword = (char *)GC_MALLOC_ATOMIC(4); 
					strcpy(keyword, "int");
					FunctionCall* makeContext = new FunctionCall(new Identifier(makeContextName), new std::vector<Expression*, gc_allocator<Expression*>>());
					VariableDefinition* cidDef = new VariableDefinition(new Keyword(keyword), new Identifier(cid), makeContext, false);
					currCid = cidDef->acceptVisitor(this);
					currId = 0;
					//make cid that identifies this thread group
				}
				statement->setCommit(true);
				//create env struct type
				llvm::StructType* currStruct = llvm::StructType::create(*getContext(), "env"); //create env struct type
				std::vector<std::string> stringVec;
				std::vector<llvm::Type*> types;
				std::vector<llvm::Value*> vals;
				for(auto it = namedValues.begin(), end = namedValues.end(); it != end; ++it) {
					stringVec.push_back(it->first);
					types.push_back(getAllocaType(it->second));
					vals.push_back(getBuilder()->CreateLoad(it->second, it->first));
				}
				currStruct->setBody(types); //insert type list into env
				structTypes.insert(std::make_pair("env", std::make_tuple(currStruct, stringVec))); //add env and fields to struct list
				//create env
				llvm::AllocaInst* alloca = createAlloca(getBuilder()->GetInsertBlock()->getParent(), currStruct, "e0");
				llvm::Constant* structDec = llvm::ConstantAggregateZero::get(currStruct);
				getBuilder()->CreateStore(structDec, alloca);
				namedValues.insert(std::make_pair("e0", alloca));
				auto loadVal = getBuilder()->CreateLoad(alloca);
				for(size_t i = 0, end = vals.size(); i != end; ++i) {
					auto structFieldRef = getStructField("env", stringVec.at(i), alloca)->getPointerOperand();
					getBuilder()->CreateStore(vals.at(i), structFieldRef);
				}
				auto copyValues = namedValues; //clone map
				auto ip = getBuilder()->saveAndClearIP(); //store block insertion point
				char* envType = (char *)GC_MALLOC_ATOMIC(5); 
				strcpy(envType, "void");
				char* envName = (char *)GC_MALLOC_ATOMIC(3); 
				strcpy(envName, "e0");
				char* identifier = (char *)GC_MALLOC_ATOMIC(32);
				std::ostringstream ss;
				ss << "lambda" << lambdaNum++;
				strcpy(identifier, (ss.str()).c_str()); // name mangle the lambda
				auto lambdaStatements = new std::vector<Statement*,gc_allocator<Statement*>>();
				LambdaReconVisitor* lambdaVisitor = new LambdaReconVisitor(this);
				statement->acceptVisitor(lambdaVisitor);
				auto exprLHS = lambdaVisitor->getLHS();
				auto exprRHS = lambdaVisitor->getRHS();
				//get LHS and RHS
				if(!exprLHS) {
					lambdaStatements->push_back(statement);
				}
				lambdaStatements->push_back(new ReturnStatement(exprRHS)); //make inserted statements
				lambdaKeyword = (char *)GC_MALLOC_ATOMIC(6); 
				//make recon vector for assign statement
				if(!exprLHS) {
					strcpy(lambdaKeyword, "void");
					reconVector.push_back(std::make_pair(nullptr, nullptr));
				}
				else {
					recon = true;
					AssignStatement* reconAssign = new AssignStatement(exprLHS, exprRHS);
					reconAssign->acceptVisitor(this);
					recon = false;
				}
				//pass struct to function def
				insideLambda = true;
				lambdaModule = llvm::make_unique<llvm::Module>(identifier, *lambdaContext);
				lambdaModule->setDataLayout(lambdaJIT->getTargetMachine().createDataLayout());
				lambdaBuilder = llvm::make_unique<llvm::IRBuilder<true, llvm::NoFolder>>(*lambdaContext);
				auto envArg = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
				envArg->push_back(new StructureDeclaration(new Identifier(envType), new Identifier(envName), true)); //add void* e0 env argument
				FunctionDefinition* fd = new FunctionDefinition(new Keyword(lambdaKeyword), new Identifier(identifier), envArg, new Block(lambdaStatements), false);
				fd->acceptVisitor(this);
				if(!error) {
					lambdaModule->dump();
				}
				insideLambda = false;
				getBuilder()->restoreIP(ip); //restore block insertion point
				namedValues = copyValues;
				//make function pointer
				auto handle = lambdaJIT->addModule(std::move(lambdaModule)); // JIT the module
				auto lambdaSymbol = lambdaJIT->findSymbol(identifier); 
				uint64_t lam = lambdaSymbol.getAddress(); //grab address of the lambda
				auto lamInt = llvm::ConstantInt::get(*getContext(), llvm::APInt(64, lam, true));
				auto lamPtr = castIntToPointer(lamInt);
				//pass env as pointer
				std::vector<llvm::Value*> schedVector;
				schedVector.push_back(lamPtr);
				schedVector.push_back(getBuilder()->CreateBitOrPointerCast((new AddressOfExpression(new Identifier(envName), nullptr))->acceptVisitor(this), 
					llvm::PointerType::get(llvm::IntegerType::get(*getContext(), 64), 0)));
				schedVector.push_back(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, currId++, true))); //id increments for next one, currId gives max + 1 in the end
				schedVector.push_back(currCid); //cid
				llvm::Function* lambdaCall = nullptr;
				if(!strcmp(lambdaKeyword, "int")) {
					lambdaCall = getModule()->getFunction("__fork_sched_int");
				}
				else if(!strcmp(lambdaKeyword, "float")) {
					lambdaCall = getModule()->getFunction("__fork_sched_float");
				}
				else if(!strcmp(lambdaKeyword, "void")) {
					lambdaCall = getModule()->getFunction("__fork_sched_void");
				}
				else {
					lambdaCall = (llvm::Function*)ErrorV("Not yet implemented closure assignment to pointer or struct types");
				}
				if(lambdaCall) {
					lastVisited = getBuilder()->CreateCall(lambdaCall, schedVector); //create lambda call
				}
				structTypes.erase("env");
				namedValues.erase("e0");
			}
			else {
				if(!executeCommit && !insideLambda) {
					executeCommit = true;
					--currId;
					for(size_t i = 0, end = reconVector.size(); i != end; ++i) {
						char* reconName = (char *)GC_MALLOC_ATOMIC(15);
						std::vector<llvm::Value*> reconVal;
						if(llvm::Value* refVar = reconVector.at(i).second) {
							llvm::Value* var = getBuilder()->CreateLoad(reconVector.at(i).second);
							if(getValType(var)->isIntegerTy()) {
								strcpy(reconName, "__recon_int");
							}
							else if(getValType(var)->isDoubleTy()) {
								strcpy(reconName, "__recon_float");
							}
							else {
								break;
							}
							reconVal.push_back(var);
							reconVal.push_back(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, 1, true)));
							reconVal.push_back(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, i, true)));
							reconVal.push_back(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, currId, true)));
							reconVal.push_back(currCid);
							llvm::Function* reconFun = getModule()->getFunction(reconName);
							auto reconCall = getBuilder()->CreateCall(reconFun, reconVal);
							getBuilder()->CreateStore(reconCall, refVar);
						}
						else {
							strcpy(reconName, "__recon_void");
							reconVal.push_back(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, i, true)));
							reconVal.push_back(llvm::ConstantInt::get(*getContext(), llvm::APInt(64, currId, true)));
							reconVal.push_back(currCid);
							llvm::Function* reconFun = getModule()->getFunction(reconName);
							getBuilder()->CreateCall(reconFun, reconVal);
						}
					}
					char* destroyContextName = (char *)GC_MALLOC_ATOMIC(18); 
					strcpy(destroyContextName, "__destroy_context");
					auto destroyContextVal = new std::vector<Expression*, gc_allocator<Expression*>>();
					char* cid = (char *)GC_MALLOC_ATOMIC(4); 
					strcpy(cid, "cid");
					destroyContextVal->push_back(new Identifier(cid));
					FunctionCall* destroyContext = new FunctionCall(new Identifier(destroyContextName), destroyContextVal);
					destroyContext->acceptVisitor(this);
					namedValues.erase("cid");
					//delete cid that identifies the previous thread group
				}
					lastVisited = statement->acceptVisitor(this);
			}
		}
	}
	return lastVisited;
}

/*===============================FunctionCall===============================*/
llvm::Value* CodeGenVisitor::visitFunctionCall(FunctionCall* f) {
	llvm::Function* func = getModule()->getFunction(f->ident->name); //search func name in module
	if(!func) { //func name does not exist
		if(insideLambda) {
			auto mainFunc = mainModule->getFunction(f->ident->name); //declare in lambda module
			func = llvm::Function::Create(mainFunc->getFunctionType(), llvm::Function::ExternalLinkage, f->ident->name, getModule());
		}
		else {
			return ErrorV("Unknown function reference");
		}
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
					argument = getIntNullPointer();
				}
				else if(getPointedType(funcArgument)->isDoubleTy()) {
					argument = getFloatNullPointer();
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
	return getBuilder()->CreateCall(func, argVector); //establish function call with name and args
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
	llvm::Function* func = getBuilder()->GetInsertBlock()->getParent();
	std::string type = v->stringType();
	llvm::Value* val = nullptr;
	if(v->hasPointerType) {
		if(v->exp) { //instantiated value
			val = v->exp->acceptVisitor(this);
			if(!val) { //Assign Variable to NULL
				if(type == "int") {
					val = getIntNullPointer();
				}
				else if(type == "float") {
					val = getFloatNullPointer();
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
				val = getIntNullPointer();
			}
			else if(type == "float") {
				val = getFloatNullPointer();
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
				val = llvm::ConstantInt::get(*getContext(), llvm::APInt(64, 0, true));
			}
			else if(type == "float") {
				val = llvm::ConstantFP::get(*getContext(), llvm::APFloat(0.0));
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
  		getBuilder()->CreateStore(val, alloca);
  		return val;
	}
	return ErrorV("Unable to generate value for variable");
}

/*===========================StructureDefinition============================*/
llvm::Value* CodeGenVisitor::visitStructureDefinition(StructureDefinition* s) {
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> vars = s->getVariables();
	std::vector<StructureDeclaration*,gc_allocator<StructureDeclaration*>> structs = s->getStructs();
	llvm::StructType* currStruct = llvm::StructType::create(*getContext(), s->ident->name); //forward declare struct type
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
	return getVoidValue();
}

/*============================FunctionDefinition============================*/
llvm::Value* CodeGenVisitor::visitFunctionDefinition(FunctionDefinition* f) {
	llvm::Function* func = getModule()->getFunction(f->ident->name);
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
	llvm::BasicBlock* block = llvm::BasicBlock::Create(*getContext(), "func", func);
	getBuilder()->SetInsertPoint(block);
	namedValues.clear();
	if(!insideLambda) { //keep variables to allow access to current scope
		for (auto &arg : func->args()) {
			llvm::AllocaInst* alloca = createAlloca(func, arg.getType(), arg.getName());
			if(!alloca) {
				return ErrorV("Unable to create stack variable inside function body for function argument");
			}
	    	getBuilder()->CreateStore(&arg, alloca); // Store init value into alloca
			namedValues.insert(std::make_pair(arg.getName(), alloca));
		} //create alloca for each argument
	}
	else {
		auto env = func->arg_begin();
		auto structTuple = structTypes.find("env")->second;
		llvm::AllocaInst* envAlloca = createAlloca(func, llvm::PointerType::getUnqual(std::get<0>(structTuple)), env->getName());
		if(!envAlloca) {
			return ErrorV("Unable to create stack variable inside lambda body for environment");
		}
		auto environ = getBuilder()->CreateBitOrPointerCast(env, llvm::PointerType::getUnqual(std::get<0>(structTuple)));
		getBuilder()->CreateStore(environ, envAlloca);
		namedValues.insert(std::make_pair(env->getName(), envAlloca));
		auto varList = std::get<1>(structTuple);
		for(size_t i = 0, end = varList.size(); i != end; ++i) {
			llvm::Value* val = getStructField("env", varList.at(i), getBuilder()->CreateLoad(envAlloca));
			llvm::AllocaInst* alloca = createAlloca(func, getValType(val), varList.at(i));
			getBuilder()->CreateStore(val, alloca);
			namedValues.insert(std::make_pair(varList.at(i), alloca));
		}
	}
	llvm::Value* retVal = f->block->acceptVisitor(this);
	return retVal;
}

/*==========================StructureDeclaration============================*/
llvm::Value* CodeGenVisitor::visitStructureDeclaration(StructureDeclaration* s) {
	llvm::Function* func = getBuilder()->GetInsertBlock()->getParent();
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
		return getBuilder()->CreateStore(structPtrDec, alloca); //store structptr in alloca
	}
	llvm::StructType* currStruct = std::get<0>(structTuple);
	llvm::AllocaInst* alloca = createAlloca(func, currStruct, name); //make struct alloca
	if(!alloca) {
		return ErrorV("Unable to create alloca of struct type");
	}
	llvm::Constant* structDec = llvm::ConstantAggregateZero::get(currStruct); //zero all values
	namedValues.insert(std::make_pair(name, alloca));
	return getBuilder()->CreateStore(structDec, alloca); //store struct in alloca
}


/*===========================ExpressionStatement============================*/
llvm::Value* CodeGenVisitor::visitExpressionStatement(ExpressionStatement* e) {
	return e->exp->acceptVisitor(this);	//evaluated but value discarded
}

/*=============================ReturnStatement==============================*/
llvm::Value* CodeGenVisitor::visitReturnStatement(ReturnStatement* r) {
	llvm::Function* func = getBuilder()->GetInsertBlock()->getParent();
	justReturned = true;
	if(r->exp) { //return exp
		if(llvm::Value* retVal = r->exp->acceptVisitor(this)) { 
			if(getValType(retVal)->isVoidTy() && getFuncRetType(func)->isVoidTy()) { //void func returned
				if(getFuncRetType(func)->isVoidTy()) {
					retVal = getBuilder()->CreateRetVoid();
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
				getBuilder()->CreateRet(retVal);
				verifyFunction(*func);
				return retVal;
			}
			else if(getValType(retVal)->isIntegerTy() && getFuncRetType(func)->isDoubleTy()) {
				getBuilder()->CreateRet(castIntToFloat(retVal));
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
				retVal = getBuilder()->CreateRet(getIntNullPointer());
			}
			else if(getFuncRetType(func)->getContainedType(0)->isDoubleTy()) {
				retVal = getBuilder()->CreateRet(getFloatNullPointer());
			}
			else if(getFuncRetType(func)->getContainedType(0)->isStructTy()) {
				retVal = getBuilder()->CreateRet(getNullPointer(getFuncRetType(func)->getContainedType(0)->getStructName()));
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
		llvm::Value* retVal = getBuilder()->CreateRetVoid();
		verifyFunction(*func);
		return retVal;
	}
	return ErrorV("Unable to return bad void type from function");
}

/*=============================AssignStatement==============================*/
llvm::Value* CodeGenVisitor::visitAssignStatement(AssignStatement* a) {
	llvm::Value* right = nullptr;
	if(!recon) {
		right = a->valxp->acceptVisitor(this); //visit RHS
	}
	AssignmentLHSVisitor* leftVisitor = new AssignmentLHSVisitor(this, right); //pass codegenvisitor and RHS to LHS visitor 
	llvm::Value* retVal = a->target->acceptVisitor(leftVisitor); //visit LHS
	if(!retVal && !recon) {
		return ErrorV("Unable to evaluate assignment statement");
	}
	return retVal;
}

/*===============================IfStatement================================*/
llvm::Value* CodeGenVisitor::visitIfStatement(IfStatement* i) {
	llvm::Value* condition = i->exp->acceptVisitor(this);
	if(!condition) { //input NULL
		condition = castPointerToInt(getIntNullPointer());
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
	llvm::Function* func = getBuilder()->GetInsertBlock()->getParent();
	llvm::BasicBlock* thenIf = llvm::BasicBlock::Create(*getContext(), "then", func);
	llvm::BasicBlock* elseIf = llvm::BasicBlock::Create(*getContext(), "else");
	llvm::BasicBlock* mergeIf = llvm::BasicBlock::Create(*getContext(), "if");
	getBuilder()->CreateCondBr(condition, thenIf, elseIf); //branch between blocks
	//insert into then
	getBuilder()->SetInsertPoint(thenIf);
	llvm::Value* ifEval = nullptr;
	if(i->block) {
		justReturned = false;
		ifEval = i->block->acceptVisitor(this);
	}
        if (!justReturned) { //Ugly but at least it's ugly just here
	  getBuilder()->CreateBr(mergeIf);
        } else {
	  justReturned = false;
	}
	thenIf = getBuilder()->GetInsertBlock();
	func->getBasicBlockList().push_back(elseIf);
	//insert into else
	getBuilder()->SetInsertPoint(elseIf);
	llvm::Value* elseEval = nullptr;
	if(i->else_block) {
		justReturned = false;
		elseEval = i->else_block->acceptVisitor(this);
	}
	if (!justReturned) { //Ugly again, but visitor pattern limits
	  getBuilder()->CreateBr(mergeIf);
	} else {
	  justReturned = false;
	}
	elseIf = getBuilder()->GetInsertBlock();
	func->getBasicBlockList().push_back(mergeIf);
	//resolve then, else in mergeIf
	getBuilder()->SetInsertPoint(mergeIf);
	return getVoidValue(); //void val never used
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
	auto varPtr = getBuilder()->CreateLoad(var);
	llvm::LoadInst* derefVar = getBuilder()->CreateLoad(getBuilder()->CreateGEP(varPtr, offset)); //offset the ptr
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
	auto varPtr = getBuilder()->CreateLoad(var);
	return getBuilder()->CreateGEP(varPtr, offset); //dereference, offset, and get address
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
	llvm::Function* func = getModule()->getFunction(e->ident->name);
	if(!func) { //func doesnt exist
		func = generateFunction(e->hasPointerType, e->type->name, e->ident->name, e->args); //define func with no body
	}
	if(!func) {
		return ErrorV("Invalid extern function signature");
	}
	return getVoidValue();
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
	if(c->recon) {
		right = c->makeSched(c->getAllocaType(var));
		c->reconVector.push_back(std::make_pair(nullptr, var));
		return right;
	}
	if(!right) { //right is NULL
		if(c->getAllocaType(var)->isPointerTy()) { //left is a pointer
			if(c->getAllocaType(var)->getContainedType(0)->isDoubleTy()) {
				right = c->getFloatNullPointer();
			}
			else if(c->getAllocaType(var)->getContainedType(0)->isIntegerTy()) {
				right = c->getIntNullPointer();
			}
			else if(c->getAllocaType(var)->getContainedType(0)->isStructTy()) {
				right = c->getNullPointer(c->getAllocaType(var)->getContainedType(0)->getStructName());
			}
			else {
				return c->ErrorV("Unable to assign to unknown pointer type");
			}
			c->getBuilder()->CreateStore(right, var);
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
	c->getBuilder()->CreateStore(right, var); //store RHS into LHS var
	return right;
}

llvm::Value* CodeGenVisitor::AssignmentLHSVisitor::visitPointerExpression(PointerExpression* e) { 
	llvm::AllocaInst* var = c->namedValues[e->ident->name];
	if(!var) {
		return c->ErrorV("Unable to evaluate dereferenced identifier left operand in assignment statement");
	}
	llvm::Value* offset = e->offsetExpression->acceptVisitor(c);
	auto varPtr = c->getBuilder()->CreateLoad(var);
	llvm::Value* refVar = c->getBuilder()->CreateLoad(c->getBuilder()->CreateGEP(varPtr, offset))->getPointerOperand(); //deref offset LHS pointer
	if(e->field) {
		llvm::Type* type = c->getPointedType(refVar);
		if(type->isStructTy()) {
			std::string typeString = type->getStructName();
			std::string fieldName = e->field->name;
			refVar = c->getStructField(typeString, fieldName, refVar)->getPointerOperand(); //grab struct field
			if(c->recon) {
				right = c->makeSched(c->getPointedType(refVar));
				c->reconVector.push_back(std::make_pair(nullptr, refVar));
				return right;
			}
			if(!right) { //right is NULL
				if(c->getPointedType(refVar)->isPointerTy()) { //LHS field is a pointer
					if(c->getPointedType(refVar)->getContainedType(0)->isDoubleTy()) {
						right = c->getFloatNullPointer();
					}
					else if(c->getPointedType(refVar)->getContainedType(0)->isIntegerTy()) {
						right = c->getIntNullPointer();
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
		if(c->recon) {
			right = c->makeSched(c->getPointedType(refVar));
			c->reconVector.push_back(std::make_pair(nullptr, refVar));
			return right;
		}
		if(!right) { //LHS is non pointer and RHS is pointer
			return c->ErrorV("Unable to assign NULL pointer to left operand of non-pointer type");
		}
		else if(c->getValType(right) != c->getPointedType(refVar)) { //LHS deref type does not match RHS type
			if(c->getValType(right)->isIntegerTy() && c->getPointedType(refVar)->isDoubleTy()) {
				right = c->castIntToFloat(right);
			}
			else {
				return c->ErrorV("Dereferenced left operand is assigned to right operand of incorrect type");
			}
		}
	}
	c->getBuilder()->CreateStore(right, refVar); //store RHS into deref LHS type/field
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
	if(c->recon) {
		right = c->makeSched(c->getPointedType(structFieldRef));
		c->reconVector.push_back(std::make_pair(nullptr, structFieldRef));
		return right;
	}
	if(!right) {
		if(c->getPointedType(structFieldRef)->isPointerTy()) { //LHS field is a pointer
			if(c->getPointedType(structFieldRef)->getContainedType(0)->isDoubleTy()) {
				right = c->getFloatNullPointer();
			}
			else if(c->getPointedType(structFieldRef)->getContainedType(0)->isIntegerTy()) {
				right = c->getIntNullPointer();
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
	c->getBuilder()->CreateStore(right, structFieldRef); //store RHS into LHS field
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

LambdaReconVisitor::LambdaReconVisitor(CodeGenVisitor* c) {
	this->c = c;
	exprLHS = nullptr;
	exprRHS = nullptr;
}
Expression* LambdaReconVisitor::getRHS() {
	return exprRHS;
}
Expression* LambdaReconVisitor::getLHS() {
	return exprLHS;
}
Expression* LambdaReconVisitor::visitNode(Node* n) {return nullptr;}
Expression* LambdaReconVisitor::visitExpression(Expression* e) {return nullptr;}
Expression* LambdaReconVisitor::visitStatement(Statement* s) {return nullptr;}
Expression* LambdaReconVisitor::visitAssignStatement(AssignStatement* a) {
	exprLHS = a->target;
	exprRHS = a->valxp;
	return exprRHS;
}