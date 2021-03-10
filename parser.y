%{
#include <iostream>
#include <cmath>
#include <cstdlib>
#define YYSTYPE double /* Define main semantic type */
int yylex(void);
int yyerror(char *s);
extern char *yytext;   /* Flex global variables */
extern int yylineno;
extern FILE *yyin;
using namespace std;
%}

%token END 0 "end-of-file"

%token <int32> INTEGER_LITERAL "integer-literal"
%token <doubl> REAL_LITERAL "real-literal" // /!\ tweak lexer to support
%token <id> STRING_LITERAL "string-literal"
%token <id> TYPE_IDENTIFIER "type-identifier"
%token <id> OBJECT_IDENTIFIER "object-identifier"

%token <id> AND "and"
%token <id> BOOL "bool"
%token <id> BREAK "break" // -ext
%token <id> CLASS "class"
%token <id> DO "do"
%token <id> DOUBLE "double" // -ext
%token <id> ELSE "else"
%token <id> EXTENDS "extends"
%token <id> EXTERN "extern" // -ext
%token <id> FALSE "false"
%token <id> FOR "for" // -ext
%token <id> IF "if"
%token <id> IN "in"
%token <id> INT32 "int32"
%token <id> ISNULL "isnull"
%token <id> LET "let"
%token <id> LETS "lets" // -ext
%token <id> NEW "new"
%token <id> NOT "not"
%token <id> MOD "mod" // -ext
%token <id> OR "or" // -ext
%token <id> SELF "self"
%token <id> SSTRING "string"
%token <id> THEN "then"
%token <id> TO "to" // -ext
%token <id> TRUE "true"
%token <id> UNIT "unit"
%token <id> WHILE "while"
%token <id> VARARG "vararg"

%token <id> LBRACE "{"
%token <id> RBRACE "}"
%token <id> LPAR "("
%token <id> RPAR ")"
%token <id> COLON ":"
%token <id> SEMICOLON ";"
%token <id> COMMA ","
%token <id> PLUS "+"
%token <id> MINUS "-"
%token <id> TIMES "*"
%token <id> DIV "/"
%token <id> POW "^"
%token <id> DOT "."
%token <id> EQUAL "="
%token <id> NEQUAL "!=" // -ext
%token <id> LOWER "<"
%token <id> LOWER_EQUAL "<="
%token <id> GREATER ">" // -ext
%token <id> GREATER_EQUAL ">=" // -ext
%token <id> ASSIGN "<-"


%token START_LEXER START_PARSER START_EXT_LEXER START_EXT_PARSER;
%start program;

%%

program: 	/* epsilon */
			| class program ;

class: CLASS TYPE_IDENTIFIER class-parent LBRACE class-body RBRACE;

class-parent:	/* epsilon */
				| EXTENDS TYPE_IDENTIFIER ;

class-body:  	/* epsilon */
				| class-body field
				| class-body method;

field: formal assignment SEMICOLON ;

assignment: /* epsilon */
			| ASSIGN expr;

method: OBJECT_IDENTIFIER LPAR formals RPAR COLON type block;

type: TYPE_IDENTIFIER | "int32" | "bool" | "string" | "unit";

formals: /*epsilon*/ | formal {COMMA, formal};

formal: OBJECT_IDENTIFIER COLON type;

block: LBRACE expr second-instr RBRACE; // To verify

second-instr: /*epsilon*/ | SEMICOLON expr ;

expr:	IF expr THEN expr else
		| WHILE expr DO expr
		| LET OBJECT_IDENTIFIER COLON type assignment IN expr
		| OBJECT_IDENTIFIER ASSIGN expr
		| NOT expr
		| expr bin-operator expr
		| MINUS expr
		| ISNULL expr
		| OBJECT_IDENTIFIER LPAR args RPAR
		| expr DOT OBJECT_IDENTIFIER LPAR args RPAR
		| NEW TYPE_IDENTIFIER
		| OBJECT_IDENTIFIER
		| SELF 
		| literal
		| LPAR RPAR 
		| LPAR expr RPAR
		| block;

else: 	/* epsilon */
		| ELSE expr ;

bin-operator: 	EQUAL 
				| LOWER 
				| LOWER_EQUAL
				| PLUS 
				| MINUS
				| TIMES
				| DIV
				| POW 
				| AND;

args: /*epsilon*/ | expr { "," expr};
literal: INTEGER_LITERAL | STRING_LITERAL | boolean-literal ;
boolean-literal: "true" | "false" ;

%%


int yyerror(char *s) {
  printf("%s on line %d - %s\n", s, yylineno, yytext);
}

int yywrap(void) {
  fprintf(stdout, "End of input reached\n");
  return 1;
}

int main(int argc, char **argv) {
  /* if any input file has been specified read from that */
  if (argc >= 2) {
    yyin = fopen(argv[1], "r");
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
