
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
#include "KaleidoscopeJIT.h"
#include <map>
#include <cctype>
#include <cstdio>
#include <cassert>
#include <string.h>
#include <vector>
#include <cstdint>
#include "./gc/include/gc.h"
#include "./gc/include/gc_cpp.h"
#include "./gc/include/gc_allocator.h"

//Externs
extern void yyerror(const char *s);

//Things defined here
class Node;
class Expression;
class Statement;
class Integer;
class Float;
class Identifier;
class NullaryOperator;
class UnaryOperator;
class BinaryOperator;
class Block;
class FunctionCall;
class Keyword;
class VariableDefinition;
class StructureDefinition;
class FunctionDefinition;
class StructureDeclaration;
class ExpressionStatement;
class ReturnStatement;
class AssignStatement;
class IfStatement;
class Visitor;
class CodeGenVisitor;
class SymbolTable;
class ReferenceExpression;

//Symbol Table
enum IdentType {
  VARIABLE,
  STRUCTURE,
  FUNCTION
};

extern SymbolTable sym_table;

//CodeGen type declarations
#include "codeGenVisitor.h"

/*===================================Node===================================*/
class Node : public gc {
public:
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*================================Expression================================*/
class Expression : public Node {
public:
	virtual void describe() const;
        virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*================================Statement=================================*/
class Statement : public Node {
public:
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*=================================Integer==================================*/
class Integer : public Expression {
public:
	int64_t value;
	Integer(int64_t value);
	virtual void describe() const;
        virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*==================================Float===================================*/
class Float : public Expression {
public:
	double value;
	Float(double value);
	virtual void describe() const;
        virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
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
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*=============================NullaryOperator==============================*/
class NullaryOperator : public Expression {
public:
	char* op;
	NullaryOperator(char* op);
	virtual void describe() const;
        virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*==============================UnaryOperator===============================*/
class UnaryOperator : public Expression {
public:
	char* op;
	Expression* exp;
	UnaryOperator(char* op, Expression* exp);
	virtual void describe() const;
        virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
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
	virtual llvm::Value* acceptVisitor(Visitor* v);
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
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*===============================FunctionCall===============================*/
class FunctionCall : public Expression {
public:
	Identifier* ident;
	std::vector<Expression*,gc_allocator<Expression*>>* args;
	FunctionCall(Identifier* ident, std::vector<Expression*, gc_allocator<Expression*>>* args);
	virtual void describe() const;
        virtual bool identsDeclared() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*=================================Keyword==================================*/
//Keyword refers to the type of a declaration, not language keywords
class Keyword : public Node {
public:
	char* name;
	Keyword(char* name);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*============================VariableDefinition============================*/
class VariableDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	Expression* exp;
	VariableDefinition(Keyword* type, Identifier* ident, Expression* exp);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*===========================StructureDefinition============================*/
class StructureDefinition : public Statement {
public:
	Identifier* ident;
	Block* block;
	StructureDefinition(Identifier* ident,Block* block);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*============================FunctionDefinition============================*/
class FunctionDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* args;
	Block* block;
	FunctionDefinition(Keyword* type, Identifier* ident, std::vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	 Block* block);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*==========================StructureDeclaration============================*/
//C-like declaration (not definition) of a structure
class StructureDeclaration : public Statement {
public:
	Identifier* type;
	Identifier* ident;
	StructureDeclaration(Identifier* type,Identifier* ident);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*===========================ExpressionStatement============================*/
//Completed expressions and blocks become statements through this type
class ExpressionStatement : public Statement {
public:
	Expression* exp;
	ExpressionStatement(Expression* exp);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*=============================ReturnStatement==============================*/
//C-like return statement AST object
class ReturnStatement : public Statement {
public:
	Expression* exp;
	ReturnStatement(Expression* exp);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*=============================AssignStatement==============================*/
//C-like assignment of a variable
class AssignStatement : public Statement {
public:
	ReferenceExpression* target;
	Expression* valxp;
	AssignStatement(ReferenceExpression* target,Expression* valxp);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*===============================IfStatement================================*/
//C-like if statement, without else clause
class IfStatement : public Statement {
public:
	Expression* exp;
	Block* block;
	IfStatement(Expression* exp,Block* block);
	virtual void describe() const;
	virtual llvm::Value* acceptVisitor(Visitor* v);
};

/*===============================SymbolTable================================*/
//Stack for identifier verification
class SymbolTable : public gc {
public:
	SymbolTable();
        void insert(const char* ident,IdentType type);
	bool check(const char* ident,IdentType type);
        bool check(const char* ident);
	void push();
        void pop();
private:
	std::vector<std::map<std::string,IdentType>> frames;
};

/*===============================ReferenceExpression================================*/
//Assignable l-expressions
class ReferenceExpression : public Expression {
public:
        Expression* offsetExpression;
        Identifier* ident;
	bool assignsPointerDirectly() const;
        ReferenceExpression(Identifier* ident, Expression* offsetExpression);
        virtual void describe() const;
        virtual bool identsDeclared() const;
        virtual llvm::Value* acceptVisitor(Visitor* v);
};


#endif
