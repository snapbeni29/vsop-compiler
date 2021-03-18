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

int previousColumn = 1;
int currentLine, currentColumn;
int previousLine = yylineno;
string text = "";
int stringRow;
int stringCol;
string filename;
stack<pair<int, int>> commentStack;

// Executed before every action.
#define YY_USER_ACTION	update();

string getEscapeChar(string s);
string charToHex(char c);
string stringToHex(string escaped);
string toDecimal(string value);
void save_pos();
void printLexicalError(int row, int col, string message);
extern int yyerror(string s);


/*
Updates the token position in the file.
*/
void update() {
	currentLine = previousLine; // update line pos
	currentColumn = previousColumn; // update column pos
	for (int i = 0; i < yyleng; i++){ // if line feed, increase the line and set column to 1, otherwise increase the column
		yytext[i] == '\n' ? previousLine++, previousColumn = 1 : previousColumn++;
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
Saves the current position of the token.
*/
void save_pos() {
	stringCol = currentColumn;
	stringRow = currentLine;
}

/*
Sets a new position.
*/
void setPos(int line, int col) {
	stringCol = col;
	stringRow = line;
}

/*
Prints an error message and the position of the error.
*/
void printLexicalError(int row, int col, string message) {
	cerr << filename << ":" << row << ":" << col << ": lexical error: " + message + "\n";
}

// Keywords vector
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
							save_pos(); yyerror("lexical error: " + text + string(" is an invalid integer"));}

{double-quote}             	{text = string(yytext); save_pos(); BEGIN(STRING);}
<STRING>{double-quote}		{text += string(yytext);
							string* tmp = new string(text.c_str());
							yylval.str = tmp;
							BEGIN(INITIAL); return STRING_LITERAL;}
<STRING>\\{lf-followed}		{}
<STRING>{regular-char}+ 	{text += string(yytext);}
<STRING>{escaped-char}		{text += stringToHex(getEscapeChar(string(yytext)));} 
<STRING><<EOF>>				{yyerror("lexical error: " + string("unexpected end of file"));} 
<STRING>\n					{save_pos(); yyerror("lexical error: " + string("unexpected line feed"));} 
<STRING>{invalid-esc-seq}	{text = yytext;
							save_pos(); yyerror("lexical error: " + text + string(" is an invalid escape sequence"));} 

{comment-one-line}			{}
{comment-begin}				{commentStack.push({currentLine, currentColumn}); BEGIN(COMMENT);}
<COMMENT>[^\0]				{}
<COMMENT>{comment-begin}	{commentStack.push({currentLine, currentColumn});}
<COMMENT><<EOF>>			{auto row_col = commentStack.top(); setPos(row_col.first, row_col.second);
							yyerror("lexical error: " + string("unexpected end of file"));}
<COMMENT>{comment-end}		{commentStack.pop(); if(commentStack.empty()) BEGIN(INITIAL);}

{invalid-character}			{text = yytext; save_pos(); yyerror("lexical error: " + text + string(" is not a VSOP valid character"));}
%%

/* User subroutines */
/*
int main(int argc, char* argv[])
{
	if(string(argv[1]) != "-lex" || argc < 3){
		cout << "Error: the executed command is not valid." << endl;
		return -1;
	}

	yyin = fopen(argv[2], "r");
	filename = string(argv[2]);
	int token;

	token = yylex();

	while(token){
		switch (token){
			case KEYWORD:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case OBJECT_IDENTIFIER:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "object-identifier," + text + "\n";
				break;
			case TYPE_IDENTIFIER:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "type-identifier," + text + "\n";
				break;
			case OPERATOR:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;
			case BEGIN_STRING_LITERAL:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "string-literal" + "," + text ;
				break;
			case REGULAR_CHAR:
				cout << text;
				break;
			case END_STRING_LITERAL:
				cout << text + "\n";
				break;
			case INT_LITERAL:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "integer-literal" + "," + text + "\n";
				break;
			case UNEXPECTED_EOF:
				printLexicalError(stringRow, stringCol, string("unexpected end of file"));
				exit(-1);
			case STRING_ERR_EOL:
				printLexicalError(currentLine, currentColumn, string("unexpected line feed"));
				exit(-1);
			case INVALID_ESC_SEQ:
				printLexicalError(currentLine, currentColumn, text + string(" is an invalid escape sequence"));
				exit(-1);
			case INVALID_CHARACTER:
				printLexicalError(currentLine, currentColumn, text + string(" is not a VSOP valid character"));
				exit(-1);
			case INVALID_INTEGER:
				printLexicalError(currentLine, currentColumn, text + string(" is an invalid integer"));
				exit(-1);
			default:
				break;
		}
		token = yylex();
	}
	fclose(yyin);
	return 0;
}
*/