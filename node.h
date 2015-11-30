
//Declarations of the AST objects, should model the language

#ifndef __NODE_H
#define __NODE_H

//#define GC_DEBUG 1
#define YYDEBUG 1

#include "llvm/Support/ManagedStatic.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/Orc/GlobalMappingLayer.h"
#include "llvm-c/Core.h"
#include "KaleidoscopeJIT.h"
#include <unordered_map>
#include <iterator>
#include <cctype>
#include <cstdio>
#include <cassert>
#include <string.h>
#include <vector>
#include <cstdint>
#include "./gc/include/gc.h"
#include "./gc/include/gc_cpp.h"
#include "./gc/include/gc_allocator.h"
#include "lib.h"

//Externs
extern void yyerror(const char* c);

//Things defined here
class Node;
class Expression;
class Statement;
class Integer;
class Float;
class Identifier;
class UnaryOperator;
class BinaryOperator;
class Block;
class FunctionCall;
class NullLiteral;
class Keyword;
class VariableDefinition;
class StructureDefinition;
class FunctionDefinition;
class StructureDeclaration;
class ExpressionStatement;
class ReturnStatement;
class AssignStatement;
class IfStatement;
class ASTVisitor;
class StatementVisitor;
class CodeGenVisitor;
class LambdaReconVisitor;
class SymbolTable;
class TypeTable;
class ExternStatement;
class PointerExpression;
class AddressOfExpression;
class StructureExpression;

//Symbol Table
enum IdentType {
  VARIABLE,
  STRUCTURE,
  FUNCTION,
  INVALID
};

extern SymbolTable sym_table;

//Visitor type declarations
#include "codeGenVisitor.h"
#include "statementVisitor.h"

/*===================================Node===================================*/
class Node : public gc {
public:
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual Expression* acceptVisitor(LambdaReconVisitor* v);
};

/*================================Expression================================*/
class Expression : public Node {
public:
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual Expression* acceptVisitor(LambdaReconVisitor* v);
};

/*================================Statement=================================*/
class Statement : public Node {
public:
	Statement();
	virtual void describe() const;
	virtual bool alreadyExistsInSymbolTable() const { return false; } //No RTTI needed in parser
	virtual bool alreadyExistsInLocalSymbolTable() const { return false; }
	virtual void insertIntoSymbolTable() { return; }
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual void acceptVisitor(StatementVisitor* v);
	virtual Expression* acceptVisitor(LambdaReconVisitor* v);
protected:
	bool commit;
};

/*=================================Integer==================================*/
class Integer : public Expression {
public:
	int64_t value;
	Integer(int64_t value);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*==================================Float===================================*/
class Float : public Expression {
public:
	double value;
	Float(double value);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*================================Identifier================================*/
class Identifier : public Expression {
public:
	char* name;
	Identifier(char* name);
        bool declaredAsVar() const;
	bool declaredAsFunc() const;
	bool declaredAtAll() const;
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*==============================UnaryOperator===============================*/
class UnaryOperator : public Expression {
public:
	char* op;
	Expression* exp;
	UnaryOperator(char* op, Expression* exp);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*==============================BinaryOperator==============================*/
class BinaryOperator : public Expression {
public:
	char* op;
	Expression* left;
	Expression* right;
	BinaryOperator(Expression* left, char* op, Expression* right);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*==================================Block===================================*/
//Must be an Expression to be contained in other blocks, but
//  strictly speaking, a Block is a collection of statements
class Block : public Node {
public:
	Block();
	Block(std::vector<Statement*,gc_allocator<Statement*>>* statements);
	std::vector<Statement*,gc_allocator<Statement*>>* statements;
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual void acceptVisitor(StatementVisitor* v);
};

/*===============================FunctionCall===============================*/
class FunctionCall : public Expression {
public:
	Identifier* ident;
	std::vector<Expression*,gc_allocator<Expression*>>* args;
	FunctionCall(Identifier* ident, std::vector<Expression*, gc_allocator<Expression*>>* args);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*===============================NullLiteral===============================*/
class NullLiteral : public Expression {
public:
	NullLiteral();
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*=================================Keyword==================================*/
//Keyword refers to the type of a declaration, not language keywords
class Keyword : public Node {
public:
	char* name;
	Keyword(char* name);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*============================VariableDefinition============================*/
class VariableDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	Expression* exp;
	bool hasPointerType;
	VariableDefinition();
	VariableDefinition(Keyword* type, Identifier* ident, Expression* exp, bool isPointer);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	virtual const char* stringType() const;
	virtual void insertIntoSymbolTable();
	virtual bool alreadyExistsInSymbolTable() const;
	virtual bool alreadyExistsInLocalSymbolTable() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual void acceptVisitor(StatementVisitor* v);
};

/*===========================StructureDefinition============================*/
class StructureDefinition : public Statement {
public:
	Identifier* ident;
	Block* block;
	StructureDefinition(Identifier* ident,Block* block);
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>> getVariables() const;
	std::vector<StructureDeclaration*,gc_allocator<StructureDeclaration*>> getStructs() const;
	bool validate() const;
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual void acceptVisitor(StatementVisitor* v);
};

/*============================FunctionDefinition============================*/
class FunctionDefinition : public Statement {
public:
	Keyword* type;
	Identifier* user_type;
	Identifier* ident;
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* args;
	Block* block;
	bool hasPointerType;
	FunctionDefinition(Keyword* type, Identifier* ident, std::vector<VariableDefinition*,
		gc_allocator<VariableDefinition*>>* args,
	Block* block, bool hasPointerType);
	FunctionDefinition(Identifier* user_type, Identifier* ident, std::vector<VariableDefinition*,
		gc_allocator<VariableDefinition*>>* args,
	Block* block, bool hasPointerType);
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	bool validate() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*==========================StructureDeclaration============================*/
//C-like declaration (not definition) of a structure
class StructureDeclaration : public VariableDefinition {
public:
	Identifier* user_type; //Keyword type is always null
	StructureDeclaration(Identifier* type,Identifier* ident,bool hasPointerType);
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	bool validate() const;
	virtual const char* stringType() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual void acceptVisitor(StatementVisitor* v);
};

/*===========================ExpressionStatement============================*/
//Completed expressions and blocks become statements through this type
class ExpressionStatement : public Statement {
public:
	Expression* exp;
	ExpressionStatement(Expression* exp);
	virtual void describe() const;
	virtual bool lambdable() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*=============================ReturnStatement==============================*/
//C-like return statement AST object
class ReturnStatement : public Statement {
public:
	Expression* exp;
	ReturnStatement(Expression* exp);
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*=============================AssignStatement==============================*/
//C-like assignment of a variable
class AssignStatement : public Statement {
public:
	Expression* target;
	Expression* valxp;
	AssignStatement(Expression* target,Expression* valxp);
	virtual void describe() const;
	virtual bool lambdable() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
	virtual Expression* acceptVisitor(LambdaReconVisitor* v);
};

/*===============================IfStatement================================*/
//C-like if statement, without else clause
class IfStatement : public Statement {
public:
	Expression* exp;
	Block* block;
	Block* else_block;
	IfStatement(Expression* exp,Block* block,Block* else_block);
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*===============================ExternStatement================================*/
//Extern declaration of a function in the standard library
class ExternStatement : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	bool hasPointerType;
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* args;
	ExternStatement(Keyword* type,Identifier* ident,
	  std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* args, bool hasPointerType);
	virtual void setCommit(const bool& commit);
	virtual bool statementCommits() const;
	virtual bool lambdable() const;
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*===============================SymbolTable================================*/
//Stack for identifier verification
class SymbolTable : public gc {
public:
	SymbolTable();
	void insert(const char* ident,IdentType type);
	IdentType at(const char* ident) const;
	bool check(const char* ident,IdentType type) const;
	bool check(const char* ident) const;
	bool checkLocal(const char* ident,IdentType type) const;
	bool checkLocal(const char* ident) const;
	void push();
	void pop();
private:
	std::vector<std::unordered_map<std::string,IdentType>> frames;
};

/*===============================TypeTable================================*/
//Table listing user-defined types
class TypeTable : public gc {
public:
	TypeTable();
	void insert(const char* ident);
	bool check(const char* ident) const;
private:
	std::vector<std::string> types;
};

/*===============================PointerExpression================================*/
class PointerExpression : public Expression {
public:
	Expression* offsetExpression;
	Identifier* ident; //Ident is always a pointer type
	Identifier* field;
	bool usesDirectValue() const; //Not referenceing
	bool referencingStruct() const;
	PointerExpression(Identifier* ident, Expression* offsetExpression, Identifier* field);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*===============================AddressOfExpression================================*/
class AddressOfExpression : public Expression {
public:
	Expression* offsetExpression; //&ident[offsetExpression]
	Identifier* ident; //Always acting on the direct value of ident
	AddressOfExpression(Identifier* ident, Expression* offsetExpression);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

/*===============================StructureExpression================================*/
class StructureExpression : public Expression {
public:
	Identifier* field;
	Identifier* ident; //structure ident
	StructureExpression(Identifier* ident,Identifier* field);
	virtual void describe() const;
	virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(ASTVisitor* v);
};

#endif
