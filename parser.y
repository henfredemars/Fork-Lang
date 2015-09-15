
//TODO: Control flow, reference types, structures, consider arrays, check compliance, includes fcall


//Prelude

%{
    #include "node.h"
    Block *program;

    extern int yylex();
    extern int yydebug;
    extern Node* ast_root;
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
%token <token> TLAND TLOR TLNOT UMINUS EMPTYFUNARGS

//Types of grammar targets
%type <identifier> ident
%type <exp> exp numeric
%type <statement> statement variableDec functionDec
%type <block> block statements program
%type <keyword> keyword
%type <variableVec> functionArgs
%type <expressionVec> callArgs
%type <token> binaryOperatorToken
%type <token> unaryOperatorToken
%type <token> nullaryOperatorToken

//Operators precedence
%precedence TCOMMIT TENDL EMTPYFUNARGS TSCOLON
%left TEQUAL TNEQUAL TLT TLTE TGT TGTE TLAND TLOR TLNOT
%left TPLUS TDASH
%left TSTAR TSLASH
%right UMINUS
%precedence TIDENTIFIER


//Start symbol
%start program


//Begin grammar rules
%%

//A program is a collection of statements in a block
program : statements { program = $1; program->describe();
		       ast_root = program;
	  	       printf("Parser: start symbol\n");}
        ;

//Statements can be collected together to form blocks
statements : statement { 
                vector<Statement*,gc_allocator<Statement*>>* statements;
		statements = new vector<Statement*,gc_allocator<Statement*>>();
                $$ = new Block(statements);
                printf("Push marker 1\n");
                $$->statements->push_back($1);
                $$->describe();
             } |
             statements statement {
                $1->statements->push_back($2);
		printf("Parser: add statement to block\n");
		$$=$1;
             } ;

//A statement is a variable declaration, function declaration, empty, or an expression-statement
statement : variableDec TSCOLON TENDL {$$=$1;printf("Parser: variableDec becomes statement\n");} 
	     | functionDec TENDL {$$=$1;printf("Parser: functionDec becomes statement\n");}
             |
	     exp TSET exp {
		$$ = new AssignStatement($1,$3);
		$$->describe();
	     } |
             exp TENDL {
                $$ = new ExpressionStatement($1);
                $$->describe();
             } |
	     TRETURN TENDL {
		$$ = new ReturnStatement(NULL);
		$$->describe();
	     } |
	     TRETURN exp TENDL {
		$$ = new ReturnStatement($2);
		$$->describe();
             } |
             exp { //Dont require a TENDL to consume
                $$ = new ExpressionStatement($1);
                $$->describe();
             } |
             TRETURN {
                $$ = new ReturnStatement(NULL);
                $$->describe();
             } |
             TRETURN exp {
                $$ = new ReturnStatement($2);
                $$->describe();
             } |
             TRETURN exp TSCOLON {
                $$ = new ReturnStatement($2);
                $$->describe();
             } ;



block : TLBRACE statements TRBRACE { $$ = $2; $$->describe();
		printf("Parser: statements become block\n"); } |
        TLBRACE TENDL statements TRBRACE { $$ = $3; $$->describe();
                printf("Parser: statements become block\n"); } |
        TLBRACE statements TENDL TRBRACE { $$ = $2; $$->describe();
                printf("Parser: statements become block\n"); } |
        TLBRACE TENDL statements TENDL TRBRACE { $$ = $3; $$->describe();
                printf("Parser: statements become block\n"); }

              | TLBRACE TRBRACE { $$ = new Block(); $$->describe(); }
	      | TLBRACE TENDL TRBRACE { $$ = new Block(); $$->describe(); }
              ;

//A variable declaration is made of a keyword, identifier, and possibly an expression
variableDec : keyword ident { $$ = new VariableDefinition($1,$2,NULL);
                $$->describe();
             } |
             keyword ident TSET exp { $$ = new VariableDefinition($1,$2,$4);
                $$->describe();
             } ;

//A function definition is made of a keyword, identifier, arguments, and a function body block
functionDec : keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             }/* |
             keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             } |
             keyword ident TLPAREN functionArgs TRPAREN TENDL block {
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
                $$ = new vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
		  printf("Parser: functionArgs, empty in function definition\n");} %prec EMPTYFUNARGS
                | variableDec { $$ = new vector<VariableDefinition*,gc_allocator<VariableDefinition*>>(); 
                  $$->push_back((VariableDefinition*)$1); 
		  printf("Parser: functionArgs, one argument in function definition\n");} 
                  //VariableDec always a VariableDefinition*, although defined as a statement
                | functionArgs TCOMMA variableDec { $1->push_back((VariableDefinition*)$3);
		  printf("Parser: additional function argument found in function definition\n");}
                ;

//Arguments of a particular function call at the call site
callArgs : /* empty */ { $$ = new vector<Expression*,gc_allocator<Expression*>>(); 
		printf("Parser: new callArgs, empty\n");} %prec EMPTYFUNARGS
              | exp { $$ = new vector<Expression*,gc_allocator<Expression*>>(); $$->push_back($1);
		printf("Parser: new callArgs, one argument expression\n");}
              | callArgs TCOMMA exp { $1->push_back($3); 
		printf("Parser: callArgs additional argument found\n");}
              ;

//An identifier comes from the corresponding token string
ident : TIDENTIFIER { $$ = new Identifier($1); $$->describe(); }
        ;

//An expression is pretty much any (valid) mixture of operators
exp : exp binaryOperatorToken exp { $$ = new BinaryOperator($1,$2,$3); $$->describe(); }
            | unaryOperatorToken exp { $$ = new UnaryOperator($1,$2); $$->describe(); } %prec UMINUS
            | nullaryOperatorToken { $$ = new NullaryOperator($1); $$->describe(); } %prec TCOMMIT
            | numeric { $$ = $1; }
            | ident { $$ = $1; }
            | TLPAREN exp TRPAREN { $$ = $2; } //Consume pairs of parentheses
            | ident TEQUAL exp { $$ = new Assignment($1,$3); $$->describe(); }
            | ident TLPAREN callArgs TRPAREN { $$ = new FunctionCall($1,$3); $$->describe(); }
            ;

numeric : TINTLIT { $$ = new Integer(atol($1)); $$->describe(); }
            | TFLOATLIT { $$ = new Float(atof($1)); $$->describe(); }
            ;

binaryOperatorToken : TEQUAL | TNEQUAL | TLT | TLTE | TGT | TGTE | TDASH 
			| TPLUS | TSTAR | TSLASH
			TLOR | TLAND | TLNOT;

unaryOperatorToken : TSTAR | TDASH | TLNOT;

nullaryOperatorToken : TSCOLON;

%%
