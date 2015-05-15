%{
    #include "node.h"
    Block *program;

    extern int yylex();
    void yyerror(const char *s) { printf("Error in parser: %s\n", s); }
%}

%union {
    Node* node;
    Expression* exp;
    Statement* statement;
    Integer* integer;
    Float* float;
    Identifier* identifier;
    NullaryOperator* nullaryOp;
    UnaryOperator* unaryOp;
    BinaryOperator* binaryOp;
    Assignment* assignment;
    Block* block;
    FunctionCall* functionCall;
    Keyword* keyword;
    VariableDefinition* variableDef;
    FunctionDefinition* functionDef;
    char* string;
    int64_t token;
}

%token <string> TIDENTIFIER TINTLIT TFLOATLIT
%token <token> TSET TEQUAL TNEQUAL TLT TLTE TGT TGTE
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TLSBRACE TRSBRACE TDOT
%token <token> TPLUS TDASH TSTAR TSLASH
%token <token> TINT TFLOAT TVOID TSTRUCT

%type <exp> exp
%type <statement> statement
%type <nullaryOp> nullaryOp
%type <binaryOp> binaryOp
%type <assignment> assignment
%type <block> block statements program
%type <functionCall> functionCall
%type <keyword> keyword
%type <variableDef> variableDef
%type <functionDef> functionDef

//Operators -- add detail here for handling long expressions
%left TPLUS TDASH
%left TSTAR TSLASH

%start program

%%

program : statements { program = $1; }
        ;
        
statements : statement { 
                vector<Statement*,gc_alloc> statements();
                $$ = new Block(statements);
                $$->statements.push_back($1);
             }
             statements statement {
                $1->statements.push_back($2);
             }
             ;

$$ = new Block(); $$->statements.push_back($<statement>1); }
      | statements statement { $1->statements.push_back($<statement>2); }
      ;

statement :
     ;

%%