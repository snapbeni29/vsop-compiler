#include <iostream>
#include <list> 
#include <string>
#include <iterator>
#include <memory>
using namespace std;

extern string filename;
extern int errors;

class Class;

struct Position {
    int row;
    int col;
};

class TreeNode {
    public: 
        TreeNode(Position p) : position(p) {}
        TreeNode() {}
        Position position;
        void printError(string s, Position p) {
            cerr << filename << ":" << p.row << ":" <<  p.col << ": semantic error: " << s << endl;
            errors++;
        }
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
};

class UnaryOperator : public Expression {
    public:
        unique_ptr<string> op;
        unique_ptr<Expression> expr;
        string class_name;
        UnaryOperator(){}
        UnaryOperator(string* _operator, Expression* _expr, Position p) : Expression(p), op(_operator), expr(_expr) {}
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName){
            string content = "";
            content += "UnOp(" + *op + ", " + expr->toString(c, classesByName) + ")";
            if (c) 
                content += " : " + getType(classesByName);
            return content;
        }
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            expr->checkTypes(classesByName);
        }

        void checkUndefinedIdentifiers(){
            expr->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            expr->checkCallsToUndefinedMethods(classesByName);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            if ((*op).compare("isnull") == 0) {
                return "bool";
            }
            return expr->getType(classesByName);
        }

        void setExpressionClasses(string name) {
            class_name = name;
            expr->setExpressionClasses(name);
        }

        void set_scope_context(map<string, string> &identifiers) {
            expr->set_scope_context(identifiers);
        }

};

class Block : public Expression {
    public:
        list<unique_ptr<Expression>> exprList;
        string class_name;
        Block(){
        }

        void addExpression(unique_ptr<Expression> e) {
            exprList.push_front(move(e));
            position = exprList.back()->position;
        }

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string exprsToStr = "["; list<unique_ptr<Expression>>::iterator f_it;
            for (f_it = exprList.begin(); f_it != exprList.end(); f_it++) {           
                 exprsToStr += (*f_it)->toString(c, classesByName) + ", ";
            }
            if (exprsToStr.length() > 1 && exprsToStr.substr(exprsToStr.length()-2) == ", ") {
                exprsToStr = exprsToStr.substr(0, exprsToStr.length()-2);
            }
            exprsToStr += "]";
            return exprsToStr;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            list<unique_ptr<Expression>>::iterator f_it;
            for (f_it = exprList.begin(); f_it != exprList.end(); f_it++) {
                (*f_it)->checkTypes(classesByName);
            }
        }

        void checkUndefinedIdentifiers(){
            list<unique_ptr<Expression>>::iterator f_it;
            for (f_it = exprList.begin(); f_it != exprList.end(); f_it++) {
                (*f_it)->checkUndefinedIdentifiers();
            }
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            list<unique_ptr<Expression>>::iterator ex;
            for (ex = exprList.begin(); ex != exprList.end(); ex++) {
                (*ex)->checkCallsToUndefinedMethods(classesByName);
            }
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            return exprList.back()->getType(classesByName);
        }

        void setExpressionClasses(string name){
            class_name = name;
            list<unique_ptr<Expression>>::iterator ex;
            for (ex = exprList.begin(); ex != exprList.end(); ex++) {
                (*ex)->setExpressionClasses(name);
            }
        }

        void set_scope_context(map<string, string> &identifiers) {
            list<unique_ptr<Expression>>::iterator ex;
            for (ex = exprList.begin(); ex != exprList.end(); ex++) {
                (*ex)->set_scope_context(identifiers);
            }
        }
};


class Formal : public TreeNode {
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        string class_name;
        Formal(){}
        Formal(string* _name, string* _type, Position p) : TreeNode(p), name(_name), type(_type) {}
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName){
            return *name + " : " + *type;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator a_pair;
            a_pair = classesByName.find(*type);
            if (isupper((*type)[0]) && a_pair == classesByName.end()) {
                // not found
                printError("type" + *type + " not defined", position);
            }
        }

        void setClass(string name){
            class_name = name;
        }
};

class Formals {
    public:
        list<unique_ptr<Formal>> formals;
        Formals(){}
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string formalsToStr = "["; list<unique_ptr<Formal>>::iterator f_it;
            for (f_it = formals.begin(); f_it != formals.end(); f_it++) {
                formalsToStr += (*f_it)->toString(c, classesByName) + ", ";
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

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            list<unique_ptr<Formal>>::iterator it;
            for (it = formals.begin(); it != formals.end(); it++) {
                // check formal types
               (*it)->checkTypes(classesByName);
            }
        }

        void setExpressionClasses(string name){
            list<unique_ptr<Formal>>::iterator formal;
            for (formal = formals.begin(); formal != formals.end(); formal++) {
                (*formal)->setClass(name);
            }
        }

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

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            if(c)
                return "Method(" + *name + ", " + formals->toString(c, classesByName) + ", "  + *returnType + ", " + block->toString(c, classesByName) + " : " + block->getType(classesByName) + ")";
            else
                return "Method(" + *name + ", " + formals->toString(c, classesByName) + ", "  + *returnType + ", " + block->toString(c, classesByName) + ")";
        }

        void checkFormalArguments(){
            list<unique_ptr<Formal>>::iterator it;
            for (it = formals->formals.begin(); it != formals->formals.end(); it++) {
                // check formal arguments redefinitions
                bool found = (find(formal_names.begin(), formal_names.end(), *((*it)->name)) != formal_names.end());
                if (!found) {
                    formal_names.push_back(*((*it)->name));
                } else {
                    printError("formal argument " + *((*it)->name) + " redefined", (*it)->position);
                }
            }
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator a_pair;
            a_pair = classesByName.find(*returnType);
            if (isupper((*returnType)[0]) && a_pair == classesByName.end()) {
                // not found
                printError("type " + *returnType + " not defined", position);
            }
            formals->checkTypes(classesByName);
            block->checkTypes(classesByName);
            // verify if block type matches returnType
            if (block->exprList.size() == 0) return;
            string blockType = block->getType(classesByName);
            if (blockType != *returnType) {
                printError("block of method " + *name + " in class " + class_name + " should return a value of type " + *returnType + " but returns a value of type " + blockType, block->exprList.back()->position);
            }
        }

        void checkUndefinedIdentifiers(){
            block->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            block->checkCallsToUndefinedMethods(classesByName);
        }

        void setExpressionClasses(string name){
            class_name = name;
            block->setExpressionClasses(name);
            formals->setExpressionClasses(name);
        }

        void set_scope_context(map<string, string> & identifiers) {
            scope_context = identifiers;
            list<unique_ptr<Formal>>::iterator formal;
            for (formal = formals->formals.begin(); formal != formals->formals.end(); formal++) {
                pair<string, string> p(*((*formal)->name), *((*formal)->type));
                map<string, string>::iterator it = scope_context.find(*((*formal)->name)); 
                if (it != scope_context.end())
                    it->second = *((*formal)->type);
                else
                    scope_context.insert(p);
            }
            block->set_scope_context(scope_context);
        }

        void checkMain() {
            if(*returnType != "int32"){
                printError("main method must return int32", position);
            }
            if(formals->formals.begin() != formals->formals.end()){
                printError("multiple parameters for main()", position);
            }
        }

};

class Methods {
    public:
        list<unique_ptr<Method>> methods;
        Methods() {
        }

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string methodsToStr = "["; list<unique_ptr<Method>>::iterator m_it;
            for (m_it = methods.begin(); m_it != methods.end(); m_it++) {
                methodsToStr += (*m_it)->toString(c, classesByName) + ", ";
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

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            list<unique_ptr<Method>>::iterator it2;
            for (it2 = methods.begin(); it2 != methods.end(); it2++) {
                // check types
                (*it2)->checkTypes(classesByName);
            }
        }
        void checkUndefinedIdentifiers(){
            list<unique_ptr<Method>>::iterator it2;
            for (it2 = methods.begin(); it2 != methods.end(); it2++) {
                (*it2)->checkUndefinedIdentifiers();
            }
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            list<unique_ptr<Method>>::iterator method;
            for (method = methods.begin(); method != methods.end(); method++) {
                (*method)->checkCallsToUndefinedMethods(classesByName);
            }
        }

        void setExpressionClasses(string name){
            list<unique_ptr<Method>>::iterator a_method;
            for (a_method = methods.begin(); a_method != methods.end(); a_method++) {
                (*a_method)->setExpressionClasses(name);
            }
        }

        void set_scope_context(map<string, string> & identifiers) {
            list<unique_ptr<Method>>::iterator a_method;
            for (a_method = methods.begin(); a_method != methods.end(); a_method++) {
                (*a_method)->set_scope_context(identifiers);
            }
        }
};

class Field : public Expression {
    public:
        unique_ptr<string> name;
        unique_ptr<string> type;
        unique_ptr<Expression> initExpr = nullptr;
        string class_name;
        Field(){}
        Field(string* _name, string* _type, Expression* _initExpr, Position p) : Expression(p), name(_name), type(_type), initExpr(_initExpr) {}
        Field(string* _name, string* _type, Position p) : Expression(p), name(_name), type(_type) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string fieldStr = "Field(" + *name + ", " + *type;
            if (initExpr != nullptr) {
                fieldStr += ", " + initExpr->toString(c, classesByName);
            }
            fieldStr += ")";
            return fieldStr;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator a_pair;
            a_pair = classesByName.find(*type);
            if (isupper((*type)[0]) && a_pair == classesByName.end()) {
                // not found
                printError("type " + *type + " not defined", position);
            }
            if (initExpr != nullptr) {
                initExpr->checkTypes(classesByName);
            }
        }
        void checkUndefinedIdentifiers() {
            // check in the expression
            if (initExpr != nullptr) {
                initExpr->checkUndefinedIdentifiers();
            }
        }
        
        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }

        void setExpressionClasses(string name){
            class_name = name;
            if (initExpr != nullptr) {
                initExpr->setExpressionClasses(name);
            }
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            return *type;
        }

        void set_scope_context(map<string, string> &identifiers) { 
            pair<string, string> p(*name, *type);
            identifiers.insert(p);
            if (initExpr != nullptr)
                initExpr->set_scope_context(identifiers);
        }   

};

class Fields {
    public:
        list<unique_ptr<Field>> fields;
        Fields(){}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string fieldsToStr = "["; list<unique_ptr<Field>>::iterator f_it;
            for (f_it = fields.begin(); f_it != fields.end(); f_it++) {
                fieldsToStr += (*f_it)->toString(c, classesByName) + ", ";
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

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            list<unique_ptr<Field>>::iterator it2;
            for (it2 = fields.begin(); it2 != fields.end(); it2++) {
                // check types
                (*it2)->checkTypes(classesByName);
            }
        }

        void setExpressionClasses(string name){
            list<unique_ptr<Field>>::iterator a_field;
            for (a_field = fields.begin(); a_field != fields.end(); a_field++) {
                (*a_field)->setExpressionClasses(name);
            }
        }
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

        void addField(unique_ptr<Field> field) {
            fields->addField(move(field));
        }
        void addMethod(unique_ptr<Method> method) {
            methods->addMethod(move(method));
        }

        void checkMethodsRedefinitions() {
            list<unique_ptr<Method>>::iterator it;
            for (it = methods->methods.begin(); it != methods->methods.end(); it++) {
                // check method redefinitions
               bool found = (method_types.find(*((*it)->name)) != method_types.end());
                if (!found) {
                    pair<string, string> p(*((*it)->name), *((*it)->returnType));
                    method_types.insert(p);
                } else {
                    printError("method " + *((*it)->name) + " redefined", (*it)->position);
                }
                (*it)->checkFormalArguments();
            }
        }

        map<string, string> getMethodsAndTypes() {
            return method_types;
        }

        // set the map of all known identifiers and their types
        void set_scope_context(map<string, string> & identifiers) {
            methods->set_scope_context(identifiers);
        }

        map<string, string> getFieldsAndTypes() {
            return field_types;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            methods->checkTypes(classesByName);
            fields->checkTypes(classesByName);
        }

        void checkUndefinedIdentifiers() {
            methods->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            methods->checkCallsToUndefinedMethods(classesByName);
        }

        void setExpressionClasses(string name){
            class_name = name;
            methods->setExpressionClasses(name);
            fields->setExpressionClasses(name);
        }

        void checkMain(){
            list<unique_ptr<Method>>::iterator it;
            bool hasMainMethod = false;
            for (it = methods->methods.begin(); it != methods->methods.end(); it++) {             
                if(*((*it)->name) == "main"){
                    hasMainMethod = true;
                    (*it)->checkMain();
                    break;
                }
            }
            if (hasMainMethod == false){
                printError("main method of class Main is not defined", {1, 1});
            }
        }

};

class Class : public TreeNode {
    public:
        unique_ptr<string> name;
        unique_ptr<string> parent;
        unique_ptr<ClassBody> classBody;
        Class(Position p) : TreeNode(p) {}
        Class(string* _name, string* _parent, ClassBody* _classBody, Position p) : TreeNode(p), name(_name), parent(_parent), classBody(_classBody) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            return "Class(" + *name + ", " + *parent + ", " + classBody->fields->toString(c, classesByName) + ", " + classBody->methods->toString(c, classesByName) + ")";
        }

        void checkFieldsRedefinitions(map<string, unique_ptr<Class>*> classesByName) {
            list<unique_ptr<Field>>::iterator it2;
            for (it2 = classBody->fields->fields.begin(); it2 != classBody->fields->fields.end(); it2++) {
                // check field redefinitions
                int currentClassDefinitions = countFieldDefinitions(classBody->fields->fields, *((*it2)->name));
                if (currentClassDefinitions > 1)
                    printError("field " + *((*it2)->name) + " of class " + *name + " is already defined in the class", (*it2)->position);
                
                if (parent == nullptr) return;
                string classContainingField = isFieldAlreadyDefined(classesByName, *parent, *((*it2)->name));
                if (classContainingField != "") {
                    printError("field " + *((*it2)->name) + " of class " + *name + " is already defined in class " + classContainingField, (*it2)->position);
                } else {
                    pair<string, string> p(*((*it2)->name), *((*it2)->type));
                    classBody->field_types.insert(p);
                }
            }
        }

        int countFieldDefinitions(list<unique_ptr<Field>> & field_list, string field_name) {
            list<unique_ptr<Field>>::iterator it_fields;
            int counter = 0;
            for (it_fields = field_list.begin(); it_fields != field_list.end(); it_fields++) {
                if (*((*it_fields)->name) == field_name) {
                    counter += 1;
                }
            }
            return counter;
        }

        string isFieldAlreadyDefined(map<string, unique_ptr<Class>*> classesByName, string currentClass, string field_name) {
            // find the class by name
            map<string, unique_ptr<Class> *>::iterator it_classes = classesByName.find(currentClass);
            if (it_classes != classesByName.end()) {
                unique_ptr<Class> * class_ptr = it_classes->second;
                list<unique_ptr<Field>>::iterator it_fields;
                if (countFieldDefinitions((*class_ptr)->classBody->fields->fields, field_name) > 0)
                    return currentClass;
                if ((*class_ptr)->parent != nullptr) {
                    return isFieldAlreadyDefined(classesByName, *((*class_ptr)->parent), field_name);
                }
            }
            return "";
        }

        void checkMethodsRedefinitions() {
            classBody->checkMethodsRedefinitions();
        }

        map<string, string> getFieldsAndTypes() {
            return classBody->getFieldsAndTypes();
        }

        map<string, string> getMethodsAndTypes() {
            return classBody->getMethodsAndTypes();
        }

        void checkUndefinedIdentifiers() {
            classBody->checkUndefinedIdentifiers();
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            classBody->checkTypes(classesByName);
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            classBody->checkCallsToUndefinedMethods(classesByName);
        }

        void setExpressionClasses(string name){
            classBody->setExpressionClasses(name);
        }

        void checkMain(){
            classBody->checkMain();
        }

        // set the map of all known identifiers and their types
        void set_scope_context(map<string, unique_ptr<Class>*> & classesByName) {
            map<string, string> identifiers(classBody->field_types);
            identifiers.insert(make_pair("self", *name));
            if (parent != nullptr)
                includeParentFields(classesByName, *parent, identifiers);
            classBody->set_scope_context(identifiers);
        }

        void includeParentFields(map<string, unique_ptr<Class> *> & classesByName, string parent_name, map<string, string> & identifiers) {
            map<string, unique_ptr<Class> *>::iterator class_it = classesByName.find(parent_name);
            list<unique_ptr<Field>>::iterator fields_it;
            for (fields_it = (*(class_it->second))->classBody->fields->fields.begin(); fields_it != (*(class_it->second))->classBody->fields->fields.end(); fields_it++) {
                identifiers.insert(make_pair(*((*fields_it)->name), *((*fields_it)->type)));
            }
            if ((*(class_it->second))->parent == nullptr) return;
            includeParentFields(classesByName, *((*(class_it->second))->parent), identifiers);
        }
        
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

        Class* createObjectClass() {
            ClassBody* cb = new ClassBody();
            Formal* s = new Formal(new string("s"), new string("string"), {1, 1});
            Formals* print_formals = new Formals();
            print_formals->addFormal(unique_ptr<Formal>(s));
            Method* _print = new Method(new string("print"), print_formals, new string("Object"), new Block(), {1, 1});
            cb->addMethod(unique_ptr<Method>(_print));

            Formal* b = new Formal(new string("b"), new string("bool"), {1, 1});
            Formals* printBool_formals = new Formals();
            printBool_formals->addFormal(unique_ptr<Formal>(b));
            Method* _printBool = new Method(new string("printBool"), printBool_formals, new string("Object"), new Block(), {1, 1});
            cb->addMethod(unique_ptr<Method>(_printBool));

            Formal* i = new Formal(new string("i"), new string("int32"), {1, 1});
            Formals* printInt32_formals = new Formals();
            printInt32_formals->addFormal(unique_ptr<Formal>(i));
            Method* _printInt32 = new Method(new string("printInt32"), printInt32_formals, new string("Object"), new Block(), {1, 1});
            cb->addMethod(unique_ptr<Method>(_printInt32));
            
            Method* inputLine = new Method(new string("inputLine"), new Formals(), new string("string"), new Block(), {1, 1});
            cb->addMethod(unique_ptr<Method>(inputLine));
            Method* inputBool = new Method(new string("inputBool"), new Formals(), new string("bool"), new Block(), {1, 1});
            cb->addMethod(unique_ptr<Method>(inputBool));
            Method* inputInt32 = new Method(new string("inputInt32"), new Formals(), new string("int32"), new Block(), {1, 1});
            cb->addMethod(unique_ptr<Method>(inputInt32));

            return new Class(new string("Object"), nullptr, cb, {1, 1});
        }

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            if (classes.size() == 1) {
                return "";
            }
            string classesToStr = "["; list<unique_ptr<Class>>::iterator f_it;
            for (f_it = classes.begin(); f_it != classes.end(); f_it++) {
                if (*((*f_it)->name) != "Object")
                    classesToStr += (*f_it)->toString(c, classesByName) + ", ";
            }
            if (classesToStr.length() > 1 && classesToStr.substr(classesToStr.length()-2) == ", ") {
                classesToStr = classesToStr.substr(0, classesToStr.length()-2);
            }
            classesToStr += "]\n";
            return classesToStr;
        }

        void checkSemantic(){
            checkClasses();
            // inform each expression in which class it actually is
            setExpressionClasses();
            // set scope context (inform each scope which identifiers are defined)
            set_scope_context();
            // check undefined identifiers in fields and methods
            checkUndefinedIdentifiers();
            checkCallsToUndefinedMethods();
            
            // check inheritance cycles
            checkInheritanceCycles(classes);

            // check undefined types (classes)
            checkTypes(classesByName);

            // check main
            checkMain(classesByName);

        }

        void set_scope_context(){
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                (*a_class)->set_scope_context(classesByName);
            }
        }

        void checkCallsToUndefinedMethods(){
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                (*a_class)->checkCallsToUndefinedMethods(classesByName);
            }
        }

        void setExpressionClasses(){
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                (*a_class)->setExpressionClasses(*((*a_class)->name));
            }
        }

        void checkMain(map<string, unique_ptr<Class> *> classesByName){
            map<string, unique_ptr<Class> *>::iterator class_pair = classesByName.find("Main");
            if (class_pair != classesByName.end()) {
                unique_ptr<Class> * mainClass = class_pair->second;
                (*mainClass)->checkMain();
            } else {
                printError("No main class", {1,1});
            }
        }

        void checkClasses() {
            list<unique_ptr<Class>>::iterator it;
            for (it = classes.begin(); it != classes.end(); it++) {
                // check classes redefinitions
                // and build a map to find classes by name
                map<string, unique_ptr<Class>*>::iterator a_pair = classesByName.find(*((*it)->name));
                if (a_pair == classesByName.end()) {
                    classesByName.insert(make_pair(*((*it)->name), &(*it)));
                } else {
                    printError("class " + *((*it)->name) + " redefined", (*it)->position);
                }
                
                // check redefined Fields and Methods (only, not all identifiers)
                // and add them in a map <name, type>
                (*it)->checkMethodsRedefinitions();
                (*it)->checkFieldsRedefinitions(classesByName);
            }
        }

        void checkUndefinedIdentifiers() {
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                (*a_class)->checkUndefinedIdentifiers();
            }
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator a_class_pair;
            for (a_class_pair = classesByName.begin(); a_class_pair != classesByName.end(); a_class_pair++) {
                (*(a_class_pair->second))->checkTypes(classesByName);

                // Check overidden method and return type
                unique_ptr<Class>* child_class = a_class_pair->second;
                if ((*child_class)->parent != nullptr) {
                    string parent_name = *((*child_class)->parent);

                    // Get the parent class object
                    map<string, unique_ptr<Class> *>::iterator parent_pair;
                    parent_pair = classesByName.find(parent_name);
                    checkParentMethods(*(a_class_pair->second), *(parent_pair->second));
                }                
            }
        }

        void checkParentMethods(unique_ptr<Class>& childClass, unique_ptr<Class>& parentClass){
            list<unique_ptr<Method>>::iterator itChild;
            for(itChild = childClass->classBody->methods->methods.begin(); itChild != childClass->classBody->methods->methods.end(); itChild++){
                unique_ptr<Class> * parent = &parentClass;
                while (!isMethodInClass(*itChild, *parent) && (*parent)->parent != nullptr) { // look in the parent class of the parent
                    map<string, unique_ptr<Class> *>::iterator pair = classesByName.find(*((*parent)->parent));
                    if (pair != classesByName.end()) {
                        parent = pair->second;
                    } else break;
                }
            }
        }

        bool isMethodInClass(unique_ptr<Method> & method, unique_ptr<Class> & class_ptr) {
            list<unique_ptr<Method>>::iterator itParent;
            bool method_found = false;
            for(itParent = class_ptr->classBody->methods->methods.begin(); itParent != class_ptr->classBody->methods->methods.end(); itParent++){
                // If the methods have the same name
                if(*(method->name) == *((*itParent)->name)){
                    method_found = true;
                    // If the return type is not the same
                    if(*(method->returnType) != *((*itParent)->returnType)){
                        printError(*(method->name) + " method has not the same type as in its parent class " + *(class_ptr->name), method->position);
                    }

                    // If the type of each argument is not the same
                    if(method->formals->formals.size() == (*itParent)->formals->formals.size()){
                        list<unique_ptr<Formal>>::iterator itArgChild;
                        int i = 0;
                        for(itArgChild = method->formals->formals.begin();  itArgChild != method->formals->formals.end(); itArgChild++){
                            list<unique_ptr<Formal>>::iterator itArgParent;
                            itArgParent = (*itParent)->formals->formals.begin();
                            advance(itArgParent, i);
                            if(*((*itArgChild)->type) != *((*itArgParent)->type)){
                                printError(*((*itArgChild)->name) + " argument of method " + *(method->name) + " has not the same type as in its parent class " + *(class_ptr->name), (*itArgChild)->position);
                            }
                            i++;
                        }
                    } else {
                        printError(*(method->name) + " method has not the same number of argument as in its parent class " + *(class_ptr->name), method->position);
                    }
                    break;
                }
            }
            return method_found;
        }

        void checkInheritanceCycles(list<unique_ptr<Class>>& classes) {
            list<unique_ptr<Class>>::iterator a_class;
            for (a_class = classes.begin(); a_class != classes.end(); a_class++) {
                list<string> ancestors;
                string parentClass = hasInheritanceCycle(*((*a_class)->name), ancestors);
                if (parentClass.compare("") != 0) {
                    printError("class " + *((*a_class)->name) + " cannot extend child class " + parentClass, (*a_class)->position);
                }
            }
        }

        string hasInheritanceCycle(string class_name, list<string> ancestors) {
            map<string, unique_ptr<Class> *>::iterator a_pair = classesByName.find(class_name);
            if (a_pair != classesByName.end()) {
                unique_ptr<Class> *class_ptr = a_pair->second;
                if ((*class_ptr)->parent == nullptr) {
                    return "";
                } else if (find(ancestors.begin(), ancestors.end(), *((*class_ptr)->parent)) != ancestors.end()) {
                    return *((*class_ptr)->parent);
                }
                ancestors.push_back(*((*class_ptr)->parent));
                return hasInheritanceCycle(*((*class_ptr)->parent), ancestors);
            }
            return "";
        }

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

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = "If(" + conditionExpr->toString(c, classesByName) + ", " + thenExpr->toString(c, classesByName);
            if (elseExpr != nullptr) {
                content += ", " + elseExpr->toString(c, classesByName);
            }
            content += ")";
            if (c) 
                content += " : " + getType(classesByName);
            return content;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            conditionExpr->checkTypes(classesByName);
            thenExpr->checkTypes(classesByName);
            if (elseExpr != nullptr) {
                elseExpr->checkTypes(classesByName);
                if(thenExpr->getType(classesByName) != "unit" && elseExpr->getType(classesByName) != "unit"){
                    if(thenExpr->getType(classesByName) != elseExpr->getType(classesByName)){
                        printError("Both branches do not agree", position);
                    }
                }
            }
        }

        void checkUndefinedIdentifiers() {
            conditionExpr->checkUndefinedIdentifiers();
            thenExpr->checkUndefinedIdentifiers();
            if (elseExpr != nullptr) {
                elseExpr->checkUndefinedIdentifiers();
            }
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            conditionExpr->checkCallsToUndefinedMethods(classesByName);
            thenExpr->checkCallsToUndefinedMethods(classesByName);
            if (elseExpr != nullptr) {
                elseExpr->checkCallsToUndefinedMethods(classesByName);
            }
        }

        void setExpressionClasses(string name) {
            class_name = name;
            conditionExpr->setExpressionClasses(name);
            thenExpr->setExpressionClasses(name);
            if (elseExpr != nullptr)
                elseExpr->setExpressionClasses(name);
        }

        void set_scope_context(map<string, string> &identifiers) {
            conditionExpr->set_scope_context(identifiers);
            thenExpr->set_scope_context(identifiers);
            if (elseExpr != nullptr)
                elseExpr->set_scope_context(identifiers);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            string type = "";
            if(thenExpr->getType(classesByName) == "unit" || elseExpr->getType(classesByName) == "unit")
                type = "unit";      
            else
                type = thenExpr->getType(classesByName);
            return type;
        }
};

class IntegerExpression : public Expression {
    public:
        int value;
        string class_name;
        IntegerExpression(int val, Position p) : Expression(p), value(val) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = to_string(value);
            if (c)
                content += " : int32";
            return content;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }

        void checkUndefinedIdentifiers() { /* empty */ }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) { /* empty */ }

        void setExpressionClasses(string name) { class_name = name; }

        void set_scope_context(map<string, string> &identifiers) { /* empty */ }

        string getType(map<string, unique_ptr<Class> *> & classesByName) { return "int32"; }
        

};

class StringLitExpression : public Expression {
    public:
        unique_ptr<string> value;
        string class_name;
        StringLitExpression(string* val, Position p) : Expression(p), value(val) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = *value;
            if (c)
                content += " : string";
            return content;
        }

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

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = *value;
            if (c)
                content += " : bool";
            return content;
        }

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

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = "While(" + conditionExpr->toString(c, classesByName) + ", " + bodyExpr->toString(c, classesByName) + ")";
            if (c)
                content += " : "  + getType(classesByName);
            return content;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            conditionExpr->checkTypes(classesByName);
            bodyExpr->checkTypes(classesByName);
        }

        void checkUndefinedIdentifiers() {
            conditionExpr->checkUndefinedIdentifiers();
            bodyExpr->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            conditionExpr->checkCallsToUndefinedMethods(classesByName);
            bodyExpr->checkCallsToUndefinedMethods(classesByName);
        }

        void setExpressionClasses(string name) {
            class_name = name;
            conditionExpr->setExpressionClasses(name);
            bodyExpr->setExpressionClasses(name);
        }

        void set_scope_context(map<string, string> &identifiers) { 
            conditionExpr->set_scope_context(identifiers);
            bodyExpr->set_scope_context(identifiers);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            return "unit";
        }
};

class Assign : public Expression{
    public:
        unique_ptr<string> name;
        unique_ptr<Expression> expr;
        string class_name;
        map<string, string> scope_identifiers;

        Assign(){}
        Assign(string* _name, Expression* _expr, Position p) : Expression(p), name(_name), expr(_expr) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = "Assign(" + *name + ", " + expr->toString(c, classesByName) + ")";
            if (c)
                content += " : " + getType(classesByName);
            return content;

        }
        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            // check both types match
            string id_type = scope_identifiers.find(*name)->second;
            string expr_type = expr->getType(classesByName);
            if (id_type != expr_type) {
                printError("cannot assign type " + expr_type + " to type " + id_type, expr->position);
            }
            expr->checkTypes(classesByName);
        }

        void checkUndefinedIdentifiers() {
            // search name in fields
            bool found = (scope_identifiers.find(*name) != scope_identifiers.end());
            if (!found) {
                printError("field " + *name + " undefined", position);
            }
            // check in the expression
            expr->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            expr->checkCallsToUndefinedMethods(classesByName);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            return expr->getType(classesByName);
        }

        void setExpressionClasses(string name) {
            class_name = name;
            expr->setExpressionClasses(name);
        }

        void set_scope_context(map<string, string> &identifiers) { 
            scope_identifiers = identifiers;
            expr->set_scope_context(identifiers);
        }      
};

class BinaryOperator : public Expression{
    public:
        unique_ptr<string> op;
        unique_ptr<Expression> left;
        unique_ptr<Expression> right;
        string class_name;
        
        BinaryOperator(){}
        BinaryOperator(string* _op, Expression* _left, Expression* _right, Position p) : Expression(p), op(_op), left(_left), right(_right) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = "BinOp(" + *op + ", " + left->toString(c, classesByName) + ", " + right->toString(c, classesByName) + ")";
            if (c)
                content += " : " + getType(classesByName);
            return content;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName){
            left->checkTypes(classesByName);
            right->checkTypes(classesByName);
        }

        void checkUndefinedIdentifiers() {
            left->checkUndefinedIdentifiers();
            right->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            left->checkCallsToUndefinedMethods(classesByName);
            right->checkCallsToUndefinedMethods(classesByName);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            if (op->compare("<=") == 0 || op->compare("<") == 0 || op->compare("=") == 0) {
                return "bool";
            }
            return "int32";
        }

        void setExpressionClasses(string name) {
            class_name = name;
            left->setExpressionClasses(name);
            right->setExpressionClasses(name);
        }

        void set_scope_context(map<string, string> &identifiers) { 
            left->set_scope_context(identifiers);
            right->set_scope_context(identifiers);
        }  
};


class Call : public Expression {
    public:
        unique_ptr<Expression> objExpr = nullptr;
        unique_ptr<string> methodName;
        unique_ptr<Block> args;
        string class_name;

        Call(){}
        Call(Expression* _objExpr, string* _methodName, Block* _args, Position p) : Expression(p), objExpr(_objExpr), methodName(_methodName), args(_args) {}
        Call(string* _methodName, Block* _args, Position p) : Expression(p), methodName(_methodName), args(_args) {}

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string obj = "self";
            if (c)
                obj += " : " + class_name;
            if (objExpr != nullptr) {
                obj = objExpr->toString(c, classesByName);
            }
            string content = "Call(" + obj + ", " + *methodName + ", " + args->toString(c, classesByName) + ")";
            if (c)
                content += " : " + getType(classesByName);
            return content;
        }


        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            if (objExpr != nullptr) {
                objExpr->checkTypes(classesByName);
            }
            // check each arg
            list<unique_ptr<Expression>>::iterator args_it;
            for (args_it = args->exprList.begin(); args_it != args->exprList.end(); args_it++) {
                (*args_it)->checkTypes(classesByName);
            }
        }

        void checkUndefinedIdentifiers() {
            // check in the expression
            if (objExpr != nullptr) {
                objExpr->checkUndefinedIdentifiers();
            }
            args->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            if (objExpr != nullptr)
                objExpr->checkCallsToUndefinedMethods(classesByName);
            // getType of objExpr and verify the type has the method
            checkMethodReturnTypeAndArgs(classesByName);
            args->checkCallsToUndefinedMethods(classesByName);
        }

        void checkMethodReturnTypeAndArgs(map<string, unique_ptr<Class> *> & classesByName) {
            unique_ptr<Class> *class_ptr;
            if (objExpr == nullptr) {
                map<string, unique_ptr<Class> *>::iterator self_class = classesByName.find(class_name);
                class_ptr = self_class->second;
            } else {
                string objExprType = objExpr->getType(classesByName);
                if (objExprType == "") return;
                if (isPrimitive(objExprType)) {
                    printError("type " + objExpr->getType(classesByName) + " has no method called " + *methodName, position);
                    return;
                } else {
                    map<string, unique_ptr<Class> *>::iterator a_class = classesByName.find(objExprType);
                    class_ptr = a_class->second;
                }
            }
            string type = getMethodType(*class_ptr, classesByName, *methodName);
            if (type.compare("") == 0) {
                printError("type " + class_name + " has no method called " + *methodName, position);
            }
            checkMethodFormals(*class_ptr, classesByName, *methodName, args);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            // if we call a method of "class_name"
            if (objExpr == nullptr) {
                map<string, unique_ptr<Class> *>::iterator self_class = classesByName.find(class_name);
                unique_ptr<Class> *class_ptr = self_class->second;
                string type = getMethodType(*class_ptr, classesByName, *methodName);
                if (type.compare("") == 0) {
                    return "";
                }
                return type;
            }
            string objExprType = objExpr->getType(classesByName);
            // get methods of this class
            if (!isPrimitive(objExprType)) {
                map<string, unique_ptr<Class> *>::iterator a_class = classesByName.find(objExprType);
                if (a_class != classesByName.end()) {
                    unique_ptr<Class> *class_ptr = a_class->second;
                    string type = getMethodType(*class_ptr, classesByName, *methodName);
                    if (type.compare("") == 0) {
                        return "";
                    }
                    return type;
                }
            }
            return "";
        }

        bool isPrimitive(string type) {
            return type.compare("string") == 0 
                || type.compare("bool") == 0 
                || type.compare("int32") == 0 
                || type.compare("unit") == 0;
        }

        bool inheritsFrom(string child_class, string parent_class, map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator it = classesByName.find(child_class);
            if (it != classesByName.end()) {
                if ((*(it->second))->parent != nullptr) {
                    if (*((*(it->second))->parent) == parent_class)
                        return true;
                    else
                        return inheritsFrom(*((*(it->second))->parent), parent_class, classesByName);
                } else return false;
            }
            return false;
        }

        void checkMethodFormals(unique_ptr<Class> & current_class, map<string, unique_ptr<Class> *> & classesByName, string methodName, unique_ptr<Block> & args) {
            list<unique_ptr<Method>> & methods = current_class->classBody->methods->methods;
            list<unique_ptr<Method>>::iterator method_it;
            // find the method
            for (method_it = methods.begin(); method_it != methods.end(); method_it++) {
                if (*((*method_it)->name) == methodName) {
                    // check nb of formals of the method
                    if ((*method_it)->formals->formals.size() != args->exprList.size()) {
                        printError("call to method " + methodName + " of class " + *(current_class->name) + " does not match the required number of formal arguments", (*method_it)->position);
                        return;
                    }
                    // check types of formals of the method
                    list<unique_ptr<Formal>>::iterator formal_it = (*method_it)->formals->formals.begin();
                    list<unique_ptr<Expression>>::iterator args_it = args->exprList.begin();
                    while (formal_it != (*method_it)->formals->formals.end()) {
                        string arg_type = (*args_it)->getType(classesByName);
                        if (*((*formal_it)->type) != arg_type && !inheritsFrom(arg_type, *((*formal_it)->type), classesByName)) {
                            printError("cannot assign type " + arg_type + " to formal argument " + *((*formal_it)->name) + " : " + *((*formal_it)->type) + " of method " + methodName, (*formal_it)->position);
                        }
                        advance(args_it, 1);
                        advance(formal_it, 1);
                    }
                }
            }
            if (current_class->parent == nullptr) {
                return "";
            }
            map<string, unique_ptr<Class> *>::iterator parent_pair = classesByName.find(*(current_class->parent));
            return checkMethodFormals(*(parent_pair->second), classesByName, methodName, args);
        }

        string getMethodType(unique_ptr<Class> & current_class, map<string, unique_ptr<Class> *> & classesByName, string methodName) {
            
            map<string, string> methods_types = current_class->getMethodsAndTypes();
            map<string, string>::iterator method_type = methods_types.find(methodName);
            if (method_type != methods_types.end()) {
                // get return type of the corresponding method
                return method_type->second;
            } else {
                if (current_class->parent == nullptr) {
                    return "";
                }
                map<string, unique_ptr<Class> *>::iterator parent_pair = classesByName.find(*(current_class->parent));
                return getMethodType(*(parent_pair->second), classesByName, methodName);
            }
        }

        void setExpressionClasses(string name) {
            class_name = name;
            args->setExpressionClasses(name);
            if (objExpr != nullptr) {
                objExpr->setExpressionClasses(name);
            }
        }

        void set_scope_context(map<string, string> &identifiers) { 
            if (objExpr != nullptr) {
                objExpr->set_scope_context(identifiers);
            }
            args->set_scope_context(identifiers);
        }
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

        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string firstPart = "Let(" + *name + ", " + *type + ", ";
            string lastPart = "";
            if (init != nullptr) {
                lastPart += init->toString(c, classesByName);
                lastPart += ", ";
            }
            lastPart += scope->toString(c, classesByName) + ")";
            if (c)
                lastPart += " : " + getType(classesByName);
            return firstPart + lastPart;   
        }


        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator a_pair;
            a_pair = classesByName.find(*type);
            if (isupper((*type)[0]) && a_pair == classesByName.end()) {
                // not found
                printError("type " + *type + " not defined", position);
            }
            if (init != nullptr) {
                init->checkTypes(classesByName);
                string initType = init->getType(classesByName);
                if (initType != *type) {
                    printError("expected type " + *type + ", but found type " + initType, init->position);
                }
            }
            scope->checkTypes(classesByName);
        }

        void checkUndefinedIdentifiers() {
            // check the init expression
            if (init != nullptr) {
                init->checkUndefinedIdentifiers();
            }
            // check in the scope
            scope->checkUndefinedIdentifiers();
        }

        void checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> & classesByName) {
            if (init != nullptr)
                init->checkCallsToUndefinedMethods(classesByName);
            scope->checkCallsToUndefinedMethods(classesByName);
        }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            return scope->getType(classesByName);
        }

        void setExpressionClasses(string name) {
            class_name = name;
            scope->setExpressionClasses(name);
            if (init != nullptr){
                init->setExpressionClasses(name);
            }
        }

        void set_scope_context(map<string, string> &identifiers) { 
            if (init != nullptr) {
                init->set_scope_context(identifiers);
            }
            pair<string, string> p(*name, *type);
            scope_identifiers = identifiers;
            map<string, string>::iterator it = scope_identifiers.find(p.first); 
            if (it != scope_identifiers.end())
                it->second = p.second;
            else
                scope_identifiers.insert(p);
            map<string, string> new_identifiers(scope_identifiers);
            scope->set_scope_context(new_identifiers);
        }
};

class New : public Expression {
    public:
        unique_ptr<string> type;
        string class_name;

        New(){}
        New(string* _type, Position p) : Expression(p), type(_type) {}
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = "New(" + *type + ")";
            if (c)
                content += " : " + *type;
            return content;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, unique_ptr<Class> *>::iterator a_pair;
            a_pair = classesByName.find(*type);
            if (isupper((*type)[0]) && a_pair == classesByName.end()) {
                // not found
                printError("type " + *type + " not defined", position);
            }
        }

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
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = *name;
            if (c)
                content += " : " + getType(classesByName);
            return content;
        }

        void checkTypes(map<string, unique_ptr<Class> *> & classesByName) { 
            map<string, string>:: iterator it = scope_identifiers.find(*name);
            if (it == scope_identifiers.end()) {
                printError("object-identifier " + *name + " is undefined", position);
            }
        }

        void checkUndefinedIdentifiers() { /* empty */ }

        string getType(map<string, unique_ptr<Class> *> & classesByName) {
            map<string, string>:: iterator it = scope_identifiers.find(*name);
            if (it != scope_identifiers.end()) {
                return it->second;
            }
            return "";
        }

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
        string toString(bool c, map<string, unique_ptr<Class> *> classesByName) {
            string content = "()";
            if (c)
                content += " : unit";
            return content;
        }

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
