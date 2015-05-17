
//Declarations of the AST objects, should model the language

#include <cstdint>
#include <vector>
#include <string.h>
#include <stdio.h>
//#include <llvm/Value.h>
#include "gc.h"
#include "gc_cpp.h"
#include "gc_alloc.h"

using namespace std;

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
class Assignment;
class Block;
class FunctionCall;
class Keyword;
class VariableDefinition;
class FunctionDefinition;

class Node : public gc {
public:
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class Expression : public Node {
public:
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class Statement : public Node {
public:
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class Integer : public Expression {
public:
	int64_t value;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class Float : public Expression {
public:
	double value;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class Identifier : public Expression {
public:
	char* name;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class NullaryOperator : public Expression {
public:
	int64_t op;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class UnaryOperator : public Expression {
public:
	int64_t op;
	Expression* exp;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class BinaryOperator : public Expression {
public:
	int64_t op;
	Expression* left;
	Expression* right;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class Assignment : public Expression {
public:
	Identifier* left;
	Expression* right;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

//Must be an Expression to be contained in other blocks, but
//  strictly speaking, a Block is a collection of statements
class Block : public Expression {
public:
	Block();
	Block(vector<Statement*,gc_alloc> statements);
	vector<Statement*,gc_alloc> statements;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class FunctionCall : public Expression {
public:
	Identifier* ident;
	vector<Expression*,gc_alloc> args;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

//Keyword refers to the type of a declaration, not language keywords
class Keyword : public Node {
public:
	char* name;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class VariableDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	Expression* exp;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

class FunctionDefinition : public Statement {
public:
	Keyword* type;
	Identifier* ident;
	vector<VariableDefinition*,gc_alloc> args;
	Block* block;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};

//Completed expressions and blocks become statements through this type
class ExpressionStatement : public Statement {
public:
	Expression* exp;
	virtual void describe();
	//virtual llvm::Value* codeGen(CodeGenContext* context);
};
