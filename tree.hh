#include <iostream> 
#include <list> 
#include <iterator> 
using namespace std; 

class Program{
    public:
        list<Class> classes;
        Program(list<Class> classList) {
            classes = classList;
        }
}

class Class {
    public:
        string name;
        string parent;
        list<Field> fields;
        list<Method> methods;
        Class(string _name, string _parent, list<Field> _fields, list<Method> _methods) {
            name = _name;
            parent = _parent;
            fields = _fields;
            methods = _methods;
        }
        string toString() {
            string fieldsToStr = "["; list<Field>::iterator f_it;
            string methodsToStr = "[";list<Method>::iterator m_it;
            for (f_it = fields.begin(); f_it != fields.end(); f_it++) {
                fieldsToStr += f_it->toString() + ", ";
            }
            for (m_it = methods.begin(); m_it != methods.end(); m_it++) {
                methodsToStr += m_it->toString() + ", ";
            }
            if (fieldsToStr.substr(fieldsToStr.length()-2) == ", ") {
                fieldsToStr = fieldsToStr.substr(0, fieldsToStr.length()-2);
            }
            if (methodsToStr.substr(methodsToStr.length()-2) == ", ") {
                methodsToStr = methodsToStr.substr(0, methodsToStr.length()-2);
            }
            fieldsToStr += "]";
            methodsToStr += "]";
            return "Class(" + name + ", " + parent + ", " + fieldsToStr + ", " + methodsToStr + ")";
        }
}

class Field{
    public:
        string name;
        string type;
        string initExpr;
        Field(string _name, string _type, string initExpr){
            name = _name;
            type = _type;
            initExpr = _initExpr;
        }
        Field(string _name, string _type){
            name = _name;
            type = _type;
        }
        string toString() {
            string fieldStr = "Field(" + name + ", " + type;
            if (initExpr != "") {
                fieldStr += ", " + initExpr;
            }
            fieldStr += ")";
            return fieldStr;
        }
}


class Method {
    public:
        string name;
        list<Formal> formals;
        string returnType;
        Block block;
        Method(string _name, list<Formal> _formals, string _returnType, Block _block) {
            name = _name;
            formals = _formals;
            returnType = _returnType;
            block = _block;
        }
}

class Block {
    public:
        list<Expression> exprList;
        Block(list<Expression> _exprList){
            exprList = _exprList;
        }
}


class Formal {
    public:
        string name;
        string type;
        Formal(string _name, string _type){
            name = _name;
        }
}

class Expression {
    public:
}

class UnaryOperator {
    public:
        string operator;
        Expression expr;
        UnaryOperator(string _operator, Expression _expr){
            operator = _operator;
            expr = _expr;
        }
}

class If {
    public:
        Expression conditionExpr;
        Expression thenExpr;
        Expression elseExpr;
        If(Expression _conditionExpr, Expression _thenExpr, Expression _elseExpr){
            conditionExpr = _conditionExpr;
            thenExpr = _thenExpr;
            elseExpr = _elseExpr;
        }
        If(Expression _conditionExpr, Expression _thenExpr){
            conditionExpr = _conditionExpr;
            thenExpr = _thenExpr;
        }
}

class While {
    public:
        Expression conditionExpr;
        Expression bodyExpr;
        While(Expression _conditionExpr, Expression _bodyExpr){
            conditionExpr = _conditionExpr;
            bodyExpr = _bodyExpr;
        }
}

class Assign {
    public:
        string name;
        Expression expr;
        Assign(string _name, string _expr){
            name = _name;
            expr = _expr;
        }
}

class BinOp{
    public:
        string op;
        Expression left;
        Expression right;
        BinOp(Expression _op, Expression _left, Expression _right){
            op = _op;
            left = _left;
            right = _right;
        }
}


class Call {
    public:
        Expression objExpr;
        string methodName;
        list<Expression> args;
        Call(Expression _objExpr, string _methodName, list<Expression> _args){
            objExpr = _objExpr;
            methodName = _methodName;
            args = _args;
        }
}

class Let {
    public:
        string name;
        string type;
        Expression init;
        Expression scope;
        Let(string _name, string _type, Expression _init, Expression _scope){
            name= _name;
            type = _type;
            init = _init;
            scope = _scope;
        }
        Let(string _name, string _type, Expression _init){
            name= _name;
            type = _type;
            init = _init;
        }
}

class New{
    public:
        string type;
        New(string _type){
            type = _type;
        }
}