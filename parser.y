
//TODO: reference types, Golgi apparatus

//Prelude

%{
    #include "node.h"
    #include "yy_overrides.h"
    Block *program;

    extern int yylex();
    extern TypeTable user_type_table;
    extern Node* ast_root;
%}

//Union of semantic types
%union {
    Node* node;
    Expression* exp;
    Statement* statement;
    StructureDefinition* structureDeclaration;
    Identifier* identifier;
    Block* block;
    Keyword* keyword;
    std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>* variableVec;
    std::vector<Expression*,gc_allocator<Expression*>>* expressionVec;
    char* string;
    int token;
}

//Tokens
%token <string> TIDENTIFIER TINTLIT TFLOATLIT TEQUAL TNEW 
%token <string> TNEQUAL TLT TLTE TGT TGTE TLOR TLNOT TSAMPR
%token <string> TPLUS TDASH TSTAR TSLASH TLAND TDOT TSCOLON
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSET TNULL
%token <token> TLSBRACE TRSBRACE TENDL TCOMMA TELSE
%token <token> TINT TFLOAT TVOID TSTRUCT TIF TEXTERN
%token <token> TWHILE TRETURN UMINUS EMPTYFUNARGS

//Types of grammar targets
%type <identifier> ident
%type <exp> exp numeric rexp
%type <statement> statement variableDec functionDec structDec_f
%type <statement> if_statement externStatement
%type <structureDeclaration> structDec_b
%type <block> block statements program
%type <keyword> var_keyword struct_keyword
%type <variableVec> functionArgs
%type <expressionVec> callArgs
%type <string> binaryOperatorToken
%type <string> unaryOperatorToken
%type <token> leftBraceToken rightBraceToken externStatement_h

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
		       auto externInjects = buildInjections();
		       program->statements->insert(program->statements->begin(),externInjects->begin(),externInjects->end());
		       delete externInjects;
		       ast_root = program;
	  	       printf("Parser: start symbol\n\n");
	} ;

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
		pprintf("Parser: add statement to block\n");
		$$=$1;
             } ;

//A statement is a variable declaration, function declaration, empty, or an expression-statement
statement : variableDec TSCOLON TENDL {
	       $$=$1; pprintf("Parser: variableDec becomes statement\n");
	       $$->setCommit(true);
	     } |
	     variableDec TENDL {
               $$=$1; pprintf("Parser: variableDec becomes statement\n");
	       $$->setCommit(false);
             }
	     | functionDec TENDL {$$=$1;pprintf("Parser: functionDec becomes statement\n");}
             | structDec_f TENDL {$$=$1;pprintf("Parser: structDec becomes statement\n");}
	     | if_statement TENDL {$$=$1;}
	     | externStatement TENDL {$$=$1;pprintf("Parser: externStatement becomes statement\n");}
	     |
	     rexp TSET exp TENDL {
		$$ = new AssignStatement($1,$3);
		//if(!($1->identsDeclared())) YYERROR;
		$$->describe();
		$$->setCommit(false);
	     } |
	     rexp TSET exp TSCOLON TENDL {
                $$ = new AssignStatement($1,$3);
		//if(!($1->identsDeclared())) YYERROR;
                $$->describe();
		$$->setCommit(true);
             } |
             exp TENDL {
                $$ = new ExpressionStatement($1);
                $$->describe();
		$$->setCommit(false);
             } |
             exp TSCOLON TENDL {
                $$ = new ExpressionStatement($1);
                $$->describe();
		$$->setCommit(true);
             } |
	     TRETURN TENDL {
		$$ = new ReturnStatement(nullptr);
		$$->describe();
	     } |
	     TRETURN TSCOLON TENDL {
		$$ = new ReturnStatement(nullptr);
		$$->describe();
	     } |
	     TRETURN exp TENDL {
		$$ = new ReturnStatement($2);
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

//Declarations of external functions (standard library for example)
externStatement : externStatement_h var_keyword TSTAR ident TLPAREN functionArgs TRPAREN TSCOLON {
		  $$ = new ExternStatement($2,$4,$6,true);
		  $$->describe();
		} |
		externStatement_h var_keyword ident TLPAREN functionArgs TRPAREN TSCOLON {
                  $$ = new ExternStatement($2,$3,$5,false);
                  $$->describe();
                };

externStatement_h : TEXTERN { $$=$1; }

if_statement : TIF TLPAREN exp TRPAREN block {
		$$ = new IfStatement($3,$5,nullptr);
		$$->describe();
	       } |
		TIF TLPAREN exp TRPAREN block TELSE block {
                $$ = new IfStatement($3,$5,$7);
                $$->describe();
               } ;


block : leftBraceToken statements rightBraceToken { $$ = $2;
		pprintf("Parser: statements become block\n"); } |
        leftBraceToken TENDL statements rightBraceToken { $$ = $3;
                pprintf("Parser: statements become block\n"); } |
        leftBraceToken statements TENDL rightBraceToken { $$ = $2;
                pprintf("Parser: statements become block\n"); } |
        leftBraceToken TENDL statements TENDL rightBraceToken { $$ = $3;
                pprintf("Parser: statements become block\n"); }

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
	     ident ident { StructureDeclaration* sd = new StructureDeclaration($1,$2,false);
		//if (!(sd->validate())) YYERROR;
		$$ = sd; //Place on stack
                $$->describe(); //Assume ident ident is a structure dec
             } |
	     ident TSTAR ident { StructureDeclaration* sd = new StructureDeclaration($1,$3,true);
		//if (!(sd->validate())) YYERROR;
		$$ = sd; //Place on stack
                $$->describe(); //Assume ident ident is a structure dec
             } |
             var_keyword ident TSET exp { $$ = new VariableDefinition($1,$2,$4,false);
                $$->describe();
             } |
             var_keyword TSTAR ident TSET exp { $$ = new VariableDefinition($1,$3,$5,true);
                $$->describe();
             } ;

//Definition of a structure
structDec_f : structDec_b block TSCOLON {
	      StructureDefinition* sd = $1;
	      sd->block = $2;
	      $$ = sd; //Place on stack
	      $$->describe();
	    } ;

//(Forward) decl of structure
structDec_b : struct_keyword ident {
	      StructureDefinition* sd = new StructureDefinition($2,nullptr);
	      //if (!(sd->validate())) YYERROR;
              $$ = sd; //Place on stack
	      pprintf("Early structure declaration of: %s\n",$2->name);
	    } ;

//A function definition is made of a var_keyword, identifier, arguments, and a function body block
functionDec : var_keyword ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6,false);
              $$->describe();
             } |
	      var_keyword TSTAR ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$3,$5,$7,true);
              $$->describe();
             } | ident ident TLPAREN functionArgs TRPAREN block {
              $$ = new FunctionDefinition($1,$2,$4,$6,false);
              $$->describe();
             } |
              ident TSTAR ident TLPAREN functionArgs TRPAREN block {
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
		  pprintf("Parser: functionArgs, empty in function definition\n");} %prec EMPTYFUNARGS
                | variableDec { $$ = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>(); 
                  $$->push_back((VariableDefinition*)$1); 
		  pprintf("Parser: functionArgs, one argument in function definition\n");} 
                  //VariableDec always a VariableDefinition*, although defined as a statement
                | functionArgs TCOMMA variableDec { $1->push_back((VariableDefinition*)$3);
		  pprintf("Parser: additional function argument found in function definition\n");
		} ;

//Arguments of a particular function call at the call site
callArgs : /* empty */ { $$ = new std::vector<Expression*,gc_allocator<Expression*>>(); 
		pprintf("Parser: new callArgs, empty\n");} %prec EMPTYFUNARGS
              | exp { $$ = new std::vector<Expression*,gc_allocator<Expression*>>(); $$->push_back($1);
		pprintf("Parser: new callArgs, one argument expression\n");}
              | callArgs TCOMMA exp { $1->push_back($3); 
		pprintf("Parser: callArgs additional argument found\n");}
              ;

rexp : ident { $$ = $1; $$->describe(); }
	    | TSTAR ident { $$ = new PointerExpression($2,new Integer(0),nullptr); $$->describe(); }
	    | ident TLSBRACE exp TRSBRACE  { $$ = new PointerExpression($1,$3,nullptr); $$->describe(); }
	    | ident TDOT ident { $$ = new StructureExpression($1,$3); $$->describe(); }
	    | TSTAR ident TDOT ident { $$ = new PointerExpression($2,new Integer(0),$4); $$->describe(); }
	    ;

//An identifier comes from the corresponding token string
ident : TIDENTIFIER { $$ = new Identifier($1); $$->describe(); }
        ;

//An expression is pretty much any (valid) mixture of operators
//Identifiers inside expressions must always be declared beforehand
exp : exp binaryOperatorToken exp { //if (!($1->identsDeclared()) || !($3->identsDeclared())) YYERROR;
		$$ = new BinaryOperator($1,$2,$3); $$->describe(); }
            | unaryOperatorToken exp { //if (!($2->identsDeclared())) YYERROR;
		$$ = new UnaryOperator($1,$2); $$->describe(); } %prec UMINUS
            | numeric { $$ = $1; }
            | rexp { /*if (!($1->identsDeclared())) YYERROR; $$ = $1;*/ }
	    | TNULL { $$ = new NullLiteral(); }
            | TLPAREN exp TRPAREN { $$ = $2; } //Consume pairs of parentheses
            | exp TEQUAL exp { //if (!($1->identsDeclared()) || !($3->identsDeclared())) YYERROR; 
		$$ = new BinaryOperator($1,$2,$3); $$->describe(); }
            | ident TLPAREN callArgs TRPAREN { $$ = new FunctionCall($1,$3); $$->describe(); }
	    | TSAMPR ident { $$ = new AddressOfExpression($2,nullptr); $$->describe(); }
	    | TSAMPR ident TLSBRACE exp TRSBRACE { 
		$$ = new AddressOfExpression($2,$4); $$->describe(); }
            ;

numeric : TINTLIT { $$ = new Integer(atol($1)); $$->describe(); }
            | TFLOATLIT { $$ = new Float(atof($1)); $$->describe(); }
            ;

binaryOperatorToken : TEQUAL | TNEQUAL | TLT | TLTE | TGT | TGTE | TDASH 
			| TPLUS | TSTAR | TSLASH | TLOR | TLAND;

leftBraceToken : TLBRACE {$$=$1; };
rightBraceToken : TRBRACE {$$=$1; };

unaryOperatorToken : TDASH | TLNOT | TNEW;

%%
