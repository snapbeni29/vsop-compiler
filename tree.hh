#include <iostream>
#include <list> 
#include <string>
#include <iterator>
#include <memory>
#include "llvm.hh"

class Class;

class Utils {
    public:
        static bool inheritsFrom(std::string child_class, std::string parent_class, std::map<std::string, std::unique_ptr<Class> *> & classesByName);
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
        void printError(std::string s, Position p);
};

class Expression : public TreeNode {
    public:
        Expression(Position p) : TreeNode(p) {}
        Expression() {}
        virtual ~Expression(){}
        llvm::Value* llvm_value;
        virtual std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName) = 0;
        virtual void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName) = 0;
        virtual void checkUndefinedIdentifiers() = 0;
        virtual void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) = 0;
        virtual std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName) = 0;
        virtual void setExpressionClasses(std::string name) = 0;
        virtual void set_scope_context(std::map<std::string, std::string> &identifiers) = 0;
        virtual llvm::Value* codegen(LLVM& ll) {};
        llvm::Value* getValue(LLVM& ll) {
            if (!llvm_value) {
                llvm_value = codegen(ll);
            }
            return llvm_value;
        }
        bool isPrimitive(std::string type);
};

class UnOp : public Expression {
    public:
        std::unique_ptr<std::string> op;
        std::unique_ptr<Expression> expr;
        std::string class_name;
        UnOp(){}
        UnOp(std::string* _operator, Expression* _expr, Position p) : Expression(p), op(_operator), expr(_expr) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class Block : public Expression {
    public:
        std::list<std::unique_ptr<Expression>> exprList;
        std::string class_name;
        Block(){
        }

        void addExpression(std::unique_ptr<Expression> e);
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};


class Formal : public TreeNode {
    public:
        std::unique_ptr<std::string> name;
        std::unique_ptr<std::string> type;
        std::string class_name;
        Formal(){}
        Formal(std::string* _name, std::string* _type, Position p) : TreeNode(p), name(_name), type(_type) {}

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setClass(std::string name);
        llvm::Value* getValue(LLVM& ll) ;
};

class Formals {
    public:
        std::list<std::unique_ptr<Formal>> formals;
        Formals(){}

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void addFormal(std::unique_ptr<Formal> f);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        llvm::Value* getValue(LLVM& ll) ;
};

class Method : public TreeNode {
    public:
        std::unique_ptr<std::string> name;
        std::unique_ptr<Formals> formals;
        std::unique_ptr<std::string> returnType;
        std::unique_ptr<Block> block;
        std::list<std::string> formal_names;
        std::string class_name;
        std::map<std::string, std::string> scope_context;
        Method(){}
        Method(std::string* _name, Formals* _formals, std::string* _returnType, Block* _block, Position p) : TreeNode(p), name(_name), formals(_formals), returnType(_returnType), block(_block) {}

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkFormalArgumentsRedefinition();
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> & identifiers);
        void checkMain();
        llvm::Value* getValue(LLVM& ll) ;

};

class Methods {
    public:
        std::list<std::unique_ptr<Method>> methods;
        Methods() {
        }

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void addMethod(std::unique_ptr<Method> m);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> & identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class Field : public Expression {
    public:
        std::unique_ptr<std::string> name;
        std::unique_ptr<std::string> type;
        std::unique_ptr<Expression> initExpr = nullptr;
        std::string class_name;
        std::map<std::string, std::string> scope_identifiers;
        Field(){}
        Field(std::string* _name, std::string* _type, Expression* _initExpr, Position p) : Expression(p), name(_name), type(_type), initExpr(_initExpr) {}
        Field(std::string* _name, std::string* _type, Position p) : Expression(p), name(_name), type(_type) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class Fields {
    public:
        std::list<std::unique_ptr<Field>> fields;
        Fields(){}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void addField(std::unique_ptr<Field> f);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void checkUndefinedIdentifiers();
        void set_scope_context(std::map<std::string, std::string> & identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class ClassBody : public TreeNode {
    public:
        std::unique_ptr<Fields> fields;
        std::unique_ptr<Methods> methods;
        std::map<std::string, std::string> method_types;
        std::map<std::string, std::string> field_types;
        std::string class_name;

        ClassBody() {
            std::unique_ptr<Fields> _fields(new Fields());
            fields = move(_fields);
            std::unique_ptr<Methods> _methods(new Methods());
            methods = move(_methods);
        }
        ClassBody(Fields* _fields, Methods* _methods) : fields(_fields), methods(_methods) {}
        
        llvm::Value* codegen(LLVM& ll);
        void addField(std::unique_ptr<Field> field);
        void addMethod(std::unique_ptr<Method> method);
        void checkMethodsRedefinitions();
        std::map<std::string, std::string> getMethodsAndTypes();
        // set the map of all known identifiers and their types
        void set_scope_context(std::map<std::string, std::string> & identifiers);
        std::map<std::string, std::string> getFieldsAndTypes();
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void checkMain();
        llvm::Value* getValue(LLVM& ll) ;

};

class Class : public TreeNode {
    public:
        std::unique_ptr<std::string> name;
        std::unique_ptr<std::string> parent;
        std::unique_ptr<ClassBody> classBody;
        Class(Position p) : TreeNode(p) {}
        Class(std::string* _name, std::string* _parent, ClassBody* _classBody, Position p) : TreeNode(p), name(_name), parent(_parent), classBody(_classBody) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        llvm::Value* codegen(LLVM& ll);
        void checkFieldsRedefinitions(std::map<std::string, std::unique_ptr<Class>*> classesByName);
        int countFieldDefinitions(std::list<std::unique_ptr<Field>> & field_list, std::string field_name);
        std::string isFieldAlreadyDefined(std::map<std::string, std::unique_ptr<Class>*> classesByName, std::string currentClass, std::string field_name);
        void checkMethodsRedefinitions();
        std::map<std::string, std::string> getFieldsAndTypes();
        std::map<std::string, std::string> getMethodsAndTypes();
        void checkUndefinedIdentifiers();
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void checkMain();
        // set the map of all known identifiers and their types
        void set_scope_context(std::map<std::string, std::unique_ptr<Class>*> & classesByName);
        void includeParentFields(std::map<std::string, std::unique_ptr<Class> *> & classesByName, std::string parent_name, std::list<std::string> seen_classes, std::map<std::string, std::string> & identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class Program : public TreeNode {
    public:
        Class* obj = createObjectClass();
        std::unique_ptr<Class> object_class = std::unique_ptr<Class>(obj);
        std::map<std::string, std::unique_ptr<Class>*> classesByName;
        std::list<std::unique_ptr<Class>> classes;
        Program(std::list<std::unique_ptr<Class>> classList) {
            classes.push_back(move(object_class));
            for(auto& c : classList) {
                classes.push_back(move(c));
            }
        }

        llvm::Value* codegen(LLVM& ll);
        Class* createObjectClass();
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkSemantic();
        void set_scope_context();
        void checkCallsToUndefinedMethods();
        void setExpressionClasses();
        void checkMain(std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkClasses();
        void checkUndefinedIdentifiers();
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkParentMethods(std::unique_ptr<Class>& childClass, std::unique_ptr<Class>& parentClass);
        bool isMethodInClass(std::unique_ptr<Method> & method, std::unique_ptr<Class> & class_ptr);
        void checkInheritanceCycles(std::list<std::unique_ptr<Class>>& classes);
        std::string hasInheritanceCycle(std::string class_name, std::list<std::string> ancestors);
        llvm::Value* getValue(LLVM& ll) ;
};

class If : public Expression {
    public:
        std::unique_ptr<Expression> conditionExpr;
        std::unique_ptr<Expression> thenExpr;
        std::unique_ptr<Expression> elseExpr = nullptr;
        std::string class_name;
        If(){}
        If(Expression* _conditionExpr, Expression* _thenExpr, Expression* _elseExpr, Position p) : Expression(p), conditionExpr(_conditionExpr), thenExpr(_thenExpr), elseExpr(_elseExpr) {}
        If(Expression* _conditionExpr, Expression* _thenExpr, Position p) : Expression(p), conditionExpr(_conditionExpr), thenExpr(_thenExpr) {}

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getNearestCommonParent(std::list<std::string> parents1, std::list<std::string> parents2);
        std::list<std::string> getInheritanceList(std::string class_name, std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        llvm::Value* getValue(LLVM& ll) ;
};

class IntegerExpression : public Expression {
    public:
        int value;
        std::string class_name;
        IntegerExpression(int val, Position p) : Expression(p), value(val) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        void setExpressionClasses(std::string name) { class_name = name; }
        void set_scope_context(std::map<std::string, std::string> &identifiers) { /* empty */ }
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { return "int32"; }
        Value* codegen(LLVM& ll);
        llvm::Value* getValue(LLVM& ll) ;
};

class StringLitExpression : public Expression {
    public:
        std::unique_ptr<std::string> value;
        std::string class_name;
        StringLitExpression(std::string* val, Position p) : Expression(p), value(val) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName){ /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        void setExpressionClasses(std::string name) { class_name = name; }
        void set_scope_context(std::map<std::string, std::string> &identifiers) { /* empty */ }
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { return "std::string"; }
        llvm::Value* getValue(LLVM& ll) ;
};

class BooleanLitExpression : public Expression {
    public:
        std::unique_ptr<std::string> value;
        std::string class_name;
        BooleanLitExpression(std::string* val, Position p) : Expression(p), value(val) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName){ /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { return "bool"; }
        void setExpressionClasses(std::string name) { class_name = name; }
        void set_scope_context(std::map<std::string, std::string> &identifiers) { /* empty */ }
        llvm::Value* getValue(LLVM& ll) ;
};

class While : public Expression {
    public:
        std::unique_ptr<Expression> conditionExpr;
        std::unique_ptr<Expression> bodyExpr;
        std::string class_name;

        While(){}
        While(Expression* _conditionExpr, Expression* _bodyExpr, Position p) : Expression(p), conditionExpr(_conditionExpr), bodyExpr(_bodyExpr) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        llvm::Value* getValue(LLVM& ll) ;
};

class Assign : public Expression{
    public:
        std::unique_ptr<std::string> name;
        std::unique_ptr<Expression> expr;
        std::string class_name;
        std::map<std::string, std::string> scope_identifiers;

        Assign(){}
        Assign(std::string* _name, Expression* _expr, Position p) : Expression(p), name(_name), expr(_expr) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class BinOp : public Expression{
    public:
        std::unique_ptr<std::string> op;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
        std::string class_name;
        BinOp(){}
        BinOp(std::string* _op, Expression* _left, Expression* _right, Position p) : Expression(p), op(_op), left(_left), right(_right) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class Args : public Expression {
    public:
        std::list<std::unique_ptr<Expression>> args;
        std::string class_name;
        Args(){}

        void addCallArgument(std::unique_ptr<Expression> arg);
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};


class Call : public Expression {
    public:
        std::unique_ptr<Expression> objExpr = nullptr;
        std::unique_ptr<std::string> methodName;
        std::unique_ptr<Args> args;
        std::string class_name;
        std::map<std::string, std::string> scope_context;

        Call(){}
        Call(Expression* _objExpr, std::string* _methodName, Args* _args, Position p) : Expression(p), objExpr(_objExpr), methodName(_methodName), args(_args) {}
        Call(std::string* _methodName, Args* _args, Position p) : Expression(p), methodName(_methodName), args(_args) {}
        
        llvm::Value* codegen(LLVM& ll, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkMethodReturnTypeAndArgs(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkMethodFormals(std::unique_ptr<Class> & current_class, std::map<std::string, std::unique_ptr<Class> *> & classesByName, std::string methodName, std::unique_ptr<Args> & args);
        std::string getMethodType(std::unique_ptr<Class> & current_class, std::map<std::string, std::unique_ptr<Class> *> & classesByName, std::string methodName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class Let : public Expression {
    public:
        std::unique_ptr<std::string> name;
        std::unique_ptr<std::string> type;
        std::unique_ptr<Expression> init = nullptr;
        std::unique_ptr<Expression> scope;
        std::string class_name;
        std::map<std::string, std::string> scope_identifiers;
        Let(){}
        Let(std::string* _name, std::string* _type, Expression* _init, Expression* _scope, Position p) : Expression(p), name(_name), type(_type), init(_init), scope(_scope) {}
        Let(std::string* _name, std::string* _type, Expression* _scope, Position p) : Expression(p), name(_name), type(_type), scope(_scope) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers();
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name);
        void set_scope_context(std::map<std::string, std::string> &identifiers);
        llvm::Value* getValue(LLVM& ll) ;
};

class New : public Expression {
    public:
        std::unique_ptr<std::string> type;
        std::string class_name;
        New(){}
        New(std::string* _type, Position p) : Expression(p), type(_type) {}

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void checkUndefinedIdentifiers() { /* empty */ }
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { return *type; }
        void setExpressionClasses(std::string name) { class_name = name; }
        void set_scope_context(std::map<std::string, std::string> &identifiers) { /* empty */ }
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        llvm::Value* getValue(LLVM& ll) ;
};

class ObjectIdentifier : public Expression {
    public:
        std::unique_ptr<std::string> name;
        std::string class_name;
        std::map<std::string, std::string> scope_identifiers;

        ObjectIdentifier(){}
        ObjectIdentifier(std::string* _name, Position p) : Expression(p), name(_name) {}
        
        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { }
        void checkUndefinedIdentifiers();
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName);
        void setExpressionClasses(std::string name) { class_name = name; }
        void set_scope_context(std::map<std::string, std::string> &identifiers) {
            scope_identifiers = identifiers;
        }

        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        llvm::Value* getValue(LLVM& ll) ;
};

class UnitExpression : public Expression {
    public:
        std::string class_name;
        std::map<std::string, std::string> scope_identifiers;
        UnitExpression(Position p) : Expression(p) {}

        std::string toString(bool c, std::map<std::string, std::unique_ptr<Class> *> classesByName);
        void checkTypes(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        void checkUndefinedIdentifiers() { /* empty */ }
        std::string getType(std::map<std::string, std::unique_ptr<Class> *> & classesByName) {
            return "unit";
        }
        void setExpressionClasses(std::string name) { /* empty */ }
        void set_scope_context(std::map<std::string, std::string> &identifiers) {
            scope_identifiers = identifiers;
        }
        void checkCallsToUndefinedMethods(std::map<std::string, std::unique_ptr<Class> *> & classesByName) { /* empty */ }
        llvm::Value* getValue(LLVM& ll);
};
