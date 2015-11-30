%{

//Includes
#include <string>
#include <stdio.h>
#include "node.h"
#include "parser.hpp"
#include "./gc/include/gc.h"
#include "./gc/include/gc_allocator.h"

//You must specify the allocator gc_alloc to STL containers to get GC memory

#define SAVE_TOKEN do { yylval.string = (char *)GC_MALLOC_ATOMIC(yyleng+1); memcpy(yylval.string,yytext,yyleng+1); } while (0)

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

<INITIAL>[ \t\r]                   ;
<INITIAL>^[ ]*\n	           ;
<INITIAL>\/\/+.*\n		   ;
<INITIAL>"\n"                      return TOKEN(TENDL);
<INITIAL>"="                       return TOKEN(TSET);
<INITIAL>"=="                      SAVE_TOKEN; return TEQUAL;
<INITIAL>"!="                      SAVE_TOKEN; return TNEQUAL;
<INITIAL>"<"                       SAVE_TOKEN; return TLT;
<INITIAL>"<="                      SAVE_TOKEN; return TLTE;
<INITIAL>">"                       SAVE_TOKEN; return TGT;
<INITIAL>">="                      SAVE_TOKEN; return TGTE;
<INITIAL>"||"                      SAVE_TOKEN; return TLOR;
<INITIAL>"&&"                      SAVE_TOKEN; return TLAND;
<INITIAL>"!"                       SAVE_TOKEN; return TLNOT;
<INITIAL>"("                       return TOKEN(TLPAREN);
<INITIAL>")"                       return TOKEN(TRPAREN);
<INITIAL>"{"                       return TOKEN(TLBRACE);
<INITIAL>"}"                       return TOKEN(TRBRACE);
<INITIAL>"["                       return TOKEN(TLSBRACE);
<INITIAL>"]"                       return TOKEN(TRSBRACE);
<INITIAL>"."                       SAVE_TOKEN; return TDOT;
<INITIAL>"+"                       SAVE_TOKEN; return TPLUS;
<INITIAL>"-"                       SAVE_TOKEN; return TDASH;
<INITIAL>"*"                       SAVE_TOKEN; return TSTAR;
<INITIAL>"/"                       SAVE_TOKEN; return TSLASH;
<INITIAL>";"                       SAVE_TOKEN; return TSCOLON;
<INITIAL>"&"			  SAVE_TOKEN; return TSAMPR;
<INITIAL>","                       return TOKEN(TCOMMA);
<INITIAL>"if"                      return TOKEN(TIF);
<INITIAL>"while"                   return TOKEN(TWHILE);
<INITIAL>"return"                  return TOKEN(TRETURN);
<INITIAL>"int"                     return TOKEN(TINT);
<INITIAL>"float"                   return TOKEN(TFLOAT);
<INITIAL>"void"                    return TOKEN(TVOID);
<INITIAL>"struct"                  return TOKEN(TSTRUCT);
<INITIAL>"extern"		  return TOKEN(TEXTERN);
<INITIAL>"else"			  return TOKEN(TELSE);
<INITIAL>"NULL"			  return TOKEN(TNULL);
<INITIAL>"new"			  SAVE_TOKEN; return TNEW;
<INITIAL>[0-9]*\.[0-9]+            SAVE_TOKEN; return TFLOATLIT;
<INITIAL>[0-9]+                    SAVE_TOKEN; return TINTLIT;
<INITIAL>[a-zA-Z$_][a-zA-Z0-9$_]*  SAVE_TOKEN; return TIDENTIFIER;
<INITIAL>.                         if (yytext) printf("Unknown token encountered in lexer: %s\n",yytext); yyterminate();

%%
