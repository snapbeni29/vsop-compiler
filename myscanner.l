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
int string_row;
int string_col;
string filename;
int opened_comments = 0;


void save_pos() {
	string_col = start_column;
	string_row = start_line;
}

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
%x STRING COMMENT

delim 						[ \t\n\r]
whitespace 					{delim}+

lowercase-letter			[a-z]
uppercase-letter			[A-Z]
letter 						{lowercase-letter}|{uppercase-letter}

bin-digit					0|1
digit 						[0-9]
hex-digit 					{digit}|[a-f]|[A-F]
integer-literal				{digit}+|"0x"{hex-digit}+
invalid-integer				"0x"({digit}|[A-Z]|[a-z])+

escaped-char				[btnr\"\\]|(x{hex-digit}{2})|(\n[ \t]*)
escape-sequence				\\{escaped-char}
regular-char				[^\0\n\"\\]
invalid-esc-seq				\\.
double-quote				"\""


comment-begin				"(*"
comment-end					"*)"
comment-one-line 			"//"[^\0\n]*


base-identifier				{letter}|{digit}|_
type-identifier 			{uppercase-letter}{base-identifier}*
object-identifier  			{lowercase-letter}({letter}|{digit}|_)*

operator					"{"|"}"|"("|")"|":"|";"|","|"+"|"-"|"*"|"/"|"^"|"."|"="|"<="|"<-"|"<"

invalid-character			.

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
{invalid-integer}			{return INVALID_INTEGER;}
{double-quote}             	{text = string(yytext); save_pos(); BEGIN(STRING); return BEGIN_STRING_LITERAL;}
<STRING>{double-quote}		{text = string(yytext); BEGIN(INITIAL); return END_STRING_LITERAL;}
<STRING>{regular-char}+ 	{text = string(yytext); return REGULAR_CHAR;}
<STRING>{escape-sequence}	{text = stringToHex(escape(string(yytext))); return REGULAR_CHAR;} 
<STRING><<EOF>>				{return UNEXPECTED_EOF;} 
<STRING>\n					{return STRING_ERR_EOL;} 
<STRING>{invalid-esc-seq}	{return INVALID_ESC_SEQ;} 

{comment-one-line}			{}
{comment-begin}				{save_pos(); BEGIN(COMMENT); opened_comments += 1;}
<COMMENT>[^\0]				{}
<COMMENT><<EOF>>			{return UNEXPECTED_EOF;}
<COMMENT>{comment-end}		{opened_comments -= 1; if(opened_comments == 0) BEGIN(INITIAL);}

{invalid-character}			{return INVALID_CHARACTER;}
%%

/* User subroutines */

int main(int argc, char** argv)
{
	int ntoken, vtoken;
	filename = string(argv[0]).substr(2);
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
			case UNEXPECTED_EOF:
				cerr << filename << ":" << string_row << ":" << string_col << ": lexical error: unexpected end of file\n";
				exit(-1);
			case STRING_ERR_EOL:
				cerr << filename << ":" << start_line << ":" << start_column << ": lexical error: unexpected line feed\n";
				exit(-1);
			case INVALID_ESC_SEQ:
				cerr << filename << ":" << start_line << ":" << start_column << ": lexical error: invalid escape sequence\n";
				exit(-1);
			case INVALID_CHARACTER:
				cerr << filename << ":" << start_line << ":" << start_column << ": lexical error: VSOP invalid character\n";
				exit(-1);
			case INVALID_INTEGER:
				cerr << filename << ":" << start_line << ":" << start_column << ": lexical error: invalid integer\n";
				exit(-1);
			default:
				break;
		}
		ntoken = yylex();
	}
	return 0;
}