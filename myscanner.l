/* Declarations */

%{
#include "myscanner.h"
#include <stdio.h>

int  installID(void);
int  installNum(void);

int yylval = 0;
%}

/* Definitions */
%option noyywrap

delim 						[ \t\n]
ws 							{delim}+

lowercase_letter			[a-z]
uppercase_letter			[A-Z]
letter 						{lowercase_letter}|{uppercase_letter}


digit 						[0-9]
hexadecimal_digit 			{digit}|[a-f]|

id 							{letter}({letter}|{digit})*
number 						{digit}+(\.{digit}+)?(E[+-]?{digit}+)?
base_identifier				{letter}|{digit}|_
type_identifier 			{uppercase_letter}{base_identifier}*

/* Rules */
%%
{ws} {/* no action and no return */ }
"if" {return IF;}
"then" {return THEN;}
"else" {return ELSE;}
"class" {return CLASS;}
{id} {yylval = installID (); return ID;}
{number} { yylval = installNum (); return NUMBER;}
{type_identifier} {return TYPE;}

"<" {yylval = LT; return RELOP;}
"<=" {yylval = LE; return RELOP;}
"=" {yylval = EQ; return RELOP;}
"<>" {yylval = NE; return RELOP;}
">" {yylval = GT; return RELOP;}
">=" {yylval = GE; return RELOP;}


%%


int  installID(void){
	/*  function  to  install  the  lexeme , whosefirst  character  is  pointed  to by yytext ,
	and  whose  length  is yyleng , into  thesymbol  table  and  return a pointer  thereto*/
	return 0;
}
int  installNum(void){
	/*  similar  to installID , but  puts  numericalconstants  into a separate  table  */
	return 0;
}

/* User subroutines */
int main()
{
	int ntoken, vtoken;

	ntoken = yylex();

	/* Loop until ntoken equal 0 */
	while(ntoken){
		switch (ntoken){
			case CLASS:
				printf("class, %d\n", yylineno);
				break;
			case TYPE:
				printf("type, %d\n", yylineno);
				break;
		}
		ntoken = yylex();
	}
	/*
	while(ntoken) {
		printf("%d\n", ntoken);
		if(yylex() != COLON) {
			printf("Syntax error in line %d, Expected a ':' but found %s\n", yylineno, yytext);
			return 1;
		}
		vtoken = yylex();
		switch (ntoken) {
			case TYPE:
			case NAME:
			case TABLE_PREFIX:
				if(vtoken != IDENTIFIER) {
					printf("Syntax error in line %d, Expected an identifier but found %s\n", yylineno, yytext);
					return 1;
				}
				printf("%s is set to %s\n", names[ntoken], yytext);
				break;
			case PORT:
				if(vtoken != INTEGER) {
					printf("Syntax error in line %d, Expected an integer but found %s\n", yylineno, yytext);
					return 1;
				}
				printf("%s is set to %s\n", names[ntoken], yytext);
				break;
			default:
				printf("Syntax error in line %d\n",yylineno);
		}
		ntoken = yylex();
	}
	*/
	return 0;
}