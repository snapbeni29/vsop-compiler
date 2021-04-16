/* Declarations */
%{
#include <iostream>
#include <map>
#include <vector>
#include <algorithm> 
#include <cstring>
#include <stack>
#include <memory>
#include "parser.tab.h"

using namespace std;

string text = "";
stack<pair<int, int>> commentStack;
stack<pair<int, int>> stringStack;

// Executed before every action.
#define YY_USER_ACTION	update();

string getEscapeChar(string s);
string charToHex(char c);
string stringToHex(string escaped);
string toDecimal(string value);
void save_pos();
extern void yyerror(string s);

/*
Updates the token position in the file.
*/
void update() {
	yylloc.first_line = yylloc.last_line; 
    yylloc.first_column = yylloc.last_column; 
    for(int i = 0; yytext[i] != '\0'; i++) { 
        if(yytext[i] == '\n') { 
            yylloc.last_line++; 
            yylloc.last_column = 1; 
        } else { 
            yylloc.last_column++; 
        } 
    }
}

/*
Reads the representation of an escaped character and transforms it
into a real escaped character (put in a string).
Returns a string with the escaped char at 1st position if it is a valid one,
returns the input string otherwise.
*/
string getEscapeChar(string s){
	string escaped = "";
	switch(s[1]) {
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

/*
Returns the hex value of the input character
*/
string charToHex(char c) {
	char out[5];
	sprintf(out, "\\x%02x", c);
	return out;
}

/*
Returns the hex value of the input string if it contains a single character,
returns the original string otherwise.
*/
string stringToHex(string escaped) {
	if(escaped.length() > 1 || escaped.empty()){
		return escaped;
	}
	else{
		char c = escaped[0];
		return charToHex(c);
	}
}

/*
Converts a string representation of an integer to the string
representation of its decimal version.
Returns a string representing the number in base 10.
*/
string toDecimal(string value) {
	int nb;
	if(value.find("0x") == 0){ // hex string
		nb = stoi(value.substr(2), 0, 16);
	}
	else{ // decimal string
		nb = stoi(value);
	}
	return to_string(nb);
}

/*
Sets a new position.
*/
void setPos(int line, int col) {
	yylloc.first_column = col;
	yylloc.first_line = line;
}

// Keywords map
map<string, int> keywords{
	{"class", CLASS},
	{"and", AND},
	{"bool", BOOL},
	{"do", DO},
	{"if", IF},
	{"else", ELSE},
	{"then", THEN},
	{"extends", EXTENDS},
	{"false", FALSE},
	{"in", IN},
	{"int32", INT32},
	{"isnull", ISNULL},
	{"let", LET},
	{"new", NEW},
	{"not", NOT},
	{"self", SELF},
	{"string", _STRING},
	{"true", TRUE},
	{"unit", UNIT},
	{"while", WHILE}
};

struct op { int code; string val; };

// Operators map
map<string, op> operators = {
	{"{", {LBRACE, "lbrace"}},
	{"}", {RBRACE, "rbrace"}},
	{"(", {LPAR, "lpar"}},
	{")", {RPAR, "rpar"}},
	{":", {COLON, "colon"}},
	{";", {SEMICOLON, "semicolon"}},
	{",", {COMMA, "comma"}},
	{"+", {PLUS, "plus"}},
	{"-", {MINUS, "minus"}},
	{"*", {TIMES, "times"}},
	{"/", {DIV, "div"}},
	{"^", {POW, "pow"}},
	{".", {DOT, "dot"}},
	{"=", {EQUAL, "equal"}},
	{"<", {LOWER, "lower"}},
	{"<=", {LOWER_EQUAL, "lower-equal"}},
	{"<-", {ASSIGN, "assign"}}
};

%}


/* Definitions */
%option noyywrap
%option yylineno
%x STRING COMMENT

whitespace 					[ \t\n\f\r]+

lowercase-letter			[a-z]
uppercase-letter			[A-Z]
letter 						{lowercase-letter}|{uppercase-letter}

bin-digit					0|1
digit 						[0-9]
hex-digit 					{digit}|[a-f]|[A-F]
integer-literal				{digit}+|"0x"{hex-digit}+
invalid-integer				{integer-literal}({letter}|{digit})+
lf-followed					\n[ \t]*
escape-sequence				[btnr\"\\]|(x{hex-digit}{2})|{lf-followed}
escaped-char				\\{escape-sequence}
regular-char				[^\b\t\r\n\"\\]
double-quote				"\""
invalid-esc-seq				\\.

comment-begin				"(*"
comment-end					"*)"
comment-one-line 			"//"[^\n]*

base-identifier				{letter}|{digit}|_
type-identifier 			{uppercase-letter}{base-identifier}*
object-identifier  			{lowercase-letter}{base-identifier}*

operator					"{"|"}"|"("|")"|":"|";"|","|"+"|"-"|"*"|"/"|"^"|"."|"="|"<="|"<-"|"<"

invalid-character			.

/* Rules */
%%
{whitespace} 				{/* do nothing */}

{type-identifier}			{text = string(yytext);
							string* tmp = new string(yytext);
							yylval.str = tmp;
							return TYPE_IDENTIFIER;}
{object-identifier}			{text = string(yytext);
							string* tmp = new string(yytext);
							yylval.str = tmp;
							auto iterator = keywords.find(text);
							if (iterator != keywords.end()){
								return iterator->second;
							}
							return OBJECT_IDENTIFIER;}

{operator}					{auto op = operators.find(string(yytext));
							if (op != operators.end()){
								text = op->second.val;
								string* tmp = new string(yytext);
								yylval.str = tmp;
								return op->second.code;
							}
							return -1;}

{integer-literal}			{text = toDecimal(yytext);
							yylval.integer = stoi(toDecimal(yytext));
							return INT_LITERAL;}
{invalid-integer}			{text = string(yytext);
							yyerror("lexical error: " + text + string(" is an invalid integer")); exit(-1);}

{double-quote}             	{text = string(yytext);
							stringStack.push({yylloc.first_line, yylloc.first_column});
							BEGIN(STRING);}
<STRING>{double-quote}		{text += string(yytext);
							string* tmp = new string(text.c_str());
							yylval.str = tmp;
							auto row_col = stringStack.top(); 
							stringStack.pop(); setPos(row_col.first, row_col.second);
							BEGIN(INITIAL); return STRING_LITERAL;}
<STRING>\\{lf-followed}		{}
<STRING>{regular-char}+ 	{text += string(yytext);}
<STRING>{escaped-char}		{text += stringToHex(getEscapeChar(string(yytext)));} 
<STRING><<EOF>>				{auto row_col = stringStack.top(); 
							stringStack.pop(); setPos(row_col.first, row_col.second); 
							yyerror("lexical error: " + string("unexpected end of file")); exit(-1);} 
<STRING>\n					{yyerror("lexical error: " + string("unexpected line feed")); exit(-1);} 
<STRING>{invalid-esc-seq}	{text = yytext;
							yyerror("lexical error: " + text + string(" is an invalid escape sequence")); exit(-1);} 

{comment-one-line}			{}
{comment-begin}				{commentStack.push({yylloc.first_line, yylloc.first_column}); BEGIN(COMMENT);}
<COMMENT>[^\0]				{}
<COMMENT>{comment-begin}	{commentStack.push({yylloc.first_line, yylloc.first_column});}
<COMMENT><<EOF>>			{auto row_col = commentStack.top(); setPos(row_col.first, row_col.second);
							yyerror("lexical error: " + string("unexpected end of file")); exit(-1);}
<COMMENT>{comment-end}		{commentStack.pop(); if(commentStack.empty()) BEGIN(INITIAL);}

{invalid-character}			{text = yytext; yyerror("lexical error: " + text + string(" is not a VSOP valid character")); exit(-1);}
%%
