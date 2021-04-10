#include <iostream>
#include <list> 
#include <string>
#include <iterator>
#include <memory>
using namespace std;

class Expression {
    public:
        Expression(){}
        virtual ~Expression(){}
        virtual string toString() = 0;
        virtual void checkTypes(map<string, string> class_names) = 0;
};

class UnaryOperator : public Expression{
    public:
        unique_ptr<string> op;
        unique_ptr<Expression> expr;
        UnaryOperator(){}
        UnaryOperator(string* _operator, Expression* _expr) : op(_operator), expr(_expr) {}
        string toString(){
            return "UnOp(" + *op + ", " + expr->toString() + ")";
        }
        void checkTypes(map<string, string> class_names){
            expr->checkTypes(class_names);
        }
};

class Block : public Expression{
    public:
        list<string> scope_identifiers;
        list<unique_ptr<Expression>> exprList;
        Block(){
        }

        void addExpression(unique_ptr<Expression> e) {
            exprList.push_front(move(e));
        }

        string toString() {
            string exprsToStr = "["; list<unique_ptr<Expression>>::iterator f_it;
            for (f_it = exprList.begin(); f_it != exprList.end(); f_it++) {
                exprsToStr += (*f_it)->toString() + ", ";
            }
            if (exprsToStr.length() > 1 && exprsToStr.substr(exprsToStr.length()-2) == ", ") {
                exprsToStr = exprsToStr.substr(0, exprsToStr.length()-2);
            }
            exprsToStr += "]";
            return exprsToStr;
        }

        void checkTypes(map<string, string> class_names){
            list<unique_ptr<Expression>>::iterator f_it;
            for (f_it = exprList.begin(); f_it != exprList.end(); f_it++) {
                (*f_it)->checkTypes(class_names);
            }
        }
};


class Formal {
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        Formal(){}
        Formal(string* _name, string* _type) : name(_name), type(_type) {}
        string toString(){
            return *name + " : " + *type;
        }

        void checkTypes(map<string, string> class_names) {
            map<string, string>::iterator a_pair;
            a_pair = class_names.find(*type);
            if (isupper((*type)[0]) && a_pair == class_names.end()) {
                // not found
                cout << "semantic error: type " << *type << " not defined" << endl;
            }
        }
};

class Formals {
    public:
        list<unique_ptr<Formal>> formals;
        Formals(){}
        string toString() {
            string formalsToStr = "["; list<unique_ptr<Formal>>::iterator f_it;
            for (f_it = formals.begin(); f_it != formals.end(); f_it++) {
                formalsToStr += (*f_it)->toString() + ", ";
            }
            if (formalsToStr.length() > 1 && formalsToStr.substr(formalsToStr.length()-2) == ", ") {
                formalsToStr = formalsToStr.substr(0, formalsToStr.length()-2);
            }
            formalsToStr += "]";
            return formalsToStr;
        }
        void addFormal(unique_ptr<Formal> f) {
            formals.push_front(move(f));
        }

        void checkTypes(map<string, string> class_names) {
            list<unique_ptr<Formal>>::iterator it;
            for (it = formals.begin(); it != formals.end(); it++) {
                // check formal types
               (*it)->checkTypes(class_names);
            }
        }

};

class Method {
    public:
        unique_ptr<string> name;
        unique_ptr<Formals> formals;
        unique_ptr<string> returnType;
        unique_ptr<Block> block;
        list<string> formal_names;
        Method(){}
        Method(string* _name, Formals* _formals, string* _returnType, Block* _block) : name(_name), formals(_formals), returnType(_returnType), block(_block) {}

        string toString() {
            return "Method(" + *name + ", " + formals->toString() + ", "  + *returnType + ", " + block->toString() + ")";
        }

        void checkFormalArguments(){
            list<unique_ptr<Formal>>::iterator it;
            for (it = formals->formals.begin(); it != formals->formals.end(); it++) {
                // check formal arguments redefinitions
               bool found = (find(formal_names.begin(), formal_names.end(), *((*it)->name)) != formal_names.end());
                if (!found) {
                    formal_names.push_back(*((*it)->name));
                } else {
                    cout << "semantic error: " + *((*it)->name) + " redefined";
                }
            }
        }

        void checkTypes(map<string, string> class_names) {
            map<string, string>::iterator a_pair;
            a_pair = class_names.find(*returnType);
            if (isupper((*returnType)[0]) && a_pair == class_names.end()) {
                // not found
                cout << "semantic error: type " << *returnType << " not defined" << endl;
            }
            formals->checkTypes(class_names);
            block->checkTypes(class_names);
        }

         void checkMain(){
             if(*returnType != "int32"){
                cout << "semantic error: Main method must return int32" << endl;
             }
            if(formals->formals.begin() != formals->formals.end()){
                cout << "semantic error: multiple parameters for main()" << endl;
            }
         }
};

class Methods {
    public:
        list<unique_ptr<Method>> methods;
        Methods() {
        }

        string toString() {
            string methodsToStr = "["; list<unique_ptr<Method>>::iterator m_it;
            for (m_it = methods.begin(); m_it != methods.end(); m_it++) {
                methodsToStr += (*m_it)->toString() + ", ";
            }
            if (methodsToStr.length() > 1 && methodsToStr.substr(methodsToStr.length()-2) == ", ") {
                methodsToStr = methodsToStr.substr(0, methodsToStr.length()-2);
            }
            methodsToStr += "]";
            return methodsToStr;
        }
        void addMethod(unique_ptr<Method> m) {
            methods.push_front(move(m));
        }

        void checkTypes(map<string, string> class_names) {
            list<unique_ptr<Method>>::iterator it2;
            for (it2 = methods.begin(); it2 != methods.end(); it2++) {
                // check types
                (*it2)->checkTypes(class_names);
            }
        }
};

class Field : public Expression{
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        unique_ptr<Expression> initExpr = nullptr;
        Field(){}
        Field(string* _name, string* _type, Expression* _initExpr) : name(_name), type(_type), initExpr(_initExpr) {}
        Field(string* _name, string* _type) : name(_name), type(_type) {}

        string toString() {
            string fieldStr = "Field(" + *name + ", " + *type;
            if (initExpr != nullptr) {
                fieldStr += ", " + initExpr->toString();
            }
            fieldStr += ")";
            return fieldStr;
        }

        void checkTypes(map<string, string> class_names) {
            map<string, string>::iterator a_pair;
            a_pair = class_names.find(*type);
            if (isupper((*type)[0]) && a_pair == class_names.end()) {
                // not found
                cout << "semantic error: type " << *type << " not defined" << endl;
            }
            if (initExpr != nullptr) {
                initExpr->checkTypes(class_names);
            }
        }
};

class Fields {
    public:
        list<unique_ptr<Field>> fields;
        Fields(){}

        string toString() {
            string fieldsToStr = "["; list<unique_ptr<Field>>::iterator f_it;
            for (f_it = fields.begin(); f_it != fields.end(); f_it++) {
                fieldsToStr += (*f_it)->toString() + ", ";
            }
            if (fieldsToStr.length() > 1 && fieldsToStr.substr(fieldsToStr.length()-2) == ", ") {
                fieldsToStr = fieldsToStr.substr(0, fieldsToStr.length()-2); 
            }
            fieldsToStr += "]";
            return fieldsToStr;
        }
        void addField(unique_ptr<Field> f) {
            fields.push_front(move(f));
        }

        void checkTypes(map<string, string> class_names) {
            list<unique_ptr<Field>>::iterator it2;
            for (it2 = fields.begin(); it2 != fields.end(); it2++) {
                // check types
                (*it2)->checkTypes(class_names);
            }
        }
};

class ClassBody {
    public:
        unique_ptr<Fields> fields;
        unique_ptr<Methods> methods;
        list<string> method_names;
        list<string> field_names;
        ClassBody() {
            unique_ptr<Fields> _fields(new Fields());
            fields = move(_fields);
            unique_ptr<Methods> _methods(new Methods());
            methods = move(_methods);
        }
        ClassBody(Fields* _fields, Methods* _methods) : fields(_fields), methods(_methods) {}

        void addField(unique_ptr<Field> field) {
            fields->addField(move(field));
        }
        void addMethod(unique_ptr<Method> method) {
            methods->addMethod(move(method));
        }

        void checkFieldsAndMethods(){
            list<unique_ptr<Method>>::iterator it;
            for (it = methods->methods.begin(); it != methods->methods.end(); it++) {
                // check method redefinitions
               bool found = (find(method_names.begin(), method_names.end(), *((*it)->name)) != method_names.end());
                if (!found) {
                    method_names.push_back(*((*it)->name));
                } else {
                    cout << "semantic error: " + *((*it)->name) + " redefined" << endl;
                }

                (*it)->checkFormalArguments();
            }

            list<unique_ptr<Field>>::iterator it2;
            for (it2 = fields->fields.begin(); it2 != fields->fields.end(); it2++) {
                // check field redefinitions
               bool found = (find(field_names.begin(), field_names.end(), *((*it2)->name)) != field_names.end());
                if (!found) {
                    field_names.push_back(*((*it2)->name));
                } else {
                    cout << "semantic error: " + *((*it2)->name) + " redefined";
                }
            }
        }

        void checkTypes(map<string, string> class_names) {
            methods->checkTypes(class_names);
            fields->checkTypes(class_names);
        }

        void checkMain(){
            list<unique_ptr<Method>>::iterator it;
            bool hasMainMethod = false;
            for (it = methods->methods.begin(); it!= methods->methods.end(); it++) {             
                if(*((*it)->name) == "main"){
                    hasMainMethod = true;
                    (*it)->checkMain();
                    break;
                }
            }
            if(hasMainMethod == false){
                cout << "semantic error: main method not defined in Main class" << endl;
            }
        }
};

class Class {
    public:
        unique_ptr<string> name;
        unique_ptr<string> parent;
        unique_ptr<ClassBody> classBody;
        Class(){}
        Class(string* _name, string* _parent, ClassBody* _classBody) : name(_name), parent(_parent), classBody(_classBody) {}

        string toString() {
            return "Class(" + *name + ", " + *parent + ", " + classBody->fields->toString() + ", " + classBody->methods->toString() + ")";
        }

        void checkFieldsAndMethods() {
            classBody->checkFieldsAndMethods();
        }

        void checkTypes(map<string, string> class_names){
            classBody->checkTypes(class_names);
        }

        void checkMain(){
            classBody->checkMain();
        }
};

class Program{
    public:
        map<string, string> inheritance = {
            {"Object", "Object"}
        };
        list<unique_ptr<Class>> classes;
        Program(list<unique_ptr<Class>> classList) {
            for(auto& c : classList) {
                classes.push_back(move(c));
            }
        }

        string toString() {
            string classesToStr = "["; list<unique_ptr<Class>>::iterator f_it;
            for (f_it = classes.begin(); f_it != classes.end(); f_it++) {
                classesToStr += (*f_it)->toString() + ", ";
            }
            if (classesToStr.length() > 1 && classesToStr.substr(classesToStr.length()-2) == ", ") {
                classesToStr = classesToStr.substr(0, classesToStr.length()-2);
            }
            classesToStr += "]";
            return classesToStr;
        }

        void checkSemantic(){
            checkClasses(move(classes));
        }

        void checkClasses(list<unique_ptr<Class>> classes) {
            list<unique_ptr<Class>>::iterator it;
            for (it = classes.begin(); it != classes.end(); it++) {
                // check classes redefinitions
                map<string, string>::iterator a_pair;
                a_pair = inheritance.find(*((*it)->name));
                if (a_pair == inheritance.end()) {
                    pair<string, string> p(*((*it)->name), *((*it)->parent));
                    inheritance.insert(p);
                } else {
                    cout << "semantic error: " + *((*it)->name) + " redefined" << endl;
                }
                
                // check Fields and Methods
                (*it)->checkFieldsAndMethods();

                // check identifiers
                // (*it)->checkIdentifiers();
            }
            // check inheritance cycles
            checkInheritanceCycles(classes);

            // check types
            checkTypes(classes, inheritance);

            //checkMain
            checkMain(classes, inheritance);
        }

        void checkMain(list<unique_ptr<Class>>& classes, map<string, string> class_names){
            list<unique_ptr<Class>>::iterator it;
            bool hasMainClass = false;
            for (it = classes.begin(); it!= classes.end(); it++) {             
                if(*((*it)->name) == "Main"){
                    hasMainClass = true;
                    (*it)->checkMain();
                    break;
                }
            }
            if(hasMainClass == false){
                cout << "semantic error: No main class" << endl;
            }
        }

        void checkTypes(list<unique_ptr<Class>>& classes, map<string, string> class_names) {
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                (*a_class)->checkTypes(class_names);
            }
        }

        void checkInheritanceCycles(list<unique_ptr<Class>>& classes) {
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                list<string> ancestors;
                string parentClass = hasInheritanceCycle(*((*a_class)->name), ancestors);
                if (parentClass.compare("") != 0) {
                    cout << "semantic error: class " << *((*a_class)->name) << " cannot extend child class " << parentClass << endl;
                }
            }
        }

        string hasInheritanceCycle(string class_name, list<string> ancestors) {
            map<string, string>::iterator a_pair;
            a_pair = inheritance.find(class_name);
            if (a_pair != inheritance.end()) {
                if (find(ancestors.begin(), ancestors.end(), a_pair->second) != ancestors.end()) {
                    return a_pair->second;
                } else if (a_pair->second.compare("Object") == 0) {
                    return "";
                }
                ancestors.push_back(a_pair->second);
                return hasInheritanceCycle(a_pair->second, ancestors);
            }
            return "";
        }

};

class If : public Expression {
    public:
        unique_ptr<Expression> conditionExpr;
        unique_ptr<Expression> thenExpr;
        unique_ptr<Expression> elseExpr = nullptr;
        If(){}
        If(Expression* _conditionExpr, Expression* _thenExpr, Expression* _elseExpr) : conditionExpr(_conditionExpr), thenExpr(_thenExpr), elseExpr(_elseExpr) {}

        If(Expression* _conditionExpr, Expression* _thenExpr) : conditionExpr(_conditionExpr), thenExpr(_thenExpr) {}

        string toString() {
            string content = "If(" + conditionExpr->toString() + ", " + thenExpr->toString();
            if (elseExpr != nullptr) {
                content += ", " + elseExpr->toString();
            }
            content += ")";
            return content;
        }

        void checkTypes(map<string, string> class_names){
            conditionExpr->checkTypes(class_names);
            thenExpr->checkTypes(class_names);
            if (elseExpr != nullptr) {
                elseExpr->checkTypes(class_names);
            }
        }
};

class IntegerExpression : public Expression {
    public:
        int value;
        IntegerExpression(int val) : value(val) {}

        string toString() {
            return to_string(value);
        }
        void checkTypes(map<string, string> class_names){

        }
};

class StringLitExpression : public Expression {
    public:
        unique_ptr<string> value;
        StringLitExpression(string* val) : value(val) {}

        string toString() {
            return *value;
        }
        void checkTypes(map<string, string> class_names){

        }
};

class While : public Expression {
    public:
        unique_ptr<Expression> conditionExpr;
        unique_ptr<Expression> bodyExpr;
        While(){}
        While(Expression* _conditionExpr, Expression* _bodyExpr) : conditionExpr(_conditionExpr), bodyExpr(_bodyExpr) {}

        string toString() {
            return "While(" + conditionExpr->toString() + ", " + bodyExpr->toString() + ")";
        }
        void checkTypes(map<string, string> class_names){
            conditionExpr->checkTypes(class_names);
            bodyExpr->checkTypes(class_names);
        }
};

class Assign : public Expression{
    public:
        unique_ptr<string> name;
        unique_ptr<Expression> expr;
        Assign(){}
        Assign(string* _name, Expression* _expr) : name(_name), expr(_expr) {}

        string toString() {
            return "Assign(" + *name + ", " + expr->toString() + ")";;
        }
        void checkTypes(map<string, string> class_names){
            expr->checkTypes(class_names);
        }
};

class BinaryOperator : public Expression{
    public:
        unique_ptr<string> op;
        unique_ptr<Expression> left;
        unique_ptr<Expression> right;
        BinaryOperator(){}
        BinaryOperator(string* _op, Expression* _left, Expression* _right) : op(_op), left(_left), right(_right) {}

        string toString() {
            return "BinOp(" + *op + ", " + left->toString() + ", " + right->toString() + ")";
        }
        void checkTypes(map<string, string> class_names){
            left->checkTypes(class_names);
            right->checkTypes(class_names);
        }
};


class Call : public Expression{
    public:
        unique_ptr<Expression> objExpr = nullptr;
        unique_ptr<string> methodName;
        unique_ptr<Block> args;
        Call(){}
        Call(Expression* _objExpr, string* _methodName, Block* _args) : objExpr(_objExpr), methodName(_methodName), args(_args) {}
        Call(string* _methodName, Block* _args) : methodName(_methodName), args(_args) {}

        string toString() {
            string obj = "self";
            if (objExpr != nullptr) {
                obj = objExpr->toString();
            }
            return "Call(" + obj + ", " + *methodName + ", " + args->toString() + ")";
        }

        void checkTypes(map<string, string> class_names) {
            if (objExpr != nullptr) {
                objExpr->checkTypes(class_names);
            }
            args->checkTypes(class_names);
        }
};

class Let : public Expression{
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        unique_ptr<Expression> init = nullptr;
        unique_ptr<Expression> scope;
        Let(){}
        Let(string* _name, string* _type, Expression* _init, Expression* _scope) : name(_name), type(_type), init(_init), scope(_scope) {}
        Let(string* _name, string* _type, Expression* _scope) : name(_name), type(_type), scope(_scope) {}

        string toString() {
            string firstPart = "Let(" + *name + ", " + *type + ", ";
            string lastPart = "";
            if (init != nullptr) {
                lastPart += init->toString() + ", ";
            }
            lastPart += scope->toString() + ")";
            return firstPart + lastPart;   
        }

        void checkTypes(map<string, string> class_names) {
            map<string, string>::iterator a_pair;
            a_pair = class_names.find(*type);
            if (isupper((*type)[0]) && a_pair == class_names.end()) {
                // not found
                cout << "semantic error: type " << *type << " not defined" << endl;
            }
            if (init != nullptr) {
                init->checkTypes(class_names);
            }
            scope->checkTypes(class_names);
        }
};

class New : public Expression {
    public:
        unique_ptr<string> type;
        New(){}
        New(string* _type) : type(_type) {}
        string toString() {
            return "New(" + *type + ")";
        }

        void checkTypes(map<string, string> class_names) {
            map<string, string>::iterator a_pair;
            a_pair = class_names.find(*type);
            if (isupper((*type)[0]) && a_pair == class_names.end()) {
                // not found
                cout << "semantic error: type " << *type << " not defined" << endl;
            }
        }
};
