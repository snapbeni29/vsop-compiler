#include <iostream> 
#include <list> 
#include <iterator> 
using namespace std; 

class Program{
    list<Class> classes;
}

class Class {
    string name;
    string parent;
    list<Field> fields;
    list<Method> methods;
}

class Field{
    string name;
    string type;
    string initExpr;
}


class Method {
    string name;
    list<Formal> formals;
    string returnType;
    Block block;
}

class Block {
    list<Expression> exprList;
}


class Formal {
    string name;
    string type;
}

class Expression {
    
}

class UnaryOperator {
    string operator;
    Expression expr;
}

class If {
    Expression conditionExpr;
    Expression thenExpr;
    Expression elseExpr;
}

class While {
    Expression conditionExpr;
    Expression bodyExpr;
}

class Assign {
    string name;
    Expression expr;
}

class BinOp{
    UnaryOperator op;
    Expression left;
    Expression right;
}


class Call {
    
}

class Let {
    string name;
    string type;
    Expression init;
    Expression scope;
}