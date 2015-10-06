
//TODO: reference types, Golgi apparatus

//Prelude

%{
    #include "node.h"
    Block *program;

    extern int yylex();
    extern int yydebug;
    extern int yylineno;
    extern SymbolTable sym_table;
    extern Node* ast_root;
    void yyerror(const char *s) { 
	printf("Error in parser near line %d: %s\n", yylineno, s);
    }
%}

//Union of semantic types
%union {
    Node* node;
    Expression* exp;
    Statement* statement;
    Identifier* identifier;
    Block* block;
    Keyword* keyword;
    vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* variableVec;
    vector<Expression*,gc_allocator<Expression*>>* expressionVec;
    char* string;
    int token;
}

//Tokens
%token <string> TIDENTIFIER TINTLIT TFLOATLIT TEQUAL 
%token <string> TNEQUAL TLT TLTE TGT TGTE TLOR TLNOT
%token <string> TPLUS TDASH TSTAR TSLASH TLAND TDOT TSCOLON
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSET
%token <token> TLSBRACE TRSBRACE TENDL TCOMMA
%token <token> TINT TFLOAT TVOID TSTRUCT TIF
%token <token> TWHILE TRETURN UMINUS EMPTYFUNARGS

//Types of grammar targets
%type <identifier> ident
%type <exp> exp numeric
%type <statement> statement variableDec functionDec structDec
%type <statement> if_statement
%type <block> block statements program
%type <keyword> var_keyword struct_keyword
%type <variableVec> functionArgs
%type <expressionVec> callArgs
%type <string> binaryOperatorToken
%type <string> unaryOperatorToken
%type <string> nullaryOperatorToken
%type <token> leftBraceToken rightBraceToken

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
             | structDec TENDL {$$=$1;printf("Parser: structDec becomes statement\n");}
	     | if_statement TENDL {$$=$1;}
	     |
	     exp TSET exp {
		$$ = new AssignStatement($1,$3);
		$$->describe();
	     } |
	     exp TSET exp TENDL {
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

if_statement : TIF TLPAREN exp TRPAREN block {
		$$ = new IfStatement($3,$5);
		$$->describe();
	       } ;

block : leftBraceToken statements rightBraceToken { $$ = $2; $$->describe();
		printf("Parser: statements become block\n"); } |
        leftBraceToken TENDL statements rightBraceToken { $$ = $3; $$->describe();
                printf("Parser: statements become block\n"); } |
        leftBraceToken statements TENDL rightBraceToken { $$ = $2; $$->describe();
                printf("Parser: statements become block\n"); } |
        leftBraceToken TENDL statements TENDL rightBraceToken { $$ = $3; $$->describe();
                printf("Parser: statements become block\n"); }

              | leftBraceToken rightBraceToken { $$ = new Block(); $$->describe(); }
	      | leftBraceToken TENDL rightBraceToken { $$ = new Block(); $$->describe(); }
              ;

//A variable declaration is made of a var_keyword, identifier, and possibly an expression
variableDec : var_keyword ident { $$ = new VariableDefinition($1,$2,NULL);
                $$->describe();
             } |
	     ident ident { $$ = new StructureDeclaration($1,$2);
                $$->describe(); //Assume ident ident is a structure dec
             } |
             var_keyword ident TSET exp { $$ = new VariableDefinition($1,$2,$4);
                $$->describe();
             } ;

//Definition of a structure
structDec : struct_keyword ident block {
	      $$ = new StructureDefinition($2,$3);
	      $$->describe();
	    } ;

//A function definition is made of a var_keyword, identifier, arguments, and a function body block
functionDec : var_keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             }/* |
             var_keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             } |
             var_keyword ident TLPAREN functionArgs TRPAREN TENDL block {
              $$ = new FunctionDefinition($1,$2,$4,$6);
              $$->describe();
             }*/ ;

//Langauge var_keywords listed here
var_keyword : TINT {
              char name[] = "int";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TFLOAT {
              char name[] = "float";
              $$ = new Keyword(name);
              $$->describe();
              } |
              TVOID {
              char name[] = "void";
              $$ = new Keyword(name);
              $$->describe();
              } ;

//Struct keyword
struct_keyword : TSTRUCT {
                 char name[] = "struct";
                 $$ = new Keyword(name);
                 $$->describe(); }
		 ;


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
            | ident { $$ = $1; if ($1->assertDeclared()) YYERROR; }
            | TLPAREN exp TRPAREN { $$ = $2; } //Consume pairs of parentheses
            | ident TEQUAL exp { $$ = new BinaryOperator($1,$2,$3); $$->describe(); }
            | ident TLPAREN callArgs TRPAREN { $$ = new FunctionCall($1,$3); $$->describe(); }
            ;

numeric : TINTLIT { $$ = new Integer(atol($1)); $$->describe(); }
            | TFLOATLIT { $$ = new Float(atof($1)); $$->describe(); }
            ;

binaryOperatorToken : TEQUAL | TNEQUAL | TLT | TLTE | TGT | TGTE | TDASH 
			| TPLUS | TSTAR | TSLASH | TDOT |
			TLOR | TLAND;

leftBraceToken : TLBRACE {$$=$1; sym_table.push(); };
rightBraceToken : TRBRACE {$$=$1; sym_table.pop(); }

unaryOperatorToken : TSTAR | TDASH | TLNOT;

nullaryOperatorToken : TSCOLON;

%%
