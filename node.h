
//Declarations of the AST objects, should model the language

#ifndef __CLASS_1_H
#define __CLASS_1_H

#include <cstdint>
#include "./gc/include/gc.h"
#include "./gc/include/gc_cpp.h"
#include "./gc/include/gc_allocator.h"
#include "codeGenVisitor.h"

#define GC_DEBUG 1
#define YYDEBUG 1

using namespace std;

//Things defined here
//class Node;
//class Expression;
//class Statement;
//class Integer;
//class Float;
//class Identifier;
//class NullaryOperator;
//class UnaryOperator;
//class BinaryOperator;
//class Block;
//class FunctionCall;
//class Keyword;
//class VariableDefinition;
//class StructureDefinition;
//class FunctionDefinition;
//class StructureDeclaration;
//class ExpressionStatement;
//class ReturnStatement;
//class AssignStatement;
//class IfStatement;

/*===================================Node===================================*/
class Node : public gc {
public:
	virtual void describe() const;
};

/*================================Expression================================*/
class Expression : public Node {
public:
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*================================Statement=================================*/
class Statement : public Node {
public:
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*=================================Integer==================================*/
class Integer : public Expression {
public:
	int64_t value;
	Integer(int64_t value);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*==================================Float===================================*/
class Float : public Expression {
public:
	double value;
	Float(double value);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*================================Identifier================================*/
class Identifier : public Expression {
public:
	char* name;
	Identifier(char* name);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*=============================NullaryOperator==============================*/
class NullaryOperator : public Expression {
public:
	char* op;
	NullaryOperator(char* op);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*==============================UnaryOperator===============================*/
class UnaryOperator : public Expression {
public:
	char* op;
	Expression* exp;
	UnaryOperator(char* op, Expression* exp);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*==============================BinaryOperator==============================*/
class BinaryOperator : public Expression {
public:
	char* op;
	Expression* left;
	Expression* right;
	BinaryOperator(Expression* left, char* op, Expression* right);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*==================================Block===================================*/
//Must be an Expression to be contained in other blocks, but
//  strictly speaking, a Block is a collection of statements
class Block : public Expression {
public:
	Block();
	Block(vector<Statement*,gc_allocator<Statement*>>* statements);
	vector<Statement*,gc_allocator<Statement*>>* statements;
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*===============================FunctionCall===============================*/
class FunctionCall : public Expression {
public:
	Identifier* ident;
	vector<Expression*,gc_allocator<Expression*>>* args;
	FunctionCall(Identifier* ident, vector<Expression*, gc_allocator<Expression*>>* args);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*=================================Keyword==================================*/
//Keyword refers to the type of a declaration, not language keywords
class Keyword : public Node {
public:
	char* name;
	Keyword(char* name);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*============================VariableDefinition============================*/
class VariableDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	Expression* exp;
	VariableDefinition(Keyword* type, Identifier* ident, Expression* exp);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*===========================StructureDefinition============================*/
class StructureDefinition : public Statement {
public:
	Identifier* ident;
	Block* block;
	StructureDefinition(Identifier* ident,Block* block);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*============================FunctionDefinition============================*/
class FunctionDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* args;
	Block* block;
	FunctionDefinition(Keyword* type, Identifier* ident, vector<VariableDefinition*, gc_allocator<VariableDefinition*>>* args,
	 Block* block);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*==========================StructureDeclaration============================*/
//C-like declaration (not definition) of a structure
class StructureDeclaration : public Statement {
public:
	Identifier* type;
	Identifier* ident;
	StructureDeclaration(Identifier* type,Identifier* ident);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*===========================ExpressionStatement============================*/
//Completed expressions and blocks become statements through this type
class ExpressionStatement : public Statement {
public:
	Expression* exp;
	ExpressionStatement(Expression* exp);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*=============================ReturnStatement==============================*/
//C-like return statement AST object
class ReturnStatement : public Statement {
public:
	Expression* exp;
	ReturnStatement(Expression* exp);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*=============================AssignStatement==============================*/
//C-like assignment of a variable
class AssignStatement : public Statement {
public:
	Expression* valxp;
	Expression* target;
	AssignStatement(Expression* target,Expression* valxp);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

/*===============================IfStatement================================*/
//C-like if statement, without else clause
class IfStatement : public Statement {
public:
	Expression* exp;
	Block* block;
	IfStatement(Expression* exp,Block* block);
	virtual void describe() const;
	llvm::Value* acceptCodeGenVisitor(CodeGenVisitor* c);
};

#endif