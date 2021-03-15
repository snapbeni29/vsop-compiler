#include <iostream>
#include <list> 
#include <string>
#include <iterator> 
using namespace std; 

class Expression {
    public:
        Expression(){}
        virtual ~Expression(){}
        virtual string toString() = 0;
};

class UnaryOperator : public Expression{
    public:
        string op;
        Expression* expr;
        UnaryOperator(){}
        UnaryOperator(string _operator, Expression* _expr){
            op = _operator;
            expr = _expr;
        }
        string toString(){
            return "UnOp(" + op + ", " + expr->toString() + ")";
        }
};

class Block : public Expression{
    public:
        list<Expression*> exprList;
        Block(){}
        Block(list<Expression*> _exprList){
            exprList = _exprList;
        }

        string toString() {
            if (exprList.size() == 1) {
                return exprList.back()->toString();
            }
            string exprsToStr = "["; list<Expression*>::iterator f_it;
            for (f_it = exprList.begin(); f_it != exprList.end(); f_it++) {
                exprsToStr += (*f_it)->toString() + ", ";
            }
            if (exprsToStr.length() > 1 && exprsToStr.substr(exprsToStr.length()-2) == ", ") {
                exprsToStr = exprsToStr.substr(0, exprsToStr.length()-2);
            }
            exprsToStr += "]";
            return exprsToStr;
        }
};


class Formal {
    public:
        string name;
        string type;
        Formal(){}
        Formal(string _name, string _type){
            name = _name;
        }
        string toString(){
            return name + " : " + type;
        }
};

class Method {
    public:
        string name;
        list<Formal*> formals;
        string returnType;
        Block* block;
        Method(){}
        Method(string _name, list<Formal*> _formals, string _returnType, Block* _block) {
            name = _name;
            formals = _formals;
            returnType = _returnType;
            block = _block;
        }

        string toString() {
            // Method(init, [hd : int32, tl : List], Cons,[Assign(head, hd), Assign(tail, tl), self])
            string formalsToStr = "["; list<Formal*>::iterator f_it;
            for (f_it = formals.begin(); f_it != formals.end(); f_it++) {
                formalsToStr += (*f_it)->toString() + ", ";
            }
            if (formalsToStr.length() > 1 && formalsToStr.substr(formalsToStr.length()-2) == ", ") {
                formalsToStr = formalsToStr.substr(0, formalsToStr.length()-2);
            }
            formalsToStr += "]";
            return "Method(" + name + ", " + formalsToStr + ","  + returnType + block->toString() + ")";
        }
};

class Field : public Expression{
    public:
        string name;
        string type;
        Expression* initExpr;
        Field(){}
        Field(string _name, string _type, Expression* _initExpr){
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
            if (initExpr->toString() != "") {
                fieldStr += ", " + initExpr->toString();
            }
            fieldStr += ")";
            return fieldStr;
        }
};

class Class {
    public:
        string name;
        string parent;
        list<Field*> fields;
        list<Method*> methods;
        Class(){}
        Class(string _name, string _parent, list<Field*> _fields, list<Method*> _methods) {
            name = _name;
            parent = _parent;
            fields = _fields;
            methods = _methods;
        }
        string toString() {
            string fieldsToStr = "["; list<Field*>::iterator f_it;
            string methodsToStr = "[";list<Method*>::iterator m_it;
            for (f_it = fields.begin(); f_it != fields.end(); f_it++) {
                fieldsToStr += (*f_it)->toString() + ", ";
            }
            for (m_it = methods.begin(); m_it != methods.end(); m_it++) {
                methodsToStr += (*m_it)->toString() + ", ";
            }
            if (fieldsToStr.length() > 1 && fieldsToStr.substr(fieldsToStr.length()-2) == ", ") {
                fieldsToStr = fieldsToStr.substr(0, fieldsToStr.length()-2);
            }
            if (methodsToStr.length() > 1 && methodsToStr.substr(methodsToStr.length()-2) == ", ") {
                methodsToStr = methodsToStr.substr(0, methodsToStr.length()-2);
            }
            fieldsToStr += "]";
            methodsToStr += "]";
            return "Class(" + name + ", " + parent + ", " + fieldsToStr + ", " + methodsToStr + ")";
        }
};

class Program{
    public:
        list<Class*> classes;
        Program(){}
        Program(list<Class*> classList) {
            classes = classList;
        }
        string toString() {
            string classesToStr = "["; list<Class*>::iterator f_it;
            for (f_it = classes.begin(); f_it != classes.end(); f_it++) {
                classesToStr += (*f_it)->toString() + ", ";
            }
            if (classesToStr.length() > 1 && classesToStr.substr(classesToStr.length()-2) == ", ") {
                classesToStr = classesToStr.substr(0, classesToStr.length()-2);
            }
            classesToStr += "]";
            return classesToStr;
        }
};

class If : public Expression {
    public:
        Expression* conditionExpr;
        Expression* thenExpr;
        Expression* elseExpr;
        If(){}
        If(Expression* _conditionExpr, Expression* _thenExpr, Expression* _elseExpr){
            conditionExpr = _conditionExpr;
            thenExpr = _thenExpr;
            elseExpr = _elseExpr;
        }
        If(Expression* _conditionExpr, Expression* _thenExpr){
            conditionExpr = _conditionExpr;
            thenExpr = _thenExpr;
        }
        string toString() {
            string content = "If(" + conditionExpr->toString() + ", " + thenExpr->toString();
            if (elseExpr->toString() != "") {
                content += ", " + elseExpr->toString();
            }
            content += ")";
            return content;
        }
};

class While : public Expression {
    public:
        Expression* conditionExpr;
        Expression* bodyExpr;
        While(){}
        While(Expression* _conditionExpr, Expression* _bodyExpr){
            conditionExpr = _conditionExpr;
            bodyExpr = _bodyExpr;
        }
        string toString() {
            return "While(" + conditionExpr->toString() + ", " + bodyExpr->toString() + ")";
        }
};

class Assign : public Expression{
    public:
        string name;
        Expression* expr;
        Assign(){}
        Assign(string _name, Expression* _expr){
            name = _name;
            expr = _expr;
        }
        string toString() {
            return "Assign(" + name + ", " + expr->toString() + ")";;
        }
};

class BinOp : public Expression{
    public:
        string op;
        Expression* left;
        Expression* right;
        BinOp(){}
        BinOp(string _op, Expression* _left, Expression* _right){
            op = _op;
            left = _left;
            right = _right;
        }
        string toString() {
            return "BinOp(" + op + ", " + left->toString() + ", " + right->toString() + ")";
        }
};


class Call : public Expression{
    public:
        Expression* objExpr;
        string methodName;
        list<Expression*> args;
        Call(){}
        Call(Expression* _objExpr, string _methodName, list<Expression*> _args){
            objExpr = _objExpr;
            methodName = _methodName;
            args = _args;
        }
        string toString() {
            string obj = "self";
            if (objExpr->toString() != "") {
                obj = objExpr->toString();
            }
            string argsStr = "["; list<Expression*>::iterator e_it;
            for (e_it = args.begin(); e_it != args.end(); e_it++) {
                argsStr += (*e_it)->toString() + ", ";
            }
            if (argsStr.length() > 1 && argsStr.substr(argsStr.length()-2) == ", ") {
                argsStr = argsStr.substr(0, argsStr.length()-2);
            }
            argsStr += "]";
            return "Call(" + obj + ", " + methodName + ", " + argsStr + ")";
        }
};

class Let : public Expression{
    public:
        string name;
        string type;
        Expression* init;
        Expression* scope;
        Let(){}
        Let(string _name, string _type, Expression* _init, Expression* _scope){
            name = _name;
            type = _type;
            init = _init;
            scope = _scope;
        }
        Let(string _name, string _type, Expression* _scope){
            name = _name;
            type = _type;
            scope = _scope;
        }
        string toString() {
            string firstPart = "Let(" + name + ", " + type + ", ";
            string lastPart = "";
            if (init != NULL) {
                lastPart += init->toString() + ", ";
            }
            lastPart += scope->toString() + ")";
            return firstPart + lastPart;   
        }
};

class New : public Expression{
    public:
        string type;
        New(){}
        New(string _type){
            type = _type;
        }
        string toString() {
            return "New(" + type + ")";
        }
};