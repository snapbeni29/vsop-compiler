/* Declarations */

%{
#include <iostream>
#include <map>
#include "myscanner.hh"

using namespace std;

string text;
int entier;

// Keywords data structure
map<std::string, int> keywords = {
	{"and", AND},
	{"bool", BOOL},
	{"class", CLASS},
	{"do", DO},
	{"else", ELSE},
	{"extends", EXTENDS},
	{"false", FALSE},
	{"if", IF},
	{"in", IN},
	{"int32", INT32},
	{"isnull", ISNULL},
	{"let", LET},
	{"new", NEW},
	{"not", NOT},
	{"self", SELF},
	{"string", STRING},
	{"then", THEN},
	{"true", TRUE},
	{"unit", UNIT},
	{"while", WHILE}
};

// Operators data structure
map<string, string> operators = {
	{"{", "lbrace"},
	{"}", "rbrace"},
	{"(", "lpar"},
	{")", "rpar"},
	{":", "colon"},
	{";", "semicolon"},
	{",", "comma"},
	{"+", "plus"},
	{"-", "minus"},
	{"*", "times"},
	{"/", "div"},
	{"^", "pow"},
	{".", "dot"},
	{"=", "equal"},
	{"<", "lower"},
	{"<=", "lower-equal"},
	{"<-", "assign"}
};

%}

/* Definitions */
%option noyywrap

delim 						[ \t\n\r]
whitespace 					{delim}+

lowercase-letter			[a-z]
uppercase-letter			[A-Z]
letter 						{lowercase-letter}|{uppercase-letter}

bin-digit					0|1
digit 						[0-9]
hex-digit 					{digit}|[a-f]|[A-F]

base-identifier				{letter}|{digit}|_
type-identifier 			{uppercase-letter}{base-identifier}*
object-identifier  			{lowercase-letter}({letter}|{digit}|_)*

operator					"{"|"}"|"("|")"|":"|";"|","|"+"|"-"|"*"|"/"|"^"|"."|"="|"<="|"<-"|"<"

/* Rules */
%%
{whitespace} 				{/* no action and no return */ }

{type-identifier}			{text = string(yytext);
							return TYPE_IDENTIFIER;}
{object-identifier}			{auto j = keywords.find(string(yytext));
							if (j != keywords.end()){
								text = j->first;
								return KEYWORD;
							}
							text = string(yytext);
							return OBJECT_IDENTIFIER;}

{operator}					{auto j = operators.find(string(yytext));
							if (j != operators.end()){
								text = j->second;
								return OPERATOR;
							}
							return -1;}

''

%%

/* User subroutines */
int main()
{
	int ntoken, vtoken;

	ntoken = yylex();

	/* Loop until ntoken equal 0 */
	while(ntoken){
		switch (ntoken){
			case KEYWORD:
				cout << text + "\n";
				break;
			case OBJECT_IDENTIFIER:
				cout << text + "\n";
				break;
			case TYPE_IDENTIFIER:
				cout << text + "\n";
				break;
			case OPERATOR:
				cout << text + "\n";
				break;
			default:
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