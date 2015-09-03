/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TIDENTIFIER = 258,
    TINTLIT = 259,
    TFLOATLIT = 260,
    TSET = 261,
    TEQUAL = 262,
    TNEQUAL = 263,
    TLT = 264,
    TLTE = 265,
    TGT = 266,
    TGTE = 267,
    TLPAREN = 268,
    TRPAREN = 269,
    TLBRACE = 270,
    TRBRACE = 271,
    TLSBRACE = 272,
    TRSBRACE = 273,
    TDOT = 274,
    TENDL = 275,
    TPLUS = 276,
    TDASH = 277,
    TSTAR = 278,
    TSLASH = 279,
    TCOMMA = 280,
    TINT = 281,
    TFLOAT = 282,
    TVOID = 283,
    TSTRUCT = 284,
    TIF = 285,
    TWHILE = 286,
    TRETURN = 287,
    TSCOLON = 288
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 16 "parser.y" /* yacc.c:1909  */

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

#line 110 "parser.hpp" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
