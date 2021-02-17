/* Declarations */

%{
#include <iostream>
#include <map>
#include "myscanner.hh"

int yycolumn = 1;
int start_line, start_column;
int prev_yylineno = yylineno;

/* This is executed before every action. */
#define YY_USER_ACTION                                                   \
  start_line = prev_yylineno; start_column = yycolumn;                   \
  if (yylineno == prev_yylineno) yycolumn += yyleng;                     \
  else {                                                                 \
    for (yycolumn = 1; yytext[yyleng - yycolumn] != '\n'; ++yycolumn) {} \
    prev_yylineno = yylineno;                                            \
  }

using namespace std;

string escape(string s);
string escape(string s){
	string escaped = "";
	switch(s[1]) {
		case '\n': return "";
		case '\0': break;
		case 'n': escaped.push_back('\n'); break;
		case 't': escaped.push_back('\t'); break;
		case 'r': escaped.push_back('\r'); break;
		case 'b': escaped.push_back('\b'); break;
		case '\\': escaped.push_back('\\'); break;
		case '\"': escaped.push_back('\"'); break;
		case 'x': return s;
		default: return s;
	}
	return escaped;
}

string charToHex(char c) {
	char out[5];
	sprintf(out, "\\x%02x", c);
	return out;
}


string stringToHex(string escaped) {
	if (escaped.length() > 1 || escaped.empty()) {
		return escaped;
	} else {
		char c = escaped[0];
		return charToHex(c);
	}
}

string toDecimal(string value) {
	int nb;
	if (value.find("0x") == 0) { // hex string
		nb = stoi(value.substr(2), 0, 16);
	} else { // decimal string
		nb = stoi(value);
	}
	return to_string(nb);
}


string text = "";
string buf = "";

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
	{"string", SELF},
	{"then", THEN},
	{"true", TRUE},
	{"unit", UNIT},
	{"while", WHILE},
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
%option yylineno
%x STRING

delim 						[ \t\n\r]
whitespace 					{delim}+

lowercase-letter			[a-z]
uppercase-letter			[A-Z]
letter 						{lowercase-letter}|{uppercase-letter}

bin-digit					0|1
digit 						[0-9]
hex-digit 					{digit}|[a-f]|[A-F]
integer-literal				{digit}+|"0x"{hex-digit}+

escape-sequence				[btnr\"\\]|(x{hex-digit}{2})|(\n[ \t]*)
escaped-char				\\{escape-sequence}
regular-char				[^\0\n\"\\]
begin						"\""
string-literal				\"({regular-char}|{escaped-char})*\"

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

{integer-literal}			{text = toDecimal(yytext); return INT_LITERAL;}

{begin}                  	{text = string(yytext); BEGIN(STRING); return BEGIN_STRING_LITERAL;}
<STRING>{begin}				{text = string(yytext); BEGIN(INITIAL); return END_STRING_LITERAL;}
<STRING>{regular-char}+ 	{text = string(yytext); return REGULAR_CHAR;}
<STRING>{escaped-char}		{text = stringToHex(escape(string(yytext))); return REGULAR_CHAR;} 

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
				cout << to_string(start_line) + "," +  to_string(start_column) + "," + text + "\n";
				break;
			case OBJECT_IDENTIFIER:
				cout << to_string(start_line) + "," + to_string(start_column) + "," + "object-identifier," + text + "\n";
				break;
			case TYPE_IDENTIFIER:
				cout << to_string(start_line) + "," + to_string(start_column) + "," + "type-identifier," + text + "\n";
				break;
			case OPERATOR:
				cout << to_string(start_line) + "," + to_string(start_column) + "," + text + "\n";
				break;
			case BEGIN_STRING_LITERAL:
				cout << to_string(start_line) + "," + to_string(start_column) + "," + "string-literal" + "," + text ;
				break;
			case REGULAR_CHAR:
				cout << text ;
				break;
			case END_STRING_LITERAL:
				cout << text + "\n";
				break;
			case INT_LITERAL:
				cout << to_string(start_line) + "," + to_string(start_column) + "," + "integer-literal" + "," + text + "\n";
				break;
			default:
				break;
		}
		ntoken = yylex();
	}
	return 0;
}