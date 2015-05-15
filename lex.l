%{

//Includes
#include <string>
#include <stdio.h>
#include "gc.h"
#include "gc_alloc.h"

//You must specify the allocator gc_alloc to STL containers to get GC memory

#define SAVE_TOKEN yylval.string = (char *)GC_MALLOC(yyleng+1); memcpy(yylval.string,yytext,yyleng);

//TOKEN macro sets token. Recall that yylval is a union type declared in the parser
#define TOKEN(t) (yylval.token = t)

//Only process a single input file
extern "C" int yywrap() { return 1; }

%}

%%

[ \t\r]                   ;
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
"["                       return TOKEN(TLSBRACE);
"]"                       return TOKEN(TRSBRACE);
"."                       return TOKEN(TDOT);
"+"                       return TOKEN(TPLUS);
"-"                       return TOKEN(TDASH);
"*"                       return TOKEN(TSTAR);
"/"                       return TOKEN(TSLASH);
";"                       return TOKEN(TSCOLON);
"if"                      return TOKEN(TIF);
"while"                   return TOKEN(TWHILE);
"return"		  return TOKEN(TRETURN);
"int"                     return TOKEN(TINT);
"float"                   return TOKEN(TFLOAT);
"void"                    return TOKEN(TVOID);
"struct"                  return TOKEN(TSTRUCT);
[0-9]*\.[0-9]+            SAVE_TOKEN; return TFLOATLIT;
[0-9]+                    SAVE_TOKEN; return TINTLIT;
[a-zA-Z$_][a-zA-Z0-9$_]*  SAVE_TOKEN; return TIDENTIFIER;
.                         printf("Unknown token encountered in lexer\n"); yyterminate();

%%
