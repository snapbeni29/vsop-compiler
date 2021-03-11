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

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    INT_LITERAL = 258,
    REAL_LITERAL = 259,
    STRING_LITERAL = 260,
    TYPE_IDENTIFIER = 261,
    OBJECT_IDENTIFIER = 262,
    AND = 263,
    BOOL = 264,
    BREAK = 265,
    CLASS = 266,
    DO = 267,
    DOUBLE = 268,
    ELSE = 269,
    EXTENDS = 270,
    EXTERN = 271,
    FALSE = 272,
    FOR = 273,
    IF = 274,
    IN = 275,
    INT32 = 276,
    ISNULL = 277,
    LET = 278,
    LETS = 279,
    NEW = 280,
    NOT = 281,
    MOD = 282,
    OR = 283,
    SELF = 284,
    SSTRING = 285,
    THEN = 286,
    TO = 287,
    TRUE = 288,
    UNIT = 289,
    WHILE = 290,
    VARARG = 291,
    LBRACE = 292,
    RBRACE = 293,
    LPAR = 294,
    RPAR = 295,
    COLON = 296,
    SEMICOLON = 297,
    COMMA = 298,
    PLUS = 299,
    MINUS = 300,
    TIMES = 301,
    DIV = 302,
    POW = 303,
    DOT = 304,
    EQUAL = 305,
    NEQUAL = 306,
    LOWER = 307,
    LOWER_EQUAL = 308,
    GREATER = 309,
    GREATER_EQUAL = 310,
    ASSIGN = 311,
    UMINUS = 312
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 35 "parser.y"

	int int32;
	double doubl;
	char* id;

#line 122 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
