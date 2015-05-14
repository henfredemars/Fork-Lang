%{

//Includes
#include <string>

//Token.string is the value lexed
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)

//TOKEN macro sets token. Recall that yylval is a union type declared in the parser
#define TOKEN(t) (yylval.token = t)

//Only process a single input file
extern "C" int yywrap() { return 1; }

%}

%%

[ \t\r]                   ;
[a-zA-Z$_][a-zA-Z0-9$_]*  SAVE_TOKEN; return TIDENTIFIER;
[0-9]*\.[0-9]+            SAVE_TOKEN; return TFLOAT;
[0-9]+                    SAVE_TOKEN; return TINT;
"="                       return TOKEN(TSET);
"=="                      return TOKEN(TEQUAL);
"!="                      return TOKEN(TNEQUAL);
"<"                       return TOKEN(TLT);
"<="                      return TOKEN(TLTE);
">"                       return TOKEN(TGT);
">="                      return TOKEN(TGTE);
"("                       return TOKEN(TLPAREN);
")"                       return TOKEN(TRPAREN);
"{"                       return TOKEN(TLBRACE);
"}"                       return TOKEN(TRBRACE);
"."                       return TOKEN(TDOT);
"+"                       return TOKEN(TPLUS);
"-"                       return TOKEN(TDASH);
"*"                       return TOKEN(TSTAR);
"/"                       return TOKEN(TSLASH);
.                         printf("Unknown token encountered in lexer\n"); yyterminate();

%%
