/* Declarations */

%{
#include <iostream>
#include <map>
#include <vector>
#include <algorithm> 
#include <cstring>
#include <stack> 
#include "vsopc.hh"

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
vector<string> keywords{
	"class",
	"and",
	"bool",
	"do",
	"if",
	"else",
	"then",
	"extends",
	"false",
	"in",
	"int32",
	"isnull",
	"let",
	"new",
	"not",
	"self",
	"string",
	"true",
	"unit",
	"while",
};

// Operators map
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
%x STRING COMMENT

whitespace 					[ \t\n\f\r]+

lowercase-letter			[a-z]
uppercase-letter			[A-Z]
letter 						{lowercase-letter}|{uppercase-letter}

bin-digit					0|1
digit 						[0-9]
hex-digit 					{digit}|[a-f]|[A-F]
integer-literal				{digit}+|"0x"{hex-digit}+
invalid-integer				{integer-literal}([A-Z]|[a-z])+
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
object-identifier  			{lowercase-letter}({letter}|{digit}|_)*

operator					"{"|"}"|"("|")"|":"|";"|","|"+"|"-"|"*"|"/"|"^"|"."|"="|"<="|"<-"|"<"

invalid-character			.

/* Rules */
%%
{whitespace} 				{/* do nothing */}

{type-identifier}			{text = string(yytext);
							return TYPE_IDENTIFIER;}
{object-identifier}			{text = string(yytext);
							auto iterator = find(keywords.begin(), keywords.end(), text);
							if (iterator != keywords.end()){
								return KEYWORD;
							}
							return OBJECT_IDENTIFIER;}

{operator}					{auto op = operators.find(string(yytext));
							if (op != operators.end()){
								text = op->second;
								return OPERATOR;
							}
							return -1;}

{integer-literal}			{text = toDecimal(yytext); return INT_LITERAL;}
{invalid-integer}			{text = string(yytext); return INVALID_INTEGER;}

{double-quote}             	{text = string(yytext); save_pos(); BEGIN(STRING); return BEGIN_STRING_LITERAL;}
<STRING>{double-quote}		{text = string(yytext); BEGIN(INITIAL); return END_STRING_LITERAL;}
<STRING>\\{lf-followed}		{}
<STRING>{regular-char}+ 	{text = string(yytext); return REGULAR_CHAR;}
<STRING>{escaped-char}		{text = stringToHex(getEscapeChar(string(yytext))); return REGULAR_CHAR;} 
<STRING><<EOF>>				{return UNEXPECTED_EOF;} 
<STRING>\n					{return STRING_ERR_EOL;} 
<STRING>{invalid-esc-seq}	{text = yytext; return INVALID_ESC_SEQ;} 

{comment-one-line}			{}
{comment-begin}				{commentStack.push({currentLine, currentColumn}); BEGIN(COMMENT);}
<COMMENT>[^\0]				{}
<COMMENT>{comment-begin}	{commentStack.push({currentLine, currentColumn});}
<COMMENT><<EOF>>			{auto row_col = commentStack.top(); setPos(row_col.first, row_col.second); return UNEXPECTED_EOF;}
<COMMENT>{comment-end}		{commentStack.pop(); if(commentStack.empty()) BEGIN(INITIAL);}

{invalid-character}			{text = yytext; return INVALID_CHARACTER;}
%%

/* User subroutines */

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
				printLexicalError(currentLine, currentColumn, text + string(" is not a invalid integer"));
				exit(-1);
			default:
				break;
		}
		token = yylex();
	}
	fclose(yyin);
	return 0;
}
