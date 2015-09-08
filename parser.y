
//TODO: Control flow, reference types, structures, consider arrays, check compliance, includes


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
    Float* flote; //name unimportant, float
    Identifier* identifier;
    NullaryOperator* nullaryOp;
    UnaryOperator* unaryOp;
    BinaryOperator* binaryOp;
    Assignment* assignment;
    Block* block;
    FunctionCall* functionCall;
    Keyword* keyword;
    VariableDefinition* variableDef;
    vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* variableVec;
    vector<Expression*,gc_allocator<Expression*>>* expressionVec;
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
%token <token> TINT TFLOAT TVOID TSTRUCT TIF
%token <token> TWHILE TRETURN TSCOLON

//Types of grammar targets
%type <identifier> ident
%type <exp> exp numeric
%type <statement> statement variableDec functionDec
%type <block> block statements program
%type <keyword> keyword
%type <variableVec> functionArgs
%type <expressionVec> callArgs
%type <token> binaryOperatorToken
//%type <token> unaryOperatorToken
%type <token> nullaryOperatorToken

//Operators precedence
%left TEQUAL TNEQUAL TLT TLTE TGT TGTE
%left TPLUS TDASH
%left TSTAR TSLASH

//The target is a program
%start program


//Begin grammar rules
%%

//A program is a collection of statements in a block
program : statements { program = $1; program->describe(); }
        ;

//Statements can be collected together to form blocks
statements : statement { 
                vector<Statement*,gc_allocator<Statement*>>* statements;
                $$ = new Block(statements);
                $$->statements->push_back($1);
                $$->describe();
             } |
             statements statement {
                $1->statements->push_back($2);
             } ;

//A statement is a variable declaration, function declaration, empty, or an expression-statement
statement : variableDec | functionDec | TENDL { $$ = new Statement(); 
             } |
             exp {
                $$ = new ExpressionStatement($1);
                $$->describe();
             } ;

block : TLBRACE statements TRBRACE { $$ = $2; $$->describe(); }
              | TLBRACE TRBRACE { $$ = new Block(); $$->describe(); }
              ;

//A variable declaration is made of a keyword, identifier, and possibly an expression
variableDec : keyword ident TENDL { $$ = new VariableDefinition($1,$2,NULL);
                $$->describe();
             } |
             keyword ident TSET exp TENDL { $$ = new VariableDefinition($1,$2,$4);
                $$->describe();
             } ;

//A function definition is made of a keyword, identifier, arguments, and a function body block
functionDec : keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             }/* |
             keyword ident TLPAREN functionArgs TRPAREN statement block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             } |
             keyword ident TLPAREN functionArgs TRPAREN statements block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             }*/ ;

//Langauge keywords listed here
keyword : TINT {
              char name[] = "int";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TFLOAT {
              char name[] = "float";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TIF {
              char name[] = "if";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TWHILE {
              char name[] = "while";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TRETURN {
              char name[] = "return";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TSTRUCT {
              char name[] = "struct";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TVOID {
              char name[] = "void";
              $$ = new Keyword(name);
              $$->describe();
              } ;

//Arguments in a function definition
functionArgs : /* empty */ { 
                $$ = new vector<VariableDefinition*,gc_allocator<VariableDefinition*>>(); }
                | variableDec { $$ = new vector<VariableDefinition*,gc_allocator<VariableDefinition*>>(); $$->push_back((VariableDefinition*)$1); } //VariableDec always a VariableDefinition*, although definied as a statement
                | functionArgs TCOMMA variableDec { $1->push_back((VariableDefinition*)$3); }
                ;

//Arguments of a particular function call at the call site
callArgs : /* empty */ { $$ = new vector<Expression*,gc_allocator<Expression*>>(); }
              | exp { $$ = new vector<Expression*,gc_allocator<Expression*>>(); $$->push_back($1); }
              | callArgs TCOMMA exp { $1->push_back($3); }
              ;

//An identifier comes from the corresponding token string
ident : TIDENTIFIER { $$ = new Identifier($1); $$->describe(); }
        ;

//An expression is pretty much any (valid) mixture of operators
exp : exp binaryOperatorToken exp { $$ = new BinaryOperator($1,$2,$3); $$->describe(); }
            | TDASH exp { $$ = new UnaryOperator($1,$2); $$->describe(); }
            | nullaryOperatorToken { $$ = new NullaryOperator($1); $$->describe(); }
            | numeric { $$ = $1; }
            | ident { $$ = $1; }
            | TLPAREN exp TRPAREN { $$ = $2; } //Consume pairs of parentheses
            | ident TEQUAL exp { $$ = new Assignment($1,$3); $$->describe(); }
            | ident TLPAREN callArgs TRPAREN { $$ = new FunctionCall($1,$3); $$->describe(); }
            ;

numeric : TINTLIT { $$ = new Integer(atol($1)); $$->describe(); }
            | TFLOATLIT { $$ = new Float(atof($1)); $$->describe(); }
            ;

binaryOperatorToken : TEQUAL | TNEQUAL | TLT | TLTE | TGT | TGTE | TDASH | TPLUS | TSTAR | TSLASH;

//unaryOperatorToken : TSTAR | TDASH;

nullaryOperatorToken : TSCOLON;

%%
