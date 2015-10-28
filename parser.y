
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
    ReferenceExpression* rexp;
    Statement* statement;
    Identifier* identifier;
    Block* block;
    Keyword* keyword;
    std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* variableVec;
    std::vector<Expression*,gc_allocator<Expression*>>* expressionVec;
    char* string;
    int token;
}

//Tokens
%token <string> TIDENTIFIER TINTLIT TFLOATLIT TEQUAL 
%token <string> TNEQUAL TLT TLTE TGT TGTE TLOR TLNOT TSAMPR
%token <string> TPLUS TDASH TSTAR TSLASH TLAND TDOT TSCOLON
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSET
%token <token> TLSBRACE TRSBRACE TENDL TCOMMA
%token <token> TINT TFLOAT TVOID TSTRUCT TIF
%token <token> TWHILE TRETURN UMINUS EMPTYFUNARGS

//Types of grammar targets
%type <identifier> ident
%type <exp> exp numeric
%type <rexp> rexp
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
%right UMINUS TSAMPER
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
                std::vector<Statement*,gc_allocator<Statement*>>* statements;
		statements = new std::vector<Statement*,gc_allocator<Statement*>>();
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
	     rexp TSET exp {
		$$ = new AssignStatement($1,$3);
		if(!($1->identsDeclared())) YYERROR;
		$$->describe();
	     } |
	     rexp TSET exp TENDL {
                $$ = new AssignStatement($1,$3);
		if(!($1->identsDeclared())) YYERROR;
                $$->describe();
             } |
             exp TENDL {
                $$ = new ExpressionStatement($1);
                $$->describe();
             } |
	     TRETURN TENDL {
		$$ = new ReturnStatement(nullptr);
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
                $$ = new ReturnStatement(nullptr);
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

block : leftBraceToken statements rightBraceToken { $$ = $2;
		printf("Parser: statements become block\n"); } |
        leftBraceToken TENDL statements rightBraceToken { $$ = $3;
                printf("Parser: statements become block\n"); } |
        leftBraceToken statements TENDL rightBraceToken { $$ = $2;
                printf("Parser: statements become block\n"); } |
        leftBraceToken TENDL statements TENDL rightBraceToken { $$ = $3;
                printf("Parser: statements become block\n"); }

              | leftBraceToken rightBraceToken { $$ = new Block(); $$->describe(); }
	      | leftBraceToken TENDL rightBraceToken { $$ = new Block(); $$->describe(); }
              ;

//A variable declaration is made of a var_keyword, identifier, and possibly an expression
variableDec : var_keyword ident { $$ = new VariableDefinition($1,$2,nullptr,false);
                $$->describe();
             } |
	     var_keyword TSTAR ident { $$ = new VariableDefinition($1,$3,nullptr,true);
                $$->describe();
             } |
	     ident ident { $$ = new StructureDeclaration($1,$2);
                $$->describe(); //Assume ident ident is a structure dec
             } |
             var_keyword ident TSET exp { $$ = new VariableDefinition($1,$2,$4,false);
                $$->describe();
             } |
             var_keyword TSTAR ident TSET exp { $$ = new VariableDefinition($1,$3,$5,true);
                $$->describe();
             } ;

//Definition of a structure
structDec : struct_keyword ident block {
	      $$ = new StructureDefinition($2,$3);
	      $$->describe();
	    } ;

//A function definition is made of a var_keyword, identifier, arguments, and a function body block
functionDec : var_keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6,false);
              $$->describe();
             } |
	      var_keyword TSTAR ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$3,$5,$7,true);
              $$->describe();
             } ;

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
                $$ = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
		  printf("Parser: functionArgs, empty in function definition\n");} %prec EMPTYFUNARGS
                | variableDec { $$ = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>(); 
                  $$->push_back((VariableDefinition*)$1); 
		  printf("Parser: functionArgs, one argument in function definition\n");} 
                  //VariableDec always a VariableDefinition*, although defined as a statement
                | functionArgs TCOMMA variableDec { $1->push_back((VariableDefinition*)$3);
		  printf("Parser: additional function argument found in function definition\n");}
                ;

//Arguments of a particular function call at the call site
callArgs : /* empty */ { $$ = new std::vector<Expression*,gc_allocator<Expression*>>(); 
		printf("Parser: new callArgs, empty\n");} %prec EMPTYFUNARGS
              | exp { $$ = new std::vector<Expression*,gc_allocator<Expression*>>(); $$->push_back($1);
		printf("Parser: new callArgs, one argument expression\n");}
              | callArgs TCOMMA exp { $1->push_back($3); 
		printf("Parser: callArgs additional argument found\n");}
              ;

rexp : ident { $$ = new ReferenceExpression($1,nullptr,false,false); $$->describe(); }
	    | TSTAR ident { $$ = new ReferenceExpression($2,new Integer(0),true,false); $$->describe(); }
	    | TSAMPR ident { $$ = new ReferenceExpression($2,new Integer(0),false,true); $$->describe(); }
	    | TSAMPR ident TLSBRACE exp TRSBRACE { 
		$$ = new ReferenceExpression($2,$4,false,true); $$->describe(); }
	    | ident TLSBRACE exp TRSBRACE  { $$ = new ReferenceExpression($1,$3,true,false); $$->describe(); }
	    ;

//An identifier comes from the corresponding token string
ident : TIDENTIFIER { $$ = new Identifier($1); $$->describe(); }
        ;

//An expression is pretty much any (valid) mixture of operators
//Identifiers inside expressions must always be declared beforehand
exp : exp binaryOperatorToken exp { if (!($1->identsDeclared()) || !($3->identsDeclared())) YYERROR;
		$$ = new BinaryOperator($1,$2,$3); $$->describe(); }
            | unaryOperatorToken exp { if (!($2->identsDeclared())) YYERROR;
		$$ = new UnaryOperator($1,$2); $$->describe(); } %prec UMINUS
            | nullaryOperatorToken { $$ = new NullaryOperator($1); $$->describe(); } %prec TCOMMIT
            | numeric { $$ = $1; }
            | rexp { if (!($1->identsDeclared())) YYERROR; $$ = $1; }
            | TLPAREN exp TRPAREN { $$ = $2; } //Consume pairs of parentheses
            | exp TEQUAL exp { if (!($1->identsDeclared()) || !($3->identsDeclared())) YYERROR; 
		$$ = new BinaryOperator($1,$2,$3); $$->describe(); }
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

unaryOperatorToken : TDASH | TLNOT;

nullaryOperatorToken : TSCOLON;

%%
