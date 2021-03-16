/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 12 "parser.y"

	#include "tree.hh"

#line 52 "parser.tab.h"

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    INT_LITERAL = 258,
    STRING_LITERAL = 259,
    TYPE_IDENTIFIER = 260,
    OBJECT_IDENTIFIER = 261,
    AND = 262,
    BOOL = 263,
    BREAK = 264,
    CLASS = 265,
    DO = 266,
    DOUBLE = 267,
    ELSE = 268,
    EXTENDS = 269,
    EXTERN = 270,
    FALSE = 271,
    FOR = 272,
    IF = 273,
    IN = 274,
    INT32 = 275,
    ISNULL = 276,
    LET = 277,
    NEW = 278,
    NOT = 279,
    SELF = 280,
    SSTRING = 281,
    THEN = 282,
    TRUE = 283,
    UNIT = 284,
    WHILE = 285,
    VARARG = 286,
    LBRACE = 287,
    RBRACE = 288,
    LPAR = 289,
    RPAR = 290,
    COLON = 291,
    SEMICOLON = 292,
    COMMA = 293,
    PLUS = 294,
    MINUS = 295,
    TIMES = 296,
    DIV = 297,
    POW = 298,
    DOT = 299,
    EQUAL = 300,
    LOWER = 301,
    LOWER_EQUAL = 302,
    ASSIGN = 303,
    UMINUS = 304,
    MOD = 305
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "parser.y"

	int integer;
	char* str;
	Class* _class;
	ClassBody* body;
	Field* field;
	Method* method;
	Assign* assign;
	Formal* formal;
	Formals* formals;
	Expression* expr;
	Block* block;

#line 129 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
