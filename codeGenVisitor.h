#ifndef __CODE_GEN_VISIT_H
#define __CODE_GEN_VISIT_H

#include "node.h"

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
	BOP_AND,
	BOP_DOT
};

class Visitor {
public:
	virtual llvm::Value* visitNode(Node* n) =0;
	virtual llvm::Value* visitExpression(Expression* e) =0;
	virtual llvm::Value* visitStatement(Statement* s) =0;
	virtual llvm::Value* visitInteger(Integer* i) =0;
	virtual llvm::Value* visitFloat(Float* f) =0;
	virtual llvm::Value* visitIdentifier(Identifier* i) =0;
	virtual llvm::Value* visitNullaryOperator(NullaryOperator* n) =0;
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
	virtual llvm::Value* visitReferenceExpression(ReferenceExpression* e) =0;
	virtual llvm::Value* visitExternStatement(ExternStatement* e) =0;
};

class CodeGenVisitor : public Visitor {
private:
	bool error;
	llvm::LLVMContext* context;
	std::unique_ptr<llvm::IRBuilder<true, llvm::NoFolder>> builder;
	std::unique_ptr<llvm::Module> theModule;
	std::unique_ptr<llvm::orc::KaleidoscopeJIT> forkJIT;
	std::map<std::string, llvm::AllocaInst*> namedValues;
	std::map<std::string, Binops> switchMap;
	void populateSwitchMap();
	llvm::Value* castIntToFloat(llvm::Value* val);
	llvm::Value* castIntToBoolean(llvm::Value* val);
	llvm::Value* castFloatToBoolean(llvm::Value* val);
	llvm::Value* castBooleantoInt(llvm::Value* val);
	llvm::Type* getValType(llvm::Value* val);
	llvm::Type* getPointedType(llvm::Value* val);
	llvm::Type* getFuncRetType(llvm::Function* func);
	llvm::Type* getAllocaType(llvm::AllocaInst* alloca);
	llvm::Value* ErrorV(const char* str);
	llvm::Function* generateFunction(bool hasPointerType, std::string returnType, std::string name, std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* arguments);
	llvm::AllocaInst* createAlloca(llvm::Function* func, llvm::Type* type, const std::string &name);
public:
	CodeGenVisitor(std::string name);
	llvm::LLVMContext* getLLVMContext();
	void executeMain();
	void printModule();
	llvm::Value* visitNode(Node* n);
	llvm::Value* visitExpression(Expression* e);
	llvm::Value* visitStatement(Statement* s);
	llvm::Value* visitInteger(Integer* i);
	llvm::Value* visitFloat(Float* f);
	llvm::Value* visitIdentifier(Identifier* i);
	llvm::Value* visitNullaryOperator(NullaryOperator* n);
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
    llvm::Value* visitReferenceExpression(ReferenceExpression* r);
	llvm::Value* visitExternStatement(ExternStatement* e);
};

#endif
