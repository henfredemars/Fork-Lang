%{

//Includes
#include <string>
#include <stdio.h>
#include "node.h"
#include "parser.hpp"
#include "./gc/include/gc.h"
#include "./gc/include/gc_allocator.h"
#include "yy_overrides.h"

//You must specify the allocator gc_alloc to STL containers to get GC memory

#define SAVE_TOKEN do { yylval.string = (char *)GC_MALLOC_ATOMIC(yyleng+1); memcpy(yylval.string,yytext,yyleng); } while (0)

//TOKEN macro sets token. Recall that yylval is a union type declared in the parser
#define TOKEN(t) (yylval.token = t)

//Only process a single input file
extern "C" int yywrap() { return 1; }

%}

%option noyyalloc
%option noyyrealloc
%option noyyfree
%option yylineno

%%

[ \t\r]                   ;
"\n"                      return TOKEN(TENDL);
"="                       return TOKEN(TSET);
"=="                      SAVE_TOKEN; return TEQUAL;
"!="                      SAVE_TOKEN; return TNEQUAL;
"<"                       SAVE_TOKEN; return TLT;
"<="                      SAVE_TOKEN; return TLTE;
">"                       SAVE_TOKEN; return TGT;
">="                      SAVE_TOKEN; return TGTE;
"||"                      SAVE_TOKEN; return TLOR;
"&&"                      SAVE_TOKEN; return TLAND;
"!"                       return TOKEN(TLNOT);
"("                       return TOKEN(TLPAREN);
")"                       return TOKEN(TRPAREN);
"{"                       return TOKEN(TLBRACE);
"}"                       return TOKEN(TRBRACE);
"["                       return TOKEN(TLSBRACE);
"]"                       return TOKEN(TRSBRACE);
"."                       SAVE_TOKEN; return TDOT;
"+"                       SAVE_TOKEN; return TPLUS;
"-"                       SAVE_TOKEN; return TDASH;
"*"                       SAVE_TOKEN; return TSTAR;
"/"                       SAVE_TOKEN; return TSLASH;
";"                       return TOKEN(TSCOLON);
","                       return TOKEN(TCOMMA);
"if"                      return TOKEN(TIF);
"while"                   return TOKEN(TWHILE);
"return"                  return TOKEN(TRETURN);
"int"                     return TOKEN(TINT);
"float"                   return TOKEN(TFLOAT);
"void"                    return TOKEN(TVOID);
"struct"                  return TOKEN(TSTRUCT);
[0-9]*\.[0-9]+            SAVE_TOKEN; return TFLOATLIT;
[0-9]+                    SAVE_TOKEN; return TINTLIT;
[a-zA-Z$_][a-zA-Z0-9$_]*  SAVE_TOKEN; return TIDENTIFIER;
.                         printf("Unknown token encountered in lexer\n"); yyterminate();

%%
