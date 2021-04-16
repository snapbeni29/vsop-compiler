// Include C++ needed libraries
%code top {
	#include <iostream>
	#include <cmath>
	#include <cstdlib>
	#include <map>
	#include <vector>
	#include <algorithm> 
	#include <cstring>
	#include <stack> 
}

// It is here you define the types referenced in the union
%code requires {
	// (in our header file)
	#include "tree.hh"
}

// Here define the types that the symbols of our grammar can take
%union // yylval
{
	int integer;
	string* str;
	Class* _class;
	ClassBody* body;
	Field* field;
	Method* method;
	Assign* assign;
	Formal* formal;
	Formals* formals;
	Expression* expr;
	Block* block;
}

%{
using namespace std;
int yylex(void);
void yyerror(string s);
extern FILE *yyin;

extern string text;
string filename;

// declare the list of classes of the input program
list<unique_ptr<Class>> classes;

void yyerror(string s){
	cerr << filename << ":" << yylloc.first_line << ":" << yylloc.first_column << ": " + s + "\n";
}
%}

%locations

// Keywords
%token <str> CLASS "class"
%token <str> AND "and"
%token <str> BOOL "bool"
%token <str> DO "do"
%token <str> IF "if"
%token <str> ELSE "else"
%token <str> THEN "then"
%token <str> EXTENDS "extends"
%token <str> FALSE "false"
%token <str> IN "in"
%token <str> INT32 "int32"
%token <str> ISNULL "isnull"
%token <str> LET "let"
%token <str> NEW "new"
%token <str> NOT "not"
%token <str> SELF "self"
%token <str> _STRING "string"
%token <str> TRUE "true"
%token <str> UNIT "unit"
%token <str> WHILE "while"

// Operators
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

// Literals
%token <integer> INT_LITERAL "integer-literal"
%token <str> STRING_LITERAL "string-literal"

// Identifiers
%token <str> TYPE_IDENTIFIER "type-identifier"
%token <str> OBJECT_IDENTIFIER "object-identifier"

// program is the start symbol
%start program;

// Define the types of the non-terminal symbols
%nterm <_class> class
%nterm <str> class-parent type 
%nterm <body> class-body
%nterm <field> field
%nterm <method> method
%nterm <formals> formals formal formal-supp
%nterm <block> block block-supp args arg args-supp
%nterm <expr> expr literal boolean-literal unary-op binary-op call assignment

%precedence IF THEN WHILE DO LET IN
%precedence ELSE

%right ASSIGN
%left AND
%right NOT
%nonassoc LOWER LOWER_EQUAL EQUAL
%left PLUS MINUS
%left TIMES DIV
%right ISNULL
%right POW
%left DOT

%%

// Rule definitions of our grammar

program: /* epsilon */
		| class program { classes.push_front(unique_ptr<Class>($1)); };

class: CLASS TYPE_IDENTIFIER class-parent LBRACE class-body RBRACE
		{ $$ = new Class($2, $3, $5); delete $1; delete $4; delete $6; };

class-parent:	/* epsilon */ { $$ = new string("Object"); }
				| EXTENDS TYPE_IDENTIFIER
				{ $$ = $2; delete $1; };

class-body:  	/* epsilon */ { $$ = new ClassBody(); }
				| field class-body { ($2)->addField(unique_ptr<Field>($1)); $$ = $2; }
				| method class-body { ($2)->addMethod(unique_ptr<Method>($1)); $$ = $2; };

field: OBJECT_IDENTIFIER COLON type ASSIGN expr SEMICOLON { $$ = new Field($1, $3, $5); delete $2; delete $4; delete $6; }
		| OBJECT_IDENTIFIER COLON type SEMICOLON { $$ = new Field($1, $3); delete $2; delete $4; };

assignment: OBJECT_IDENTIFIER ASSIGN expr { $$ = new Assign($1, $3); delete $2; };

method: OBJECT_IDENTIFIER LPAR formals RPAR COLON type block { $$ = new Method($1, $3, $6, $7); delete $2; delete $4; delete $5; };

type: TYPE_IDENTIFIER { $$ = $1; } 
		| INT32 { $$ = $1; }
		| BOOL { $$ = $1; }
		| _STRING { $$ = $1; }
		| UNIT { $$ = $1; }
		| OBJECT_IDENTIFIER { yyerror(*$1 + " is not a primitive type"); $$ = $1; };

formals: /*epsilon*/ { $$ = new Formals(); }
		| formal { $$ = $1; };

formal: OBJECT_IDENTIFIER COLON type formal-supp { ($4)->addFormal(unique_ptr<Formal>(new Formal($1, $3))); $$ = $4; delete $2; };

formal-supp: /*epsilon*/ { $$ = new Formals(); }
			| COMMA formal { $$ = $2; delete $1; };

block: LBRACE expr block-supp RBRACE { ($3)->addExpression(unique_ptr<Expression>($2)); $$ = $3; delete $1; delete $4; }; // To verify

block-supp: /* epsilon */ { $$ = new Block(); }
			| SEMICOLON expr block-supp { ($3)->addExpression(unique_ptr<Expression>($2)); $$ = $3; delete $1; };

expr: 	IF expr THEN expr { $$ = new If($2, $4); delete $1; delete $3; }
		| IF expr THEN expr ELSE expr { $$ = new If($2, $4, $6); delete $1; delete $3; delete $5; }
		| WHILE expr DO expr { $$ = new While($2, $4); delete $1; delete $3; }
        | LET OBJECT_IDENTIFIER COLON type IN expr { $$ = new Let($2, $4, $6); delete $1; delete $3; delete $5; }
        | LET OBJECT_IDENTIFIER COLON type ASSIGN expr IN expr { $$ = new Let($2, $4, $6, $8); delete $1; delete $3; delete $5; delete $7; }
        | assignment { $$ = $1; }
		| unary-op { $$ = $1; }
        | binary-op { $$ = $1; }
        | call { $$ = $1; }
        | NEW TYPE_IDENTIFIER { $$ = new New($2); delete $1; }
		| OBJECT_IDENTIFIER { $$ = new ObjectIdentifier($1); }
		| SELF { $$ = new ObjectIdentifier($1); }
		| literal { $$ = $1; }
		| LPAR RPAR { $$ = new UnitExpression(); delete $1; delete $2;  }
		| LPAR expr RPAR { $$ = $2;  delete $1; delete $3; }
        | block { $$ = $1; };

literal: INT_LITERAL { $$ = new IntegerExpression($1); }
		| STRING_LITERAL { $$ = new StringLitExpression($1); }
		| boolean-literal { $$ = $1; };

boolean-literal: TRUE { $$ = new BooleanLitExpression($1); }
				| FALSE { $$ = new BooleanLitExpression($1); };

args: 	/* epsilon */ { $$ = new Block(); }
		| arg { $$ = $1; };

arg: expr args-supp { ($2)->addExpression(unique_ptr<Expression>($1)); $$ = $2; };

args-supp: 	/* epsilon */ { $$ = new Block(); }
			| COMMA arg { $$ = $2; delete $1;};

call:	OBJECT_IDENTIFIER LPAR args RPAR { $$ = new Call($1, $3); delete $2; delete $4; }
		| expr DOT OBJECT_IDENTIFIER LPAR args RPAR { $$ = new Call($1, $3, $5); delete $2;  delete $4; delete $6; };

binary-op: 	expr EQUAL expr { $$ = new BinaryOperator($2, $1, $3); }
		   	| expr LOWER expr { $$ = new BinaryOperator($2, $1, $3); }
		  	| expr LOWER_EQUAL expr { $$ = new BinaryOperator($2, $1, $3); }
			| expr PLUS expr { $$ = new BinaryOperator($2, $1, $3); }
			| expr MINUS expr { $$ = new BinaryOperator($2, $1, $3); }
			| expr TIMES expr { $$ = new BinaryOperator($2, $1, $3); }
			| expr DIV expr { $$ = new BinaryOperator($2, $1, $3); }
			| expr POW expr { $$ = new BinaryOperator($2, $1, $3); };
			| expr AND expr { $$ = new BinaryOperator($2, $1, $3); };

unary-op: NOT expr { $$ = new UnaryOperator($1, $2); }
		| MINUS expr { $$ = new UnaryOperator($1, $2); }
		| ISNULL expr { $$ = new UnaryOperator($1, $2); };

%%


void semanticChecker(unique_ptr<Program>& p) {
	p->checkSemantic();
}

unique_ptr<Program> parser(){
	yyparse();
	// Create and print the program 
	unique_ptr<Program> p(new Program(move(classes)));
	return p;
}


void lexer(){
	int token;
	token = yylex();

	while(token){
		switch (token){
			// Keywords
			case CLASS:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case AND:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case BOOL:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case DO:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case IF:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case ELSE:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case THEN:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case EXTENDS:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case FALSE:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case IN:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case INT32:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case ISNULL:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case LET:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case NEW:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case NOT:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case SELF:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case _STRING:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case TRUE:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case UNIT:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case WHILE:
				cout << to_string(yylloc.first_line) + "," +  to_string(yylloc.first_column) + "," + text + "\n";
				break;

			case OBJECT_IDENTIFIER:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + "object-identifier," + text + "\n";
				break;
			case TYPE_IDENTIFIER:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + "type-identifier," + text + "\n";
				break;

			// Operators
			case LBRACE:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case RBRACE:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case LPAR:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case RPAR:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case COLON:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case SEMICOLON:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case COMMA:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case PLUS:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case MINUS:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case TIMES:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case DIV:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case POW:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case DOT:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case EQUAL:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;
			case LOWER:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	
			case LOWER_EQUAL:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;				
			case ASSIGN:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + text + "\n";
				break;	

			case STRING_LITERAL:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + "string-literal" + "," + text  + "\n";
				break;
			case INT_LITERAL:
				cout << to_string(yylloc.first_line) + "," + to_string(yylloc.first_column) + "," + "integer-literal" + "," + text + "\n";
				break;
			default:
				break;
		}
		token = yylex();
	};
}

int main(int argc, char **argv) {

	if((string(argv[1]) != "-l" && string(argv[1]) != "-p" && string(argv[1]) != "-c") || argc < 3){
		cout << "Error: the executed command is not valid." << endl;
		return -1;
	}

	yyin = fopen(argv[2], "r");
	filename = string(argv[2]);
	// Stop if error
	if (!yyin) {
		fprintf(stderr, "Failed to open input file\n");
		return EXIT_FAILURE;
	}
	unique_ptr<Program> p = nullptr;

	if (string(argv[1]) == "-l"){
		lexer();
	}
	
	else if(string(argv[1]) == "-p" || string(argv[1]) == "-c"){
		p = parser();
		if (string(argv[1]) == "-c"){
			semanticChecker(p);
		} 
		cout << p->toString();
	}
	
	// Close the file and exit
	fclose(yyin);
	return EXIT_SUCCESS;
}


