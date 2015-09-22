#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

class Node;
class Expression;
class Statement;
class Integer;
class Float;
class Identifier;
class NullaryOperator;
class UnaryOperator;
class BinaryOperator;
class Assignment;
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

class Visitor {
public:
	virtual llvm::Value* visitExpression(Expression e) =0;
	virtual llvm::Value* visitStatement(Statement s) =0;
	virtual llvm::Value* visitInteger(Integer i) =0;
	virtual llvm::Value* visitFloat(Float f) =0;
	virtual llvm::Value* visitIdentifier(Identifier i) =0;
	virtual llvm::Value* visitNullaryOperator(NullaryOperator n) =0;
	virtual llvm::Value* visitUnaryOperator(UnaryOperator u) =0;
	virtual llvm::Value* visitBinaryOperator(BinaryOperator b) =0;
	virtual llvm::Value* visitAssignment(Assignment a) =0;
	virtual llvm::Value* visitBlock(Block b) =0;
	virtual llvm::Value* visitFunctionCall(FunctionCall f) =0;
	virtual llvm::Value* visitKeyword(Keyword k) =0;
	virtual llvm::Value* visitVariableDefinition(VariableDefinition v) =0;
	virtual llvm::Value* visitStructureDefinition(StructureDefinition s) =0;
	virtual llvm::Value* visitFunctionDefinition(FunctionDefinition f) =0;
	virtual llvm::Value* visitStructureDeclaration(StructureDeclaration s) =0;
	virtual llvm::Value* visitExpressionStatement(ExpressionStatement e) =0;
	virtual llvm::Value* visitReturnStatement(ReturnStatement r) =0;
	virtual llvm::Value* visitAssignStatement(AssignStatement a) =0;
	virtual llvm::Value* visitIfStatement(IfStatement i) =0;
};

class CodeGenVisitor : public Visitor {
public:
	llvm::Value* visitExpression(Expression e);
	llvm::Value* visitStatement(Statement s);
	llvm::Value* visitInteger(Integer i);
	llvm::Value* visitFloat(Float f);
	llvm::Value* visitIdentifier(Identifier i);
	llvm::Value* visitNullaryOperator(NullaryOperator n);
	llvm::Value* visitUnaryOperator(UnaryOperator u);
	llvm::Value* visitBinaryOperator(BinaryOperator b);
	llvm::Value* visitAssignment(Assignment a);
	llvm::Value* visitBlock(Block b);
	llvm::Value* visitFunctionCall(FunctionCall f);
	llvm::Value* visitKeyword(Keyword k);
	llvm::Value* visitVariableDefinition(VariableDefinition v);
	llvm::Value* visitStructureDefinition(StructureDefinition s);
	llvm::Value* visitFunctionDefinition(FunctionDefinition f);
	llvm::Value* visitStructureDeclaration(StructureDeclaration s);
	llvm::Value* visitExpressionStatement(ExpressionStatement e);
	llvm::Value* visitReturnStatement(ReturnStatement r);
	llvm::Value* visitAssignStatement(AssignStatement a);
	llvm::Value* visitIfStatement(IfStatement i);
};