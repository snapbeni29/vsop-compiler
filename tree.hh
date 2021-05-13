#include <iostream>
#include <list> 
#include <string>
#include <iterator>
#include <memory>
#include "llvm.hh"

using namespace std;
using namespace llvm;

extern string filename;
extern int errors;

class Class;

class Utils {
    public:
        static bool inheritsFrom(string child_class, string parent_class, map<string, unique_ptr<Class> *> & classesByName);
};


struct Position {
    int row;
    int col;
};

class TreeNode {
    public: 
        TreeNode(Position p) : position(p) {}
        TreeNode() {}
        Position position;
        void printError(string s, Position p);
};

class Expression : public TreeNode {
    public:
        Expression(Position p) : TreeNode(p) {}
        Expression() {}
        virtual ~Expression(){}
        virtual string toString(bool c, map<string, unique_ptr<Class> *> classesByName) = 0;
        virtual void checkTypes(map<string, unique_ptr<Class> *> & classesByName) = 0;
        virtual void checkUndefinedIdentifiers() = 0;
        virtual void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) = 0;
        virtual string getType(map<string, unique_ptr<Class> *> & classesByName) = 0;
        virtual void setExpressionClasses(string name) = 0;
        virtual void set_scope_context(map<string, string> &identifiers) = 0;
        virtual llvm::Value* codegen(LLVM& ll) {};
        bool isPrimitive(string type);
};

class UnOp : public Expression {
    public:
        unique_ptr<string> op;
        unique_ptr<Expression> expr;
        string class_name;
        UnOp(){}
        UnOp(string* _operator, Expression* _expr, Position p) : Expression(p), op(_operator), expr(_expr) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};

class Block : public Expression {
    public:
        list<unique_ptr<Expression>> exprList;
        string class_name;
        Block(){
        }

        void addExpression(unique_ptr<Expression> e);
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};


class Formal : public TreeNode {
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        string class_name;
        Formal(){}
        Formal(string* _name, string* _type, Position p) : TreeNode(p), name(_name), type(_type) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void setClass(string name);
};

class Formals {
    public:
        list<unique_ptr<Formal>> formals;
        Formals(){}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void addFormal(unique_ptr<Formal> f);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
};

class Method : public TreeNode {
    public:
        unique_ptr<string> name;
        unique_ptr<Formals> formals;
        unique_ptr<string> returnType;
        unique_ptr<Block> block;
        list<string> formal_names;
        string class_name;
        map<string, string> scope_context;
        Method(){}
        Method(string* _name, Formals* _formals, string* _returnType, Block* _block, Position p) : TreeNode(p), name(_name), formals(_formals), returnType(_returnType), block(_block) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkFormalArgumentsRedefinition();
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> & identifiers);
        void checkMain();

};

class Methods {
    public:
        list<unique_ptr<Method>> methods;
        Methods() {
        }

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void addMethod(unique_ptr<Method> m);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> & identifiers);
};

class Field : public Expression {
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        unique_ptr<Expression> initExpr = nullptr;
        string class_name;
        map<string, string> scope_identifiers;
        Field(){}
        Field(string* _name, string* _type, Expression* _initExpr, Position p) : Expression(p), name(_name), type(_type), initExpr(_initExpr) {}
        Field(string* _name, string* _type, Position p) : Expression(p), name(_name), type(_type) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void set_scope_context(map<string, string> &identifiers);
};

class Fields {
    public:
        list<unique_ptr<Field>> fields;
        Fields(){}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void addField(unique_ptr<Field> f);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void checkUndefinedIdentifiers();
        void set_scope_context(map<string, string> & identifiers);
};

class ClassBody : public TreeNode {
    public:
        unique_ptr<Fields> fields;
        unique_ptr<Methods> methods;
        map<string, string> method_types;
        map<string, string> field_types;
        string class_name;

        ClassBody() {
            unique_ptr<Fields> _fields(new Fields());
            fields = move(_fields);
            unique_ptr<Methods> _methods(new Methods());
            methods = move(_methods);
        }
        ClassBody(Fields* _fields, Methods* _methods) : fields(_fields), methods(_methods) {}
        
        llvm::Value* codegen(LLVM& ll);
        void addField(unique_ptr<Field> field);
        void addMethod(unique_ptr<Method> method);
        void checkMethodsRedefinitions();
        map<string, string> getMethodsAndTypes();
        // set the map of all known identifiers and their types
        void set_scope_context(map<string, string> & identifiers);
        map<string, string> getFieldsAndTypes();
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void checkMain();

};

class Class : public TreeNode {
    public:
        unique_ptr<string> name;
        unique_ptr<string> parent;
        unique_ptr<ClassBody> classBody;
        Class(Position p) : TreeNode(p) {}
        Class(string* _name, string* _parent, ClassBody* _classBody, Position p) : TreeNode(p), name(_name), parent(_parent), classBody(_classBody) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        llvm::Value* codegen(LLVM& ll);
        void checkFieldsRedefinitions(map<string, unique_ptr<Class>*> classesByName);
        int countFieldDefinitions(list<unique_ptr<Field>> & field_list, string field_name);
        string isFieldAlreadyDefined(map<string, unique_ptr<Class>*> classesByName, string currentClass, string field_name);
        void checkMethodsRedefinitions();
        map<string, string> getFieldsAndTypes();
        map<string, string> getMethodsAndTypes();
        void checkUndefinedIdentifiers();
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void checkMain();
        // set the map of all known identifiers and their types
        void set_scope_context(map<string, unique_ptr<Class>*> & classesByName);
        void includeParentFields(map<string, unique_ptr<Class> *> & classesByName, string parent_name, list<string> seen_classes, map<string, string> & identifiers);
};

class Program : public TreeNode {
    public:
        Class* obj = createObjectClass();
        unique_ptr<Class> object_class = unique_ptr<Class>(obj);
        map<string, unique_ptr<Class>*> classesByName;
        list<unique_ptr<Class>> classes;
        Program(list<unique_ptr<Class>> classList) {
            classes.push_back(move(object_class));
            for(auto& c : classList) {
                classes.push_back(move(c));
            }
        }

        llvm::Value* codegen(LLVM& ll);
        Class* createObjectClass();
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkSemantic();
        void set_scope_context();
        void checkCallsToUndefinedMethods();
        void setExpressionClasses();
        void checkMain(map<string, unique_ptr<Class> *> classesByName);
        void checkClasses();
        void checkUndefinedIdentifiers();
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkParentMethods(unique_ptr<Class>& childClass, unique_ptr<Class>& parentClass);
        bool isMethodInClass(unique_ptr<Method> & method, unique_ptr<Class> & class_ptr);
        void checkInheritanceCycles(list<unique_ptr<Class>>& classes);
        string hasInheritanceCycle(string class_name, list<string> ancestors);

};

class If : public Expression {
    public:
        unique_ptr<Expression> conditionExpr;
        unique_ptr<Expression> thenExpr;
        unique_ptr<Expression> elseExpr = nullptr;
        string class_name;
        If(){}
        If(Expression* _conditionExpr, Expression* _thenExpr, Expression* _elseExpr, Position p) : Expression(p), conditionExpr(_conditionExpr), thenExpr(_thenExpr), elseExpr(_elseExpr) {}
        If(Expression* _conditionExpr, Expression* _thenExpr, Position p) : Expression(p), conditionExpr(_conditionExpr), thenExpr(_thenExpr) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        string getNearestCommonParent(list<string> parents1, list<string> parents2);
        list<string> getInheritanceList(string class_name, map<string, unique_ptr<Class> *> & classesByName);
};

class IntegerExpression : public Expression {
    public:
        int value;
        string class_name;
        IntegerExpression(int val, Position p) : Expression(p), value(val) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
        void setExpressionClasses(string name) { class_name = name; }
        void set_scope_context(map<string, string> &identifiers) { /* empty */ }
        string getType(map<string, unique_ptr<Class> *> & classesByName) { return "int32"; }
        Value* codegen(LLVM& ll);
};

class StringLitExpression : public Expression {
    public:
        unique_ptr<string> value;
        string class_name;
        StringLitExpression(string* val, Position p) : Expression(p), value(val) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){ /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
        void setExpressionClasses(string name) { class_name = name; }
        void set_scope_context(map<string, string> &identifiers) { /* empty */ }
        string getType(map<string, unique_ptr<Class> *> & classesByName) { return "string"; }
};

class BooleanLitExpression : public Expression {
    public:
        unique_ptr<string> value;
        string class_name;
        BooleanLitExpression(string* val, Position p) : Expression(p), value(val) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){ /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
        string getType(map<string, unique_ptr<Class> *> & classesByName) { return "bool"; }
        void setExpressionClasses(string name) { class_name = name; }
        void set_scope_context(map<string, string> &identifiers) { /* empty */ }
};

class While : public Expression {
    public:
        unique_ptr<Expression> conditionExpr;
        unique_ptr<Expression> bodyExpr;
        string class_name;

        While(){}
        While(Expression* _conditionExpr, Expression* _bodyExpr, Position p) : Expression(p), conditionExpr(_conditionExpr), bodyExpr(_bodyExpr) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
};

class Assign : public Expression{
    public:
        unique_ptr<string> name;
        unique_ptr<Expression> expr;
        string class_name;
        map<string, string> scope_identifiers;

        Assign(){}
        Assign(string* _name, Expression* _expr, Position p) : Expression(p), name(_name), expr(_expr) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};

class BinOp : public Expression{
    public:
        unique_ptr<string> op;
        unique_ptr<Expression> left;
        unique_ptr<Expression> right;
        string class_name;
        BinOp(){}
        BinOp(string* _op, Expression* _left, Expression* _right, Position p) : Expression(p), op(_op), left(_left), right(_right) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};

class Args : public Expression {
    public:
        list<unique_ptr<Expression>> args;
        string class_name;
        Args(){}

        void addCallArgument(unique_ptr<Expression> arg);
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};


class Call : public Expression {
    public:
        unique_ptr<Expression> objExpr = nullptr;
        unique_ptr<string> methodName;
        unique_ptr<Args> args;
        string class_name;
        map<string, string> scope_context;

        Call(){}
        Call(Expression* _objExpr, string* _methodName, Args* _args, Position p) : Expression(p), objExpr(_objExpr), methodName(_methodName), args(_args) {}
        Call(string* _methodName, Args* _args, Position p) : Expression(p), methodName(_methodName), args(_args) {}
        
        llvm::Value* codegen(LLVM& ll, unique_ptr<Class> *> classesByName);
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        void checkMethodReturnTypeAndArgs(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void checkMethodFormals(unique_ptr<Class> & current_class, map<string, unique_ptr<Class> *> & classesByName, string methodName, unique_ptr<Args> & args);
        string getMethodType(unique_ptr<Class> & current_class, map<string, unique_ptr<Class> *> & classesByName, string methodName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};

class Let : public Expression {
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        unique_ptr<Expression> init = nullptr;
        unique_ptr<Expression> scope;
        string class_name;
        map<string, string> scope_identifiers;
        Let(){}
        Let(string* _name, string* _type, Expression* _init, Expression* _scope, Position p) : Expression(p), name(_name), type(_type), init(_init), scope(_scope) {}
        Let(string* _name, string* _type, Expression* _scope, Position p) : Expression(p), name(_name), type(_type), scope(_scope) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName);
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name);
        void set_scope_context(map<string, string> &identifiers);
};

class New : public Expression {
    public:
        unique_ptr<string> type;
        string class_name;
        New(){}
        New(string* _type, Position p) : Expression(p), type(_type) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers() { /* empty */ }
        string getType(map<string, unique_ptr<Class> *> & classesByName) { return *type; }
        void setExpressionClasses(string name) { class_name = name; }
        void set_scope_context(map<string, string> &identifiers) { /* empty */ }
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
};

class ObjectIdentifier : public Expression {
    public:
        unique_ptr<string> name;
        string class_name;
        map<string, string> scope_identifiers;

        ObjectIdentifier(){}
        ObjectIdentifier(string* _name, Position p) : Expression(p), name(_name) {}
        
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) { }
        void checkUndefinedIdentifiers();
        string getType(map<string, unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(string name) { class_name = name; }
        void set_scope_context(map<string, string> &identifiers) {
            scope_identifiers = identifiers;
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }

};

class UnitExpression : public Expression {
    public:
        string class_name;
        map<string, string> scope_identifiers;
        UnitExpression(Position p) : Expression(p) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName);
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            return "unit";
        }
        void setExpressionClasses(string name) { /* empty */ }
        void set_scope_context(map<string, string> &identifiers) {
            scope_identifiers = identifiers;
        }
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }
};
