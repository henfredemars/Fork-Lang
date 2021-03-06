#ifndef __CODE_GEN_VISIT_H
#define __CODE_GEN_VISIT_H

#include "node.h"
#include <iostream>
#include <sstream>

//AST visitor

enum Binops {
	BOP_PLUS,
	BOP_MINUS,
	BOP_MULT,
	BOP_DIV,
	BOP_GTE,
	BOP_LTE,
	BOP_LT,
	BOP_GT,
	BOP_NEQ,
	BOP_EQ,
	BOP_OR,
	BOP_AND
};

class ASTVisitor : public gc {
public:
	virtual llvm::Value* visitNode(Node* n) =0;
	virtual llvm::Value* visitExpression(Expression* e) =0;
	virtual llvm::Value* visitStatement(Statement* s) =0;
	virtual llvm::Value* visitInteger(Integer* i) =0;
	virtual llvm::Value* visitFloat(Float* f) =0;
	virtual llvm::Value* visitIdentifier(Identifier* i) =0;
	virtual llvm::Value* visitUnaryOperator(UnaryOperator* u) =0;
	virtual llvm::Value* visitBinaryOperator(BinaryOperator* b) =0;
	virtual llvm::Value* visitBlock(Block* b) =0;
	virtual llvm::Value* visitFunctionCall(FunctionCall* f) =0;
	virtual llvm::Value* visitKeyword(Keyword* k) =0;
	virtual llvm::Value* visitVariableDefinition(VariableDefinition* v) =0;
	virtual llvm::Value* visitStructureDefinition(StructureDefinition* s) =0;
	virtual llvm::Value* visitFunctionDefinition(FunctionDefinition* f) =0;
	virtual llvm::Value* visitStructureDeclaration(StructureDeclaration* s) =0;
	virtual llvm::Value* visitExpressionStatement(ExpressionStatement* e) =0;
	virtual llvm::Value* visitReturnStatement(ReturnStatement* r) =0;
	virtual llvm::Value* visitAssignStatement(AssignStatement* a) =0;
	virtual llvm::Value* visitIfStatement(IfStatement* i) =0;
	virtual llvm::Value* visitPointerExpression(PointerExpression* e) =0;
	virtual llvm::Value* visitAddressOfExpression(AddressOfExpression* e) =0;
	virtual llvm::Value* visitStructureExpression(StructureExpression* e) =0;
	virtual llvm::Value* visitExternStatement(ExternStatement* e) =0;
	virtual llvm::Value* visitNullLiteral(NullLiteral* n) =0;
};

class CodeGenVisitor : public ASTVisitor {
private:
	class AssignmentLHSVisitor : public ASTVisitor {
	private:
		CodeGenVisitor* c;
		llvm::Value* right;
	public:
		AssignmentLHSVisitor(CodeGenVisitor* c, llvm::Value* right);
		llvm::Value* visitNode(Node* n);
		llvm::Value* visitExpression(Expression* e);
		llvm::Value* visitStatement(Statement* s);
		llvm::Value* visitInteger(Integer* i);
		llvm::Value* visitFloat(Float* f);
		llvm::Value* visitIdentifier(Identifier* i);
		llvm::Value* visitUnaryOperator(UnaryOperator* u);
		llvm::Value* visitBinaryOperator(BinaryOperator* b);
		llvm::Value* visitBlock(Block* b);
		llvm::Value* visitFunctionCall(FunctionCall* f);
		llvm::Value* visitKeyword(Keyword* k);
		llvm::Value* visitVariableDefinition(VariableDefinition* v);
		llvm::Value* visitStructureDefinition(StructureDefinition* s);
		llvm::Value* visitFunctionDefinition(FunctionDefinition* f);
		llvm::Value* visitStructureDeclaration(StructureDeclaration* s);
		llvm::Value* visitExpressionStatement(ExpressionStatement* e);
		llvm::Value* visitReturnStatement(ReturnStatement* r);
		llvm::Value* visitAssignStatement(AssignStatement* a);
		llvm::Value* visitIfStatement(IfStatement* i);
		llvm::Value* visitPointerExpression(PointerExpression* e);
		llvm::Value* visitAddressOfExpression(AddressOfExpression* e);
		llvm::Value* visitStructureExpression(StructureExpression* e);
		llvm::Value* visitExternStatement(ExternStatement* e);
		llvm::Value* visitNullLiteral(NullLiteral* n);
	};
	int lambdaNum; //lambda
	bool insideLambda; //lambda
	llvm::Value* currCid; //lambda
	int currId; //lambda
	bool executeCommit; //lambda
	char* lambdaKeyword;
	bool error;
	bool justReturned;
	llvm::LLVMContext* mainContext;
	llvm::LLVMContext* lambdaContext; //lambda
	std::vector<std::pair<llvm::Value*, llvm::Value*>> reconVector;
	std::unique_ptr<llvm::IRBuilder<true, llvm::NoFolder>> mainBuilder;
	std::unique_ptr<llvm::IRBuilder<true, llvm::NoFolder>> lambdaBuilder; //lambda
	std::unique_ptr<llvm::Module> mainModule;
	std::unique_ptr<llvm::Module> lambdaModule; //lambda
	std::unique_ptr<llvm::orc::KaleidoscopeJIT> mainJIT;
	std::unique_ptr<llvm::orc::KaleidoscopeJIT> lambdaJIT; //lambda
	llvm::Value* mainVoidValue;
	llvm::Value* lambdaVoidValue; //lambda
	llvm::Constant* mainIntNullPointer;
	llvm::Constant* lambdaIntNullPointer; //lambda
	llvm::Constant* mainFloatNullPointer;
	llvm::Constant* lambdaFloatNullPointer; //lambda
	std::unordered_map<std::string, llvm::AllocaInst*> namedValues;
	std::unordered_map<std::string, std::tuple<llvm::StructType*, std::vector<std::string>>> structTypes;
	std::unordered_map<std::string, Binops> switchMap;
	llvm::Value* ErrorV(const char* str);
	void populateSwitchMap();
	llvm::Value* castIntToFloat(llvm::Value* val);
	llvm::Value* castIntToBoolean(llvm::Value* val);
	llvm::Value* castFloatToBoolean(llvm::Value* val);
	llvm::Value* castBooleantoInt(llvm::Value* val);
	llvm::Value* castPointerToInt(llvm::Value* val);
	llvm::Value* castIntToPointer(llvm::Value* val);
	llvm::Type* getValType(llvm::Value* val);
	llvm::Type* getPointedType(llvm::Value* val);
	llvm::Type* getFuncRetType(llvm::Function* func);
	llvm::Type* getAllocaType(llvm::AllocaInst* alloca);
	llvm::Constant* getNullPointer(std::string typeName);
	llvm::LoadInst* getStructField(std::string typeString, std::string fieldName, llvm::Value* var);
	llvm::Type* getTypeFromString(std::string typeName, bool isPointer, bool allowsVoid);
	llvm::Module* getModule(); //lambda
	llvm::IRBuilder<true, llvm::NoFolder>* getBuilder(); //lambda
	llvm::LLVMContext* getContext(); //lambda
	llvm::Value* getVoidValue();
	llvm::Constant* getIntNullPointer();
	llvm::Constant* getFloatNullPointer(); 
	llvm::Value* makeSched(llvm::Type* type); //lambda
	llvm::Function* generateFunction(bool hasPointerType, std::string returnType, std::string name, std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* arguments);
	llvm::AllocaInst* createAlloca(llvm::Function* func, llvm::Type* type, const std::string &name);
public:
	bool recon; //lambda
	CodeGenVisitor(std::string name);
	llvm::LLVMContext* getLLVMContext();
	void executeMain();
	void printModule() const;
	llvm::Value* visitNode(Node* n);
	llvm::Value* visitExpression(Expression* e);
	llvm::Value* visitStatement(Statement* s);
	llvm::Value* visitInteger(Integer* i);
	llvm::Value* visitFloat(Float* f);
	llvm::Value* visitIdentifier(Identifier* i);
	llvm::Value* visitUnaryOperator(UnaryOperator* u);
	llvm::Value* visitBinaryOperator(BinaryOperator* b);
	llvm::Value* visitBlock(Block* b);
	llvm::Value* visitFunctionCall(FunctionCall* f);
	llvm::Value* visitKeyword(Keyword* k);
	llvm::Value* visitVariableDefinition(VariableDefinition* v);
	llvm::Value* visitStructureDefinition(StructureDefinition* s);
	llvm::Value* visitFunctionDefinition(FunctionDefinition* f);
	llvm::Value* visitStructureDeclaration(StructureDeclaration* s);
	llvm::Value* visitExpressionStatement(ExpressionStatement* e);
	llvm::Value* visitReturnStatement(ReturnStatement* r);
	llvm::Value* visitAssignStatement(AssignStatement* a);
	llvm::Value* visitIfStatement(IfStatement* i);
	llvm::Value* visitPointerExpression(PointerExpression* r);
	llvm::Value* visitAddressOfExpression(AddressOfExpression* r);
	llvm::Value* visitStructureExpression(StructureExpression* r);
	llvm::Value* visitExternStatement(ExternStatement* e);
	llvm::Value* visitNullLiteral(NullLiteral* n);
};

class LambdaReconVisitor : public gc {
private:
	CodeGenVisitor* c;
	Expression* exprLHS;
	Expression* exprRHS;
public:
	Expression* getRHS();
	Expression* getLHS();
	LambdaReconVisitor(CodeGenVisitor* c);
	Expression* visitNode(Node* n);
	Expression* visitExpression(Expression* e);
	Expression* visitStatement(Statement* s);
	Expression* visitAssignStatement(AssignStatement* a);
};

#endif /* __CODE_GEN_VISIT_H */

