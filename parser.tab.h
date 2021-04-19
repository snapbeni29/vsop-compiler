/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#line 14 "parser.y" /* yacc.c:1909  */

	
	// (in our header file)
	#include "tree.hh"

#line 50 "parser.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CLASS = 258,
    AND = 259,
    BOOL = 260,
    DO = 261,
    IF = 262,
    ELSE = 263,
    THEN = 264,
    EXTENDS = 265,
    FALSE = 266,
    IN = 267,
    INT32 = 268,
    ISNULL = 269,
    LET = 270,
    NEW = 271,
    NOT = 272,
    SELF = 273,
    _STRING = 274,
    TRUE = 275,
    UNIT = 276,
    WHILE = 277,
    LBRACE = 278,
    RBRACE = 279,
    LPAR = 280,
    RPAR = 281,
    COLON = 282,
    SEMICOLON = 283,
    COMMA = 284,
    PLUS = 285,
    MINUS = 286,
    TIMES = 287,
    DIV = 288,
    POW = 289,
    DOT = 290,
    EQUAL = 291,
    LOWER = 292,
    LOWER_EQUAL = 293,
    ASSIGN = 294,
    INT_LITERAL = 295,
    STRING_LITERAL = 296,
    TYPE_IDENTIFIER = 297,
    OBJECT_IDENTIFIER = 298
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 22 "parser.y" /* yacc.c:1909  */

	int integer;
	string* str;
	Class* _class;
	ClassBody* body;
	Field* field;
	Method* method;
	Assign* assign;
	Formal* formal;
	Formals* formals;
	Expression* expr;
	Block* block;
	Args* args;

#line 121 "parser.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
