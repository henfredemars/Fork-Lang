
//TODO: Reference types, structures, check compliance, includes


//Prelude

%{
    #include "node.h"
    Block *program;

    extern int yylex();
    void yyerror(const char *s) { printf("Error in parser: %s\n", s); }
%}

//Union of semantic types
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
    vector<Expression*,gc_alloc> expressionVec;
    FunctionDefinition* functionDef;
    char* string;
    int64_t token;
}

//Tokens
%token <string> TIDENTIFIER TINTLIT TFLOATLIT
%token <token> TSET TEQUAL TNEQUAL TLT TLTE TGT TGTE
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TLSBRACE TRSBRACE TDOT TENDL
%token <token> TPLUS TDASH TSTAR TSLASH TCOMMA
%token <token> TINT TFLOAT TVOID TSTRUCT

//Types of grammar targets
%type <identifier> ident
%type <exp> exp numeric
%type <statement> statement variableDec functionDec
%type <assignment> assignment
%type <block> block statements program
%type <functionCall> functionCall
%type <keyword> keyword
%type <variableDef> variableDef
%type <functionDef> functionDef
%type <variableVec> functionArgs
%type <expressionVec> callArgs
%type <token> binaryOperatorToken
%type <token> unaryOperatorToken
%type <token> nullaryOperatorToken

//Operators and their precedence
%left TPLUS TDASH
%left TSTAR TSLASH

//The target is a program
%start program


//Begin grammar rules
%%

//A program is a collection of statements in a block
program : statements { program = $1; }
        ;

//Statements can be collected together to form blocks
statements : statement { 
                vector<Statement*,gc_alloc> statements();
                $$ = new Block(statements);
                $$->statements.push_back($1);
             } |
             statements statement {
                $1->statements.push_back($2);
             } ;

//A statement is a variable declaration, function declaration, empty, or an expression-statement
statement : variableDec | functionDec | TENDL { $$ = new Statement(); 
             } |
             exp {
                $$ = new ExpressionStatement(exp);
             } ;

block : TLBRACE statements TRBRACE { $$ = $2; }
              | TLBRACE TRBRACE { $$ = new Block(); }
              ;

//A variable declaration is made of a keyword, identifier, and possibly an expression
variableDec : keyword ident TENDL { $$ = new VariableDefinition($1,$2,NULL);
             } |
             keyword ident TSET exp TENDL { $$ = new VariableDefinition($1,$2,$4);
             } ;

//A function definition is made of a keyword, identifier, arguments, and a function body block
functionDec : keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
             } |
             keyword ident TLPAREN functionArgs TRPAREN statement block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
             } |
             keyword ident TLPAREN functionArgs TRPAREN statements block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
             } ;

//Langauge keywords listed here
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

//Arguments in a function definition
functionArgs : /* empty */ { 
                $$ = vector<VariableDefinition*,gc_alloc>(); }
                | variableDec { $$ = vector<VariableDefinition*,gc_alloc>(); $$.push_back($1); }
                | functionArgs TCOMMA variableDec { $1.push_back($3); }
                ;

//Arguments of a particular function call at the call site
callArgs : /* empty */ { $$ = vector<Expression*,gc_alloc>(); }
              | exp { $$ = vector<Expression*,gc_alloc>(); $$.push_back($1); }
              | callArgs TCOMMA exp { $1.push_back($3); }
              ;

//An identifier comes from the corresponding token string
ident : TIDENTIFIER { $$ = new Identifier($1); }
        ;

//An expression is pretty much any (valid) mixture of operators
exp : exp binaryOperatorToken exp { $$ = new BinaryOperator($1,$2,$3); }
            | TDASH exp { $$ = new UnaryOperator($1,$2); }
            | exp TSTAR { $$ = new UnaryOperator($2,$1); }
            | nullaryOperatorToken { $$ = new NullaryOperator($1); }
            | numeric { $$ = $1; }
            | ident { $$ = $1; }
            | TLPAREN exp TRPAREN { $$ = 2; } //Consume pairs of parentheses
            | ident TEQUAL exp { $$ = new Assignment($1,$3); }
            | ident TLPAREN callArgs TRPAREN { $$ = new FunctionCall($1,$3); }
            ;

numeric : TINTLIT { $$ = new Integer($1); }
            | TFLOATLIT { $$ = new Float($1); }
            ;

binaryOperatorToken : TEQUAL | TNEQUAL | TLT | TLTE | TGT | TGTE | TDASH | TPLUS | TSTAR | TSLASH;

unaryOperatorToken : TSTAR | TDASH;

nullaryOperatorToken : TSCOLON;

%%