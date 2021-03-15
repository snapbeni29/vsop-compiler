%{
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm> 
#include <cstring>
#include <stack> 
#include "tree.hh"
using namespace std;

int yylex(void);
int yyerror(string s);
extern char *yytext;   /* Flex global variables */
extern int yylineno;
extern FILE *yyin;

extern int previousColumn;
extern int currentLine, currentColumn;
extern int previousLine;
extern string text;
extern int stringRow;
extern int stringCol;
extern string filename;
extern stack<pair<int, int>> commentStack;

int yyerror(string s) {
	cerr << filename << ":" << stringRow << ":" << stringCol << ": lexical error: " + s + "\n";
}
%}

%union // yylval
{
	int integer;
	char* str;
}

%token END 0 "end-of-file"

%token <integer> INT_LITERAL "integer-literal"
%token <str> STRING_LITERAL "string-literal"
%token <str> TYPE_IDENTIFIER "type-identifier"
%token <str> OBJECT_IDENTIFIER "object-identifier"

%token <str> AND "and"
%token <str> BOOL "bool"
%token <str> BREAK "break" // -ext
%token <str> CLASS "class"
%token <str> DO "do"
%token <str> DOUBLE "double" // -ext
%token <str> ELSE "else"
%token <str> EXTENDS "extends"
%token <str> EXTERN "extern" // -ext
%token <str> FALSE "false"
%token <str> FOR "for" // -ext
%token <str> IF "if"
%token <str> IN "in"
%token <str> INT32 "int32"
%token <str> ISNULL "isnull"
%token <str> LET "let"
%token <str> NEW "new"
%token <str> NOT "not"
%token <str> SELF "self"
%token <str> SSTRING "string"
%token <str> THEN "then"
%token <str> TRUE "true"
%token <str> UNIT "unit"
%token <str> WHILE "while"
%token <str> VARARG "vararg"

%token <str> LBRACE "{"
%token <str> RBRACE "}"
%token <str> LPAR "("
%token <str> RPAR ")"
%token <str> COLON ":"
%token <str> SEMICOLON ";"
%token <str> COMMA ","
%token <str> PLUS "+"
%token <str> MINUS "-"
%token <str> TIMES "*"
%token <str> DIV "/"
%token <str> POW "^"
%token <str> DOT "."
%token <str> EQUAL "="
%token <str> LOWER "<"
%token <str> LOWER_EQUAL "<="
%token <str> ASSIGN "<-"

%start program;

%precedence IF THEN WHILE DO LET IN
%precedence ELSE

%right ASSIGN
%left AND
%right NOT
%nonassoc LOWER LOWER_EQUAL EQUAL
%left PLUS MINUS
%left TIMES DIV
%right UMINUS ISNULL
%right MOD POW
%left DOT

%%

program: /* epsilon */
		| class program ;

class: CLASS TYPE_IDENTIFIER class-parent LBRACE class-body RBRACE;

class-parent:	/* epsilon */
				| EXTENDS TYPE_IDENTIFIER ;

class-body:  	/* epsilon */
				| class-body field
				| class-body method;

field: formal opt-assignment SEMICOLON ;

opt-assignment: /* epsilon */
			| assignment;

assignment: ASSIGN expr;

method: OBJECT_IDENTIFIER LPAR formals RPAR COLON type block;

type: TYPE_IDENTIFIER | "int32" | "bool" | "string" | "unit";

formals: /*epsilon*/ | formal;

formal: OBJECT_IDENTIFIER COLON type formal-supp;

formal-supp: /*epsilon*/ | COMMA formal;

block: LBRACE expr block-supp RBRACE; // To verify

block-supp: /* epsilon */ | SEMICOLON expr block-supp ;

expr: 	IF expr THEN expr 
		| IF expr THEN expr ELSE expr
		| WHILE expr DO expr
        | LET OBJECT_IDENTIFIER COLON type opt-assignment IN expr
        | OBJECT_IDENTIFIER assignment
		| unary-op
        | binary-op
        | call
        | NEW TYPE_IDENTIFIER
		| OBJECT_IDENTIFIER
		| SELF
		| literal
		| LPAR RPAR
		| LPAR expr RPAR
        | block;

literal: INT_LITERAL | STRING_LITERAL | boolean-literal;
boolean-literal: TRUE | FALSE ;

args: 	/* epsilon */ 
		| arg;

arg: expr args-supp;

args-supp: 	/* epsilon */ 
			| COMMA arg ;

call:	OBJECT_IDENTIFIER LPAR args RPAR
		| expr DOT OBJECT_IDENTIFIER LPAR args RPAR ;

binary-op: 	expr EQUAL expr
		   	| expr LOWER expr
		  	| expr LOWER_EQUAL expr
			| expr PLUS expr
			| expr MINUS expr
			| expr TIMES expr
			| expr DIV expr
			| expr POW expr;

unary-op: NOT expr
		| MINUS expr
		| ISNULL expr;

%%


int yyerror(char *s) {
  printf("%s on line %d - %s\n", s, yylineno, yytext);
}

int yywrap(void) {
  fprintf(stdout, "End of input reached\n");
  return 1;
}


int main(int argc, char **argv) {
  if (argc >= 2) {
    yyin = fopen(argv[1], "r");
	filename = string(argv[1]);
    if (!yyin) {
      fprintf(stderr, "Failed to open input file\n");
      return EXIT_FAILURE;
    }
  }
  
  if (!yyparse()) {
    fprintf(stdout, "Successful parsing\n");
  }
  
  fclose(yyin);
  fprintf(stdout, "End of processing\n");
  return EXIT_SUCCESS;
}

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
			// Keywords
			case CLASS:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case AND:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case BOOL:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case DO:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case IF:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case ELSE:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case THEN:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case EXTENDS:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case FALSE:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case IN:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case INT32:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case ISNULL:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case LET:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case NEW:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case NOT:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case SELF:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case SSTRING:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case TRUE:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case UNIT:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;
			case WHILE:
				cout << to_string(currentLine) + "," +  to_string(currentColumn) + "," + text + "\n";
				break;

			case OBJECT_IDENTIFIER:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "object-identifier," + text + "\n";
				break;
			case TYPE_IDENTIFIER:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "type-identifier," + text + "\n";
				break;

			// Operators
			case LBRACE:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;
			case RBRACE:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;
			case LPAR:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;
			case RPAR:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case COLON:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case SEMICOLON:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case COMMA:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case PLUS:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case MINUS:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case TIMES:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case DIV:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;
			case POW:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case DOT:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	
			case EQUAL:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;
			case LOWER_EQUAL:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;				
			case ASSIGN:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + text + "\n";
				break;	

			case STRING_LITERAL:
				cout << to_string(stringRow) + "," + to_string(stringCol) + "," + "string-literal" + "," + text  + "\n";
				break;
			case INT_LITERAL:
				cout << to_string(currentLine) + "," + to_string(currentColumn) + "," + "integer-literal" + "," + text + "\n";
				break;
			default:
				break;
		}
		token = yylex();
	}
	fclose(yyin);
	return 0;
}
*/

