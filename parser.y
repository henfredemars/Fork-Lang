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
    vector<VariableDefinition*,gc_alloc> variableVec;
    FunctionDefinition* functionDef;
    char* string;
    int64_t token;
}

%token <string> TIDENTIFIER TINTLIT TFLOATLIT
%token <token> TSET TEQUAL TNEQUAL TLT TLTE TGT TGTE
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TLSBRACE TRSBRACE TDOT TENDL
%token <token> TPLUS TDASH TSTAR TSLASH
%token <token> TINT TFLOAT TVOID TSTRUCT

%type <exp> exp
%type <statement> statement variableDec functionDec
%type <nullaryOp> nullaryOp
%type <binaryOp> binaryOp
%type <assignment> assignment
%type <block> block statements program
%type <functionCall> functionCall
%type <keyword> type_keyword
%type <variableDef> variableDef
%type <functionDef> functionDef
%type <variableVec> functionArgs

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
             } |
             statements statement {
                $1->statements.push_back($2);
             } ;

statement : variableDec | functionDec | TENDL { $$ = new Statement(); 
             } |
             exp {
                $$ = new ExpressionStatement(exp);
             } ;

variableDec : keyword ident TENDL { $$ = new VariableDefinition($1,$2,NULL);
             } |
             keyword ident TSET exp TENDL { $$ = new VariableDefinition($1,$2,$4);
             } ;

functionDec : keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
             } |
             keyword ident TLPAREN functionArgs TRPAREN statement block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
             } |
             keyword ident TLPAREN functionArgs TRPAREN statements block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
             } ;

keyword : TINT {
              char[] name = "int";
              $$ = new Keyword(name);
              } |
              TFLOAT {
              char[] name = "float";
              $$ = new Keyword(name);
              } |
              TIF {
              char[] name = "if";
              $$ = new Keyword(name);
              } |
              TWHILE {
              char[] name = "while";
              $$ = new Keyword(name);
              } |
              TRETURN {
              char[] name = "return";
              $$ = new Keyword(name);
              } |
              TSTRUCT {
              char[] name = "struct";
              $$ = new Keyword(name);
              } |
              TVOID {
              char[] name = "void";
              $$ = new Keyword(name);
              } ;

ident : TIDENTIFIER { $$ = new Identifier($1); }
        ;



%%