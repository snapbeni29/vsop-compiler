#include <iostream>
#include <list>
#include <string>
#include <iterator>
#include <memory>
#include <map>
#include <algorithm>
#include "tree.hh"

using namespace std;
using namespace llvm;

extern string filename;
extern int errors;

bool Utils::inheritsFrom(string child_class, string parent_class, map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator it = classesByName.find(child_class);
    if (it != classesByName.end())
    {
        if ((*(it->second))->parent != nullptr)
        {
            if (*((*(it->second))->parent) == parent_class)
                return true;
            else
                return Utils::inheritsFrom(*((*(it->second))->parent), parent_class, classesByName);
        }
        else
            return false;
    }
    return false;
}

void TreeNode::printError(string s, Position p)
{
    cerr << filename << ":" << p.row << ":" << p.col << ": semantic error: " << s << endl;
    errors++;
}

//////////
// UNOP //
//////////

string UnOp::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "";
    content += "UnOp(" + *op + ", " + expr->toString(c, classesByName) + ")";
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void UnOp::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    string exprType = expr->getType(classesByName);
    if (*op == "isnull" && isPrimitive(exprType))
    {
        printError("isnull is applied to a primitive type", expr->position);
    }
    expr->checkTypes(classesByName);
}

void UnOp::checkUndefinedIdentifiers()
{
    expr->checkUndefinedIdentifiers();
}

void UnOp::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    expr->checkCallsToUndefinedMethods(classesByName);
}

string UnOp::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    if ((*op).compare("isnull") == 0)
    {
        return "bool";
    }
    return expr->getType(classesByName);
}

void UnOp::setExpressionClasses(string name)
{
    class_name = name;
    expr->setExpressionClasses(name);
}

void UnOp::set_scope_context(map<string, string> &identifiers)
{
    expr->set_scope_context(identifiers);
}

Value *UnOp::codegen(LLVM &ll)
{
    Value *val = expr->getValue();
    if (!val)
        return nullptr;

    switch (op)
    {
    case NOT:
        return ll.Builder->CreateNot(val);
    case MINUS:
        return ll.Builder->CreateNeg(val);
    case ISNULL:
        return ll.Builder->CreateIsNull(val);
    default:
        return nullptr;
    }
}

////////////////
// EXPRESSION //
////////////////

bool Expression::isPrimitive(string type)
{
    return type.compare("string") == 0 || type.compare("bool") == 0 || type.compare("int32") == 0 || type.compare("unit") == 0;
}

///////////
// BLOCK //
///////////

void Block::addExpression(unique_ptr<Expression> e)
{
    exprList.push_front(move(e));
    position = exprList.back()->position;
}

string Block::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string exprsToStr = "[";
    list<unique_ptr<Expression>>::iterator f_it;
    for (f_it = exprList.begin(); f_it != exprList.end(); f_it++)
    {
        exprsToStr += (*f_it)->toString(c, classesByName) + ", ";
    }
    if (exprsToStr.length() > 1 && exprsToStr.substr(exprsToStr.length() - 2) == ", ")
    {
        exprsToStr = exprsToStr.substr(0, exprsToStr.length() - 2);
    }
    exprsToStr += "]";
    if (c)
        exprsToStr += " : " + getType(classesByName);
    return exprsToStr;
}

void Block::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Expression>>::iterator f_it;
    for (f_it = exprList.begin(); f_it != exprList.end(); f_it++)
    {
        (*f_it)->checkTypes(classesByName);
    }
}

void Block::checkUndefinedIdentifiers()
{
    list<unique_ptr<Expression>>::iterator f_it;
    for (f_it = exprList.begin(); f_it != exprList.end(); f_it++)
    {
        (*f_it)->checkUndefinedIdentifiers();
    }
}

void Block::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Expression>>::iterator ex;
    for (ex = exprList.begin(); ex != exprList.end(); ex++)
    {
        (*ex)->checkCallsToUndefinedMethods(classesByName);
    }
}

string Block::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    return exprList.back()->getType(classesByName);
}

void Block::setExpressionClasses(string name)
{
    class_name = name;
    list<unique_ptr<Expression>>::iterator ex;
    for (ex = exprList.begin(); ex != exprList.end(); ex++)
    {
        (*ex)->setExpressionClasses(name);
    }
}

void Block::set_scope_context(map<string, string> &identifiers)
{
    list<unique_ptr<Expression>>::iterator ex;
    for (ex = exprList.begin(); ex != exprList.end(); ex++)
    {
        (*ex)->set_scope_context(identifiers);
    }
}

llvm::Value *Block::codegen(LLVM &ll)
{
    for (auto &expr : exprList)
    {
        expr->codegen(ll);
    }
    return exprList.size() == 0 ? nullptr : exprlist.back()->getValue(ll);
}

////////////
// FORMAL //
////////////

string Formal::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    return *name + " : " + *type;
}

void Formal::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator a_pair;
    a_pair = classesByName.find(*type);
    if (isupper((*type)[0]) && a_pair == classesByName.end())
    {
        // not found
        printError("type" + *type + " not defined", position);
    }
}

void Formal::setClass(string name)
{
    class_name = name;
}

/////////////
// FORMALS //
/////////////

string Formals::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string formalsToStr = "[";
    list<unique_ptr<Formal>>::iterator f_it;
    for (f_it = formals.begin(); f_it != formals.end(); f_it++)
    {
        formalsToStr += (*f_it)->toString(c, classesByName) + ", ";
    }
    if (formalsToStr.length() > 1 && formalsToStr.substr(formalsToStr.length() - 2) == ", ")
    {
        formalsToStr = formalsToStr.substr(0, formalsToStr.length() - 2);
    }
    formalsToStr += "]";
    return formalsToStr;
}

void Formals::addFormal(unique_ptr<Formal> f)
{
    formals.push_front(move(f));
}

void Formals::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Formal>>::iterator it;
    for (it = formals.begin(); it != formals.end(); it++)
    {
        // check formal types
        (*it)->checkTypes(classesByName);
    }
}

void Formals::setExpressionClasses(string name)
{
    list<unique_ptr<Formal>>::iterator formal;
    for (formal = formals.begin(); formal != formals.end(); formal++)
    {
        (*formal)->setClass(name);
    }
}

////////////
// METHOD //
////////////

llvm::Value *Method::codegen(LLVM &ll)
{

    std::vector<Type *> types;
    for (auto &formal : formals->formals)
    {
        types.push_back(ll.toLLVMType(*(formal->type)));
    }
    FunctionType *funcType = FunctionType::get(ll.toLLVMType(*returnType), types, false);

    Function *func = Function::Create(funcType, Function::ExternalLinkage, *name + "_func_" + class_name, ll.TheModule.get());
    unsigned idx = 0;

    for (auto &arg : func->args())
        arg.setName(*(formals->formals[idx++]->name));

    if (!func->empty())
    {
        // Handle function redefinition
    }

    BasicBlock *bb = BasicBlock::Create(*ll.TheContext, *name + "_func_block_" + class_name, func);
    ll.Builder->SetInsertPoint(bb);

    // Add formals
    for (auto &arg : func->args())
        ll.pushValue(arg.getName(), &arg);

    Value *returnVal = block->codegen();

    for (auto &arg : func->args())
        ll.popValue(&arg);

    if (returnVal)
    {
        ll.Builder->CreateRet(returnVal);
        verifyFunction(*func);
        return func;
    }
    return nullptr;
}

string Method::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    return "Method(" + *name + ", " + formals->toString(c, classesByName) + ", " + *returnType + ", " + block->toString(c, classesByName) + ")";
}

void Method::checkFormalArgumentsRedefinition()
{
    list<unique_ptr<Formal>>::iterator it;
    for (it = formals->formals.begin(); it != formals->formals.end(); it++)
    {
        // check formal arguments redefinitions
        bool found = (find(formal_names.begin(), formal_names.end(), *((*it)->name)) != formal_names.end());
        if (!found)
        {
            formal_names.push_back(*((*it)->name));
        }
        else
        {
            printError("formal argument " + *((*it)->name) + " redefined", (*it)->position);
        }
    }
}

void Method::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator a_pair;
    a_pair = classesByName.find(*returnType);
    if (isupper((*returnType)[0]) && a_pair == classesByName.end())
    {
        // not found
        printError("type " + *returnType + " not defined", position);
    }
    formals->checkTypes(classesByName);
    block->checkTypes(classesByName);
    // verify if block type matches returnType
    if (block->exprList.size() == 0)
        return;
    string blockType = block->getType(classesByName);
    if (blockType != *returnType && !Utils::inheritsFrom(blockType, *returnType, classesByName))
    {
        printError("block of method " + *name + " in class " + class_name + " should return a value of type " + *returnType + " but returns a value of type " + blockType, block->exprList.back()->position);
    }
}

void Method::checkUndefinedIdentifiers()
{
    block->checkUndefinedIdentifiers();
}

void Method::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    block->checkCallsToUndefinedMethods(classesByName);
}

void Method::setExpressionClasses(string name)
{
    class_name = name;
    block->setExpressionClasses(name);
    formals->setExpressionClasses(name);
}

void Method::set_scope_context(map<string, string> &identifiers)
{
    scope_context = identifiers;
    list<unique_ptr<Formal>>::iterator formal;
    for (formal = formals->formals.begin(); formal != formals->formals.end(); formal++)
    {
        pair<string, string> p(*((*formal)->name), *((*formal)->type));
        map<string, string>::iterator it = scope_context.find(*((*formal)->name));
        if (it != scope_context.end())
            it->second = *((*formal)->type);
        else
            scope_context.insert(p);
    }
    block->set_scope_context(scope_context);
}

void Method::checkMain()
{
    if (*returnType != "int32")
    {
        printError("main method must return int32", position);
    }
    if (formals->formals.begin() != formals->formals.end())
    {
        printError("multiple parameters for main()", position);
    }
}

/////////////
// METHODS //
/////////////

string Methods::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string methodsToStr = "[";
    list<unique_ptr<Method>>::iterator m_it;
    for (m_it = methods.begin(); m_it != methods.end(); m_it++)
    {
        methodsToStr += (*m_it)->toString(c, classesByName) + ", ";
    }
    if (methodsToStr.length() > 1 && methodsToStr.substr(methodsToStr.length() - 2) == ", ")
    {
        methodsToStr = methodsToStr.substr(0, methodsToStr.length() - 2);
    }
    methodsToStr += "]";
    return methodsToStr;
}

void Methods::addMethod(unique_ptr<Method> m)
{
    methods.push_front(move(m));
}

void Methods::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Method>>::iterator it2;
    for (it2 = methods.begin(); it2 != methods.end(); it2++)
    {
        // check types
        (*it2)->checkTypes(classesByName);
    }
}

void Methods::checkUndefinedIdentifiers()
{
    list<unique_ptr<Method>>::iterator it2;
    for (it2 = methods.begin(); it2 != methods.end(); it2++)
    {
        (*it2)->checkUndefinedIdentifiers();
    }
}

void Methods::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Method>>::iterator method;
    for (method = methods.begin(); method != methods.end(); method++)
    {
        (*method)->checkCallsToUndefinedMethods(classesByName);
    }
}

void Methods::setExpressionClasses(string name)
{
    list<unique_ptr<Method>>::iterator a_method;
    for (a_method = methods.begin(); a_method != methods.end(); a_method++)
    {
        (*a_method)->setExpressionClasses(name);
    }
}

void Methods::set_scope_context(map<string, string> &identifiers)
{
    list<unique_ptr<Method>>::iterator a_method;
    for (a_method = methods.begin(); a_method != methods.end(); a_method++)
    {
        (*a_method)->set_scope_context(identifiers);
    }
}

///////////
// FIELD //
///////////

string Field::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string fieldStr = "Field(" + *name + ", " + *type;
    if (initExpr != nullptr)
    {
        fieldStr += ", " + initExpr->toString(c, classesByName);
    }
    fieldStr += ")";
    return fieldStr;
}

void Field::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator a_pair;
    a_pair = classesByName.find(*type);
    if (isupper((*type)[0]) && a_pair == classesByName.end())
    {
        // not found
        printError("type " + *type + " not defined", position);
    }
    if (initExpr != nullptr)
    {
        string resultType = initExpr->getType(classesByName);
        if (resultType != *type && !Utils::inheritsFrom(resultType, *type, classesByName))
        {
            printError("expected type was " + *type + ", but found " + resultType, initExpr->position);
        }
        initExpr->checkTypes(classesByName);
    }
}

void Field::checkUndefinedIdentifiers()
{
    // check in the expression
    if (initExpr != nullptr)
    {
        initExpr->checkUndefinedIdentifiers();
    }
}

void Field::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    if (initExpr != nullptr)
    {
        initExpr->checkCallsToUndefinedMethods(classesByName);
    }
}

void Field::setExpressionClasses(string name)
{
    class_name = name;
    if (initExpr != nullptr)
    {
        initExpr->setExpressionClasses(name);
    }
}

string Field::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    return *type;
}

void Field::set_scope_context(map<string, string> &identifiers)
{
    scope_identifiers = identifiers;
    pair<string, string> p(*name, *type);
    scope_identifiers.insert(p);
    if (initExpr != nullptr)
        initExpr->set_scope_context(scope_identifiers);
}

////////////
// FIELDS //
////////////

string Fields::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string fieldsToStr = "[";
    list<unique_ptr<Field>>::iterator f_it;
    for (f_it = fields.begin(); f_it != fields.end(); f_it++)
    {
        fieldsToStr += (*f_it)->toString(c, classesByName) + ", ";
    }
    if (fieldsToStr.length() > 1 && fieldsToStr.substr(fieldsToStr.length() - 2) == ", ")
    {
        fieldsToStr = fieldsToStr.substr(0, fieldsToStr.length() - 2);
    }
    fieldsToStr += "]";
    return fieldsToStr;
}

void Fields::addField(unique_ptr<Field> f)
{
    fields.push_front(move(f));
}

void Fields::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Field>>::iterator it2;
    for (it2 = fields.begin(); it2 != fields.end(); it2++)
    {
        // check types
        (*it2)->checkTypes(classesByName);
    }
}

void Fields::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    list<unique_ptr<Field>>::iterator a_field;
    for (a_field = fields.begin(); a_field != fields.end(); a_field++)
    {
        (*a_field)->checkCallsToUndefinedMethods(classesByName);
    }
}

void Fields::setExpressionClasses(string name)
{
    list<unique_ptr<Field>>::iterator a_field;
    for (a_field = fields.begin(); a_field != fields.end(); a_field++)
    {
        (*a_field)->setExpressionClasses(name);
    }
}

void Fields::checkUndefinedIdentifiers()
{
    list<unique_ptr<Field>>::iterator a_field;
    for (a_field = fields.begin(); a_field != fields.end(); a_field++)
    {
        (*a_field)->checkUndefinedIdentifiers();
    }
}

void Fields::set_scope_context(map<string, string> &identifiers)
{
    list<unique_ptr<Field>>::iterator a_field;
    for (a_field = fields.begin(); a_field != fields.end(); a_field++)
    {
        (*a_field)->set_scope_context(identifiers);
    }
}

///////////////
// CLASSBODY //
///////////////

llvm::Value *ClassBody::codegen(LLVM &ll)
{
    llvm::Function *function = ll.TheModule->getFunction(class_name);

    llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(*ll.TheContext, "", function);
    ll.Builder->SetInsertPoint(entry_block);

    // Methods code generation
    //methods->codegen();
    return nullptr;
}

void ClassBody::addField(unique_ptr<Field> field)
{
    fields->addField(move(field));
}

void ClassBody::addMethod(unique_ptr<Method> method)
{
    methods->addMethod(move(method));
}

void ClassBody::checkMethodsRedefinitions()
{
    list<unique_ptr<Method>>::iterator it;
    for (it = methods->methods.begin(); it != methods->methods.end(); it++)
    {
        // check method redefinitions
        bool found = (method_types.find(*((*it)->name)) != method_types.end());
        if (!found)
        {
            pair<string, string> p(*((*it)->name), *((*it)->returnType));
            method_types.insert(p);
        }
        else
        {
            printError("method " + *((*it)->name) + " redefined", (*it)->position);
        }
        (*it)->checkFormalArgumentsRedefinition();
    }
}

map<string, string> ClassBody::getMethodsAndTypes()
{
    return method_types;
}

void ClassBody::set_scope_context(map<string, string> &identifiers)
{
    methods->set_scope_context(identifiers);
    map<string, string> no_fields_ids;
    // fields have non access to other fields (object not initialized yet)
    fields->set_scope_context(no_fields_ids);
}

map<string, string> ClassBody::getFieldsAndTypes()
{
    return field_types;
}

void ClassBody::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    methods->checkTypes(classesByName);
    fields->checkTypes(classesByName);
}

void ClassBody::checkUndefinedIdentifiers()
{
    methods->checkUndefinedIdentifiers();
    fields->checkUndefinedIdentifiers();
}

void ClassBody::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    fields->checkCallsToUndefinedMethods(classesByName);
    methods->checkCallsToUndefinedMethods(classesByName);
}

void ClassBody::setExpressionClasses(string name)
{
    class_name = name;
    methods->setExpressionClasses(name);
    fields->setExpressionClasses(name);
}

void ClassBody::checkMain()
{
    list<unique_ptr<Method>>::iterator it;
    bool hasMainMethod = false;
    for (it = methods->methods.begin(); it != methods->methods.end(); it++)
    {
        if (*((*it)->name) == "main")
        {
            hasMainMethod = true;
            (*it)->checkMain();
            break;
        }
    }
    if (hasMainMethod == false)
    {
        printError("main method of class Main is not defined", {1, 1});
    }
}

///////////
// CLASS //
///////////

string Class::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    return "Class(" + *name + ", " + *parent + ", " + classBody->fields->toString(c, classesByName) + ", " + classBody->methods->toString(c, classesByName) + ")";
}

llvm::Value *Class::codegen(LLVM &ll)
{
    return nullptr;
}

void Class::checkFieldsRedefinitions(map<string, unique_ptr<Class> *> classesByName)
{
    list<unique_ptr<Field>>::iterator it2;
    for (it2 = classBody->fields->fields.begin(); it2 != classBody->fields->fields.end(); it2++)
    {
        // check field redefinitions
        int currentClassDefinitions = countFieldDefinitions(classBody->fields->fields, *((*it2)->name));
        if (currentClassDefinitions > 1)
            printError("field " + *((*it2)->name) + " of class " + *name + " is already defined in the class", (*it2)->position);

        if (parent == nullptr)
            return;
        string classContainingField = isFieldAlreadyDefined(classesByName, *parent, *((*it2)->name));
        if (classContainingField != "")
        {
            printError("field " + *((*it2)->name) + " of class " + *name + " is already defined in class " + classContainingField, (*it2)->position);
        }
        else
        {
            pair<string, string> p(*((*it2)->name), *((*it2)->type));
            classBody->field_types.insert(p);
        }
    }
}

int Class::countFieldDefinitions(list<unique_ptr<Field>> &field_list, string field_name)
{
    list<unique_ptr<Field>>::iterator it_fields;
    int counter = 0;
    for (it_fields = field_list.begin(); it_fields != field_list.end(); it_fields++)
    {
        if (*((*it_fields)->name) == field_name)
        {
            counter += 1;
        }
    }
    return counter;
}

string Class::isFieldAlreadyDefined(map<string, unique_ptr<Class> *> classesByName, string currentClass, string field_name)
{
    // find the class by name
    map<string, unique_ptr<Class> *>::iterator it_classes = classesByName.find(currentClass);
    if (it_classes != classesByName.end())
    {
        unique_ptr<Class> *class_ptr = it_classes->second;
        list<unique_ptr<Field>>::iterator it_fields;
        if (countFieldDefinitions((*class_ptr)->classBody->fields->fields, field_name) > 0)
            return currentClass;
        if ((*class_ptr)->parent != nullptr)
        {
            return isFieldAlreadyDefined(classesByName, *((*class_ptr)->parent), field_name);
        }
    }
    return "";
}

void Class::checkMethodsRedefinitions()
{
    classBody->checkMethodsRedefinitions();
}

map<string, string> Class::getFieldsAndTypes()
{
    return classBody->getFieldsAndTypes();
}

map<string, string> Class::getMethodsAndTypes()
{
    return classBody->getMethodsAndTypes();
}

void Class::checkUndefinedIdentifiers()
{
    classBody->checkUndefinedIdentifiers();
}

void Class::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    classBody->checkTypes(classesByName);
}

void Class::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    classBody->checkCallsToUndefinedMethods(classesByName);
}

void Class::setExpressionClasses(string name)
{
    classBody->setExpressionClasses(name);
}

void Class::checkMain()
{
    classBody->checkMain();
}

void Class::set_scope_context(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, string> identifiers(classBody->field_types);
    identifiers.insert(make_pair("self", *name));
    if (parent != nullptr)
    {
        list<string> seen_classes = {*name};

        includeParentFields(classesByName, *parent, seen_classes, identifiers);
    }
    classBody->set_scope_context(identifiers);
}

void Class::includeParentFields(map<string, unique_ptr<Class> *> &classesByName, string parent_name, list<string> seen_classes, map<string, string> &identifiers)
{
    if (find(seen_classes.begin(), seen_classes.end(), parent_name) != seen_classes.end())
    {
        return;
    }
    map<string, unique_ptr<Class> *>::iterator class_it = classesByName.find(parent_name);
    list<unique_ptr<Field>>::iterator fields_it;
    if (class_it == classesByName.end())
    {
        return;
    }
    for (fields_it = (*(class_it->second))->classBody->fields->fields.begin(); fields_it != (*(class_it->second))->classBody->fields->fields.end(); fields_it++)
    {
        identifiers.insert(make_pair(*((*fields_it)->name), *((*fields_it)->type)));
    }
    if ((*(class_it->second))->parent == nullptr)
        return;
    seen_classes.push_back(parent_name);
    includeParentFields(classesByName, *((*(class_it->second))->parent), seen_classes, identifiers);
}

/////////////
// PROGRAM //
/////////////

llvm::Value *Program::codegen(LLVM &ll)
{
    list<unique_ptr<Class>>::iterator a_class;
    for (a_class = classes.begin(); a_class != classes.end(); a_class++)
    {
        (*a_class)->codegen(ll);
    }
    map<string, unique_ptr<Class> *>::iterator class_pair = classesByName.find("Main");
    if (class_pair != classesByName.end())
    {
        unique_ptr<Class> *mainClass = class_pair->second;
        llvm::FunctionType *functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ll.TheContext), {}, false);
        llvm::Function *function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", *ll.TheModule);
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(*ll.TheContext, "", function);
        ll.Builder->SetInsertPoint(entryBlock);
        ll.Builder->CreateRet(Call(new New("Main", {0, 0}), "main", new Args(), {0, 0}).codegen(ll));
    }
    return nullptr;
}

Class *Program::createObjectClass()
{
    ClassBody *cb = new ClassBody();
    Formal *s = new Formal(new string("s"), new string("string"), {1, 1});
    Formals *print_formals = new Formals();
    print_formals->addFormal(unique_ptr<Formal>(s));
    Method *_print = new Method(new string("print"), print_formals, new string("Object"), new Block(), {1, 1});
    cb->addMethod(unique_ptr<Method>(_print));

    Formal *b = new Formal(new string("b"), new string("bool"), {1, 1});
    Formals *printBool_formals = new Formals();
    printBool_formals->addFormal(unique_ptr<Formal>(b));
    Method *_printBool = new Method(new string("printBool"), printBool_formals, new string("Object"), new Block(), {1, 1});
    cb->addMethod(unique_ptr<Method>(_printBool));

    Formal *i = new Formal(new string("i"), new string("int32"), {1, 1});
    Formals *printInt32_formals = new Formals();
    printInt32_formals->addFormal(unique_ptr<Formal>(i));
    Method *_printInt32 = new Method(new string("printInt32"), printInt32_formals, new string("Object"), new Block(), {1, 1});
    cb->addMethod(unique_ptr<Method>(_printInt32));

    Method *inputLine = new Method(new string("inputLine"), new Formals(), new string("string"), new Block(), {1, 1});
    cb->addMethod(unique_ptr<Method>(inputLine));
    Method *inputBool = new Method(new string("inputBool"), new Formals(), new string("bool"), new Block(), {1, 1});
    cb->addMethod(unique_ptr<Method>(inputBool));
    Method *inputInt32 = new Method(new string("inputInt32"), new Formals(), new string("int32"), new Block(), {1, 1});
    cb->addMethod(unique_ptr<Method>(inputInt32));

    return new Class(new string("Object"), nullptr, cb, {1, 1});
}

string Program::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    if (classes.size() == 1)
    {
        return "";
    }
    string classesToStr = "[";
    list<unique_ptr<Class>>::iterator f_it;
    for (f_it = classes.begin(); f_it != classes.end(); f_it++)
    {
        if (*((*f_it)->name) != "Object")
            classesToStr += (*f_it)->toString(c, classesByName) + ", ";
    }
    if (classesToStr.length() > 1 && classesToStr.substr(classesToStr.length() - 2) == ", ")
    {
        classesToStr = classesToStr.substr(0, classesToStr.length() - 2);
    }
    classesToStr += "]\n";
    return classesToStr;
}

void Program::checkSemantic()
{
    checkClasses();
    // inform each expression in which class it actually is
    setExpressionClasses();
    // set scope context (inform each scope which identifiers are defined)
    set_scope_context();
    // check inheritance cycles
    checkInheritanceCycles(classes);
    // check undefined identifiers in fields and methods
    checkUndefinedIdentifiers();
    checkCallsToUndefinedMethods();

    // check undefined types (classes)
    checkTypes(classesByName);

    // check main
    checkMain(classesByName);
}

void Program::set_scope_context()
{
    list<unique_ptr<Class>>::iterator a_class;
    for (a_class = classes.begin(); a_class != classes.end(); a_class++)
    {
        (*a_class)->set_scope_context(classesByName);
    }
}

void Program::checkCallsToUndefinedMethods()
{
    list<unique_ptr<Class>>::iterator a_class;
    for (a_class = classes.begin(); a_class != classes.end(); a_class++)
    {
        (*a_class)->checkCallsToUndefinedMethods(classesByName);
    }
}

void Program::setExpressionClasses()
{
    list<unique_ptr<Class>>::iterator a_class;
    for (a_class = classes.begin(); a_class != classes.end(); a_class++)
    {
        (*a_class)->setExpressionClasses(*((*a_class)->name));
    }
}

void Program::checkMain(map<string, unique_ptr<Class> *> classesByName)
{
    map<string, unique_ptr<Class> *>::iterator class_pair = classesByName.find("Main");
    if (class_pair != classesByName.end())
    {
        unique_ptr<Class> *mainClass = class_pair->second;
        (*mainClass)->checkMain();
    }
    else
    {
        printError("No main class", {1, 1});
    }
}

void Program::checkClasses()
{
    list<unique_ptr<Class>>::iterator it;
    for (it = classes.begin(); it != classes.end(); it++)
    {
        // check classes redefinitions
        // and build a map to find classes by name
        map<string, unique_ptr<Class> *>::iterator a_pair = classesByName.find(*((*it)->name));
        if (a_pair == classesByName.end())
        {
            classesByName.insert(make_pair(*((*it)->name), &(*it)));
        }
        else
        {
            printError("class " + *((*it)->name) + " redefined", (*it)->position);
        }

        // check redefined Fields and Methods (only, not all identifiers)
        // and add them in a map <name, type>
        (*it)->checkMethodsRedefinitions();
        (*it)->checkFieldsRedefinitions(classesByName);
    }
}

void Program::checkUndefinedIdentifiers()
{
    list<unique_ptr<Class>>::iterator a_class;
    for (a_class = classes.begin(); a_class != classes.end(); a_class++)
    {
        (*a_class)->checkUndefinedIdentifiers();
    }
}

void Program::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator a_class_pair;
    for (a_class_pair = classesByName.begin(); a_class_pair != classesByName.end(); a_class_pair++)
    {
        (*(a_class_pair->second))->checkTypes(classesByName);

        // Check overidden method and return type
        unique_ptr<Class> *child_class = a_class_pair->second;
        if ((*child_class)->parent != nullptr)
        {
            string parent_name = *((*child_class)->parent);

            // Get the parent class object
            map<string, unique_ptr<Class> *>::iterator parent_pair;
            parent_pair = classesByName.find(parent_name);
            if (parent_pair != classesByName.end())
                checkParentMethods(*(a_class_pair->second), *(parent_pair->second));
            else
                printError("use of undefined type " + parent_name, (*child_class)->position);
        }
    }
}

void Program::checkParentMethods(unique_ptr<Class> &childClass, unique_ptr<Class> &parentClass)
{
    list<unique_ptr<Method>>::iterator itChild;
    for (itChild = childClass->classBody->methods->methods.begin(); itChild != childClass->classBody->methods->methods.end(); itChild++)
    {
        unique_ptr<Class> *parent = &parentClass;
        while (!isMethodInClass(*itChild, *parent) && (*parent)->parent != nullptr)
        { // look in the parent class of the parent
            map<string, unique_ptr<Class> *>::iterator pair = classesByName.find(*((*parent)->parent));
            if (pair != classesByName.end())
            {
                parent = pair->second;
            }
            else
                break;
        }
    }
}

bool Program::isMethodInClass(unique_ptr<Method> &method, unique_ptr<Class> &class_ptr)
{
    list<unique_ptr<Method>>::iterator itParent;
    bool method_found = false;
    for (itParent = class_ptr->classBody->methods->methods.begin(); itParent != class_ptr->classBody->methods->methods.end(); itParent++)
    {
        // If the methods have the same name
        if (*(method->name) == *((*itParent)->name))
        {
            method_found = true;
            // If the return type is not the same
            if (*(method->returnType) != *((*itParent)->returnType))
            {
                printError(*(method->name) + " method has not the same type as in its parent class " + *(class_ptr->name), method->position);
            }

            // If the type of each argument is not the same
            if (method->formals->formals.size() == (*itParent)->formals->formals.size())
            {
                list<unique_ptr<Formal>>::iterator itArgChild;
                int i = 0;
                for (itArgChild = method->formals->formals.begin(); itArgChild != method->formals->formals.end(); itArgChild++)
                {
                    list<unique_ptr<Formal>>::iterator itArgParent;
                    itArgParent = (*itParent)->formals->formals.begin();
                    advance(itArgParent, i);
                    if (*((*itArgChild)->type) != *((*itArgParent)->type))
                    {
                        printError(*((*itArgChild)->name) + " argument of method " + *(method->name) + " has not the same type as in its parent class " + *(class_ptr->name), (*itArgChild)->position);
                    }
                    i++;
                }
            }
            else
            {
                printError(*(method->name) + " method has not the same number of argument as in its parent class " + *(class_ptr->name), method->position);
            }
            break;
        }
    }
    return method_found;
}

void Program::checkInheritanceCycles(list<unique_ptr<Class>> &classes)
{
    list<unique_ptr<Class>>::iterator a_class;
    for (a_class = classes.begin(); a_class != classes.end(); a_class++)
    {
        list<string> ancestors;
        string parentClass = hasInheritanceCycle(*((*a_class)->name), ancestors);
        if (parentClass.compare("") != 0)
        {
            printError("class " + *((*a_class)->name) + " cannot extend child class " + parentClass, (*a_class)->position);
        }
    }
}

string Program::hasInheritanceCycle(string class_name, list<string> ancestors)
{
    map<string, unique_ptr<Class> *>::iterator a_pair = classesByName.find(class_name);
    if (a_pair != classesByName.end())
    {
        unique_ptr<Class> *class_ptr = a_pair->second;
        if ((*class_ptr)->parent == nullptr)
        {
            return "";
        }
        else if (find(ancestors.begin(), ancestors.end(), *((*class_ptr)->parent)) != ancestors.end())
        {
            return *((*class_ptr)->parent);
        }
        ancestors.push_back(*((*class_ptr)->parent));
        return hasInheritanceCycle(*((*class_ptr)->parent), ancestors);
    }
    return "";
}

////////
// IF //
////////

string If::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "If(" + conditionExpr->toString(c, classesByName) + ", " + thenExpr->toString(c, classesByName);
    if (elseExpr != nullptr)
    {
        content += ", " + elseExpr->toString(c, classesByName);
    }
    content += ")";
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void If::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    conditionExpr->checkTypes(classesByName);
    string condType = conditionExpr->getType(classesByName);
    string thenType = thenExpr->getType(classesByName);
    if (condType != "bool")
    {
        printError("condition expression is of type " + condType + ", but should be of type bool", conditionExpr->position);
    }
    thenExpr->checkTypes(classesByName);
    if (elseExpr != nullptr)
    {
        elseExpr->checkTypes(classesByName);
        string elseType = elseExpr->getType(classesByName);
        if (!isPrimitive(thenType) && !isPrimitive(elseType))
        {
            list<string> thenParents = getInheritanceList(thenType, classesByName);
            list<string> elseParents = getInheritanceList(elseType, classesByName);
            string commonParent = getNearestCommonParent(thenParents, elseParents);
            if (commonParent == "")
            {
                printError("then expression and else expression have no compatible types", position);
            }
        }
        else if (thenType != elseType && thenType != "unit" && elseType != "unit")
        {
            printError("then expression (" + thenType + ") and else expression (" + elseType + ") have no compatible types", position);
        }
    }
}

void If::checkUndefinedIdentifiers()
{
    conditionExpr->checkUndefinedIdentifiers();
    thenExpr->checkUndefinedIdentifiers();
    if (elseExpr != nullptr)
    {
        elseExpr->checkUndefinedIdentifiers();
    }
}

void If::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    conditionExpr->checkCallsToUndefinedMethods(classesByName);
    thenExpr->checkCallsToUndefinedMethods(classesByName);
    if (elseExpr != nullptr)
    {
        elseExpr->checkCallsToUndefinedMethods(classesByName);
    }
}

void If::checkTypes(string name)
{
    class_name = name;
    conditionExpr->setExpressionClasses(name);
    thenExpr->setExpressionClasses(name);
    if (elseExpr != nullptr)
        elseExpr->setExpressionClasses(name);
}

void If::setExpressionClasses(string name)
{
    class_name = name;
    conditionExpr->setExpressionClasses(name);
    thenExpr->setExpressionClasses(name);
    if (elseExpr != nullptr)
        elseExpr->setExpressionClasses(name);
}

void If::set_scope_context(map<string, string> &identifiers)
{
    conditionExpr->set_scope_context(identifiers);
    thenExpr->set_scope_context(identifiers);
    if (elseExpr != nullptr)
        elseExpr->set_scope_context(identifiers);
}

string If::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    string type = "";
    string thenType = thenExpr->getType(classesByName);
    if (elseExpr != nullptr)
    {
        string elseType = elseExpr->getType(classesByName);
        if (!isPrimitive(thenType) && !isPrimitive(elseType))
        {
            list<string> thenParents = getInheritanceList(thenType, classesByName);
            list<string> elseParents = getInheritanceList(elseType, classesByName);
            string commonParent = getNearestCommonParent(thenParents, elseParents);
            if (commonParent == "")
            {
                return "";
            }
            else
            {
                return commonParent;
            }
        }
        if (thenType == "unit" || elseType == "unit")
            return "unit";
        else if (isPrimitive(thenType) && isPrimitive(elseType) && thenType == elseType)
        {
            return thenType;
        }
        else
        {
            return "";
        }
    }
    return "unit";
}

string If::getNearestCommonParent(list<string> parents1, list<string> parents2)
{
    list<string>::iterator it1;
    list<string>::iterator it2;
    for (it1 = parents1.begin(); it1 != parents1.end(); it1++)
    {
        for (it2 = parents2.begin(); it2 != parents2.end(); it2++)
        {
            if (*it1 == *it2)
            {
                return *it1;
            }
        }
    }
    return "";
}

list<string> If::getInheritanceList(string class_name, map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator it = classesByName.find(class_name);
    if (it != classesByName.end())
    {
        if (*((*(it->second))->name) == "Object")
        {
            list<string> l = {"Object"};
            return l;
        }
        else
        {
            list<string> ancestors = getInheritanceList(*((*(it->second))->parent), classesByName);
            ancestors.push_front(*((*(it->second))->name));
            return ancestors;
        }
    }
    else
    {
        return list<string>();
    }
}

llvm::Value *If::codegen(LLVM &ll)
{
    llvm::Value *CondV = conditionExpr->codegen();
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    CondV = ll.Builder.CreateFCmpONE(CondV, llvm::ConstantFP::get(ll.TheContext, llvm::APFloat(0.0)), "ifcond");

    llvm::Function *TheFunction = ll.Builder->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*ll.TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*ll.TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*ll.TheContext, "ifcont");

    ll.Builder.CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then value.
    ll.Builder.SetInsertPoint(ThenBB);

    llvm::Value *ThenV = thenExpr->codegen();
    if (!ThenV)
        return nullptr;

    ll.Builder.CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = ll.Builder->GetInsertBlock();

    // Emit else block.
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    llvm::Value *ElseV = elseExpr->codegen();
    if (!ElseV)
        return nullptr;

    ll.Builder->CreateBr(MergeBB);
    // codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = ll.Builder->GetInsertBlock();

    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    ll.Builder->SetInsertPoint(MergeBB);
    llvm::PHINode *PN = ll.Builder->CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}

//////////////
// INTEGERS //
//////////////

string IntegerExpression::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = to_string(value);
    if (c)
        content += " : int32";
    return content;
}

Value *IntegerExpression::codegen(LLVM &ll)
{
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(ll.TheContext)), *value);
}

/////////////
// STRINGS //
/////////////

string StringLitExpression::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = *value;
    if (c)
        content += " : string";
    return content;
}

llvm::Value *StringLitExpression::codegen(LLVM &ll){
    return ll.Builder->CreateGlobalStringPtr(*value, "str");
}

//////////
// BOOL //
//////////

string BooleanLitExpression::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = *value;
    if (c)
        content += " : bool";
    return content;
}

llvm::Value* BooleanLitExpression::codegen(LLVM &ll){
    return llvm::ConstantInt::get(toLLVMtype("bool"), value);
}

///////////
// WHILE //
///////////

string While::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "While(" + conditionExpr->toString(c, classesByName) + ", " + bodyExpr->toString(c, classesByName) + ")";
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void While::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    string condType = conditionExpr->getType(classesByName);
    if (condType != "bool")
    {
        printError("expected type was bool, but found " + condType, position);
    }
    conditionExpr->checkTypes(classesByName);
    bodyExpr->checkTypes(classesByName);
}

void While::checkUndefinedIdentifiers()
{
    conditionExpr->checkUndefinedIdentifiers();
    bodyExpr->checkUndefinedIdentifiers();
}

void While::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    conditionExpr->checkCallsToUndefinedMethods(classesByName);
    bodyExpr->checkCallsToUndefinedMethods(classesByName);
}

void While::setExpressionClasses(string name)
{
    class_name = name;
    conditionExpr->setExpressionClasses(name);
    bodyExpr->setExpressionClasses(name);
}

void While::set_scope_context(map<string, string> &identifiers)
{
    conditionExpr->set_scope_context(identifiers);
    bodyExpr->set_scope_context(identifiers);
}

string While::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    return "unit";
}

////////////
// ASSIGN //
////////////

string Assign::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "Assign(" + *name + ", " + expr->toString(c, classesByName) + ")";
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void Assign::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    // check both types match
    string id_type = scope_identifiers.find(*name)->second;
    string expr_type = expr->getType(classesByName);
    if (id_type != expr_type)
    {
        printError("cannot assign type " + expr_type + " to type " + id_type, expr->position);
    }
    expr->checkTypes(classesByName);
}

void Assign::checkUndefinedIdentifiers()
{
    // search name in fields
    bool found = (scope_identifiers.find(*name) != scope_identifiers.end());
    if (!found)
    {
        printError("field " + *name + " undefined", position);
    }
    // check in the expression
    expr->checkUndefinedIdentifiers();
}

void Assign::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    expr->checkCallsToUndefinedMethods(classesByName);
}

string Assign::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    return expr->getType(classesByName);
}

void Assign::setExpressionClasses(string name)
{
    class_name = name;
    expr->setExpressionClasses(name);
}

void Assign::set_scope_context(map<string, string> &identifiers)
{
    scope_identifiers = identifiers;
    expr->set_scope_context(identifiers);
}

///////////
// BINOP //
///////////

string BinOp::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "BinOp(" + *op + ", " + left->toString(c, classesByName) + ", " + right->toString(c, classesByName) + ")";
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void BinOp::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    string leftType = left->getType(classesByName);
    string rightType = right->getType(classesByName);
    if (leftType != rightType)
    {
        printError("cannot compare a value of type " + leftType + " and a value of type " + rightType, position);
    }
    left->checkTypes(classesByName);
    right->checkTypes(classesByName);
}

void BinOp::checkUndefinedIdentifiers()
{
    left->checkUndefinedIdentifiers();
    right->checkUndefinedIdentifiers();
}

void BinOp::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    left->checkCallsToUndefinedMethods(classesByName);
    right->checkCallsToUndefinedMethods(classesByName);
}

string BinOp::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    if (op->compare("<=") == 0 || op->compare("<") == 0 || op->compare("=") == 0 || op->compare("and") == 0)
    {
        return "bool";
    }
    return "int32";
}

void BinOp::setExpressionClasses(string name)
{
    class_name = name;
    left->setExpressionClasses(name);
    right->setExpressionClasses(name);
}

void BinOp::set_scope_context(map<string, string> &identifiers)
{
    left->set_scope_context(identifiers);
    right->set_scope_context(identifiers);
}

Value *BinOp::codegen(LLVM &ll)
{
    Value *leftVal = left->codegen();
    Value *rightVal = right->codegen();
    if (!leftVal || !rightVal)
        return nullptr;

    switch (op)
    {
    // TODO: AND
    case PLUS:
        return ll.Builder.CreateFAdd(leftVal, rightVal, "addtmp");
    case MINUS:
        return ll.Builder.CreateFSub(leftVal, rightVal, "subtmp");
    case TIMES:
        return ll.Builder.CreateFMul(leftVal, rightVal, "multmp");
    case DIV:
        return ll.Builder.CreateFDiv(leftVal, rightVal, "divtmp");
    case LOWER:
        return ll.Builder.CreateFCmpULT(leftVal, rightVal, "lttmp");
    case LOWER_EQUAL:
        return ll.Builder.CreateFCmpULE(leftVal, rightVal, "letmp");
    case EQUAL:
        return ll.Builder.CreateFCmpUEQ(leftVal, rightVal, "eqtmp");
    default:
        return nullptr;
    }
}

//////////
// ARGS //
//////////

void Args::addCallArgument(unique_ptr<Expression> arg)
{
    args.push_front(move(arg));
}

string Args::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    list<unique_ptr<Expression>>::iterator args_it;
    string content = "[";
    for (args_it = args.begin(); args_it != args.end(); args_it++)
    {
        content += (*args_it)->toString(c, classesByName) + ", ";
    }
    if (content.length() > 1 && content.substr(content.length() - 2) == ", ")
    {
        content = content.substr(0, content.length() - 2);
    }
    return content + "]";
}

void Args::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    // check each arg
    list<unique_ptr<Expression>>::iterator args_it;
    for (args_it = args.begin(); args_it != args.end(); args_it++)
    {
        (*args_it)->checkTypes(classesByName);
    }
}

void Args::checkUndefinedIdentifiers()
{
    // check in the expressions
    list<unique_ptr<Expression>>::iterator args_it;
    for (args_it = args.begin(); args_it != args.end(); args_it++)
    {
        (*args_it)->checkUndefinedIdentifiers();
    }
}

void Args::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    // getType of objExpr and verify the type has the method
    list<unique_ptr<Expression>>::iterator args_it;
    for (args_it = args.begin(); args_it != args.end(); args_it++)
    {
        (*args_it)->checkCallsToUndefinedMethods(classesByName);
    }
}

string Args::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    return "";
}

void Args::setExpressionClasses(string name)
{
    class_name = name;
    list<unique_ptr<Expression>>::iterator args_it;
    for (args_it = args.begin(); args_it != args.end(); args_it++)
    {
        (*args_it)->setExpressionClasses(name);
    }
}

void Args::set_scope_context(map<string, string> &identifiers)
{
    list<unique_ptr<Expression>>::iterator args_it;
    for (args_it = args.begin(); args_it != args.end(); args_it++)
    {
        (*args_it)->set_scope_context(identifiers);
    }
}

//////////
// CALL //
//////////

llvm::Value *Call::codegen(LLVM &ll, unique_ptr<Class> * > classesByName)
{
    string objType;
    if (objExpr != nullptr)
    {
        objExpr.codegen(ll, classesByName);
        objType = objExpr.getType(classesByName);
    }
    else
    {
        objType = class_name;
    }
    args.codegen(ll, classesByName);
    Class *c;
    unique_ptr<Method> *method;
llvm:
    Function *func;
llvm:
    Value *object;
    if (isUnit(objType))
    { // If the object has type unit (or is null)
        map<string, unique_ptr<Method> *>::iterator = functionsByName.find(methodName);
        if (iterator != functionsByName.end())
        {
            method = *iterator->second;
            func = method->getFunction(ll);
        }
        else
        { // if we call a function of self
            object = Self().codegen();
        }
    }
    else if (isClass(objType))
    { // If we call a function of another class
        // Get its value
        object = objExpr->getValue();
        // Find the class Object
        map<string, unique_ptr<Class> *>::classIterator = classesByName.find(objType);
        if (classIterator != classesByName.end())
        {
            unique_ptr<Class> *cl = *(classIterator->second);
            // Get all the methods of the class
            list<unique_ptr<Method>> cl_methods = cl->classBody->methods->methods;
            // Find the desired one
            list<unique_ptr<Method>>::methIterator;
            for (methIterator = cl_methods.begin(); methIterator != cl_methods.end(); methIterator++)
            {
                if (*((*methIterator)->name) == methodName)
                {
                    method = methIterator;
                    break;
                }
            }
            func = method->getFunction(ll);
        }
    }
    if (func)
    {
        std::vector<Value *> ArgsValues;
        list<unique_ptr<Expression>>::iterator argsIterator;
        for (argsIterator = args->args.begin(); argsIterator != args->args.end(); argsIterator++)
        {
            ArgsValues.push_back((*argsIterator)->codegen());
            if (!ArgsValues.back())
                return nullptr;
        }
        return ll.Builder->CreateCall(func, ArgsValues, methodName);
    }
}

string Call::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string obj = "self";
    if (c)
        obj += " : " + class_name;
    if (objExpr != nullptr)
    {
        obj = objExpr->toString(c, classesByName);
    }
    string content = "Call(" + obj + ", " + *methodName + ", " + args->toString(c, classesByName) + ")";
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void Call::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    if (objExpr != nullptr)
    {
        objExpr->checkTypes(classesByName);
    }
    // check each arg
    args->checkTypes(classesByName);
}

void Call::checkUndefinedIdentifiers()
{
    if (objExpr == nullptr && scope_context.find("self") == scope_context.end())
    {
        printError("cannot use self in field initializer", position);
    }
    // check in the expression
    if (objExpr != nullptr)
    {
        objExpr->checkUndefinedIdentifiers();
    }
    args->checkUndefinedIdentifiers();
}

void Call::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    if (objExpr == nullptr && scope_context.find("self") == scope_context.end())
    {
        printError("cannot find method " + *methodName + " in type <invalid-type>", position);
    }
    if (objExpr != nullptr)
        objExpr->checkCallsToUndefinedMethods(classesByName);
    // getType of objExpr and verify the type has the method
    checkMethodReturnTypeAndArgs(classesByName);
    args->checkCallsToUndefinedMethods(classesByName);
}

void Call::checkMethodReturnTypeAndArgs(map<string, unique_ptr<Class> *> &classesByName)
{
    unique_ptr<Class> *class_ptr;
    if (objExpr == nullptr)
    {
        map<string, unique_ptr<Class> *>::iterator self_class = classesByName.find(class_name);
        class_ptr = self_class->second;
    }
    else
    {
        string objExprType = objExpr->getType(classesByName);
        if (objExprType == "")
            return;
        if (isPrimitive(objExprType))
        {
            printError("type " + objExpr->getType(classesByName) + " has no method called " + *methodName, position);
            return;
        }
        else
        {
            map<string, unique_ptr<Class> *>::iterator a_class = classesByName.find(objExprType);
            class_ptr = a_class->second;
        }
    }
    string type = getMethodType(*class_ptr, classesByName, *methodName);
    if (type.compare("") == 0)
    {
        printError("type " + class_name + " has no method called " + *methodName, position);
    }
    checkMethodFormals(*class_ptr, classesByName, *methodName, args);
}

string Call::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    // if we call a method of "class_name"
    if (objExpr == nullptr)
    {
        map<string, unique_ptr<Class> *>::iterator self_class = classesByName.find(class_name);
        unique_ptr<Class> *class_ptr = self_class->second;
        string type = getMethodType(*class_ptr, classesByName, *methodName);
        if (type.compare("") == 0)
        {
            return "";
        }
        return type;
    }
    string objExprType = objExpr->getType(classesByName);
    // get methods of this class
    if (!isPrimitive(objExprType))
    {
        map<string, unique_ptr<Class> *>::iterator a_class = classesByName.find(objExprType);
        if (a_class != classesByName.end())
        {
            unique_ptr<Class> *class_ptr = a_class->second;
            string type = getMethodType(*class_ptr, classesByName, *methodName);
            if (type.compare("") == 0)
            {
                return "";
            }
            return type;
        }
    }
    return "";
}

void Call::checkMethodFormals(unique_ptr<Class> &current_class, map<string, unique_ptr<Class> *> &classesByName, string methodName, unique_ptr<Args> &args)
{
    list<unique_ptr<Method>> &methods = current_class->classBody->methods->methods;
    list<unique_ptr<Method>>::iterator method_it;
    // find the method
    for (method_it = methods.begin(); method_it != methods.end(); method_it++)
    {
        if (*((*method_it)->name) == methodName)
        {
            // check nb of formals of the method
            if ((*method_it)->formals->formals.size() != args->args.size())
            {
                printError("call to method " + methodName + " of class " + *(current_class->name) + " does not match the required number of formal arguments", position);
                return;
            }
            // check types of formals of the method
            list<unique_ptr<Formal>>::iterator formal_it = (*method_it)->formals->formals.begin();
            list<unique_ptr<Expression>>::iterator args_it = args->args.begin();
            while (formal_it != (*method_it)->formals->formals.end())
            {
                string arg_type = (*args_it)->getType(classesByName);
                if (*((*formal_it)->type) != arg_type && !Utils::inheritsFrom(arg_type, *((*formal_it)->type), classesByName))
                {
                    printError("cannot assign type " + arg_type + " to formal argument " + *((*formal_it)->name) + " : " + *((*formal_it)->type) + " of method " + methodName, (*args_it)->position);
                }
                advance(args_it, 1);
                advance(formal_it, 1);
            }
        }
    }
    if (current_class->parent == nullptr)
    {
        return "";
    }
    map<string, unique_ptr<Class> *>::iterator parent_pair = classesByName.find(*(current_class->parent));
    return checkMethodFormals(*(parent_pair->second), classesByName, methodName, args);
}

string Call::getMethodType(unique_ptr<Class> &current_class, map<string, unique_ptr<Class> *> &classesByName, string methodName)
{
    map<string, string> methods_types = current_class->getMethodsAndTypes();
    map<string, string>::iterator method_type = methods_types.find(methodName);
    if (method_type != methods_types.end())
    {
        // get return type of the corresponding method
        return method_type->second;
    }
    else
    {
        if (current_class->parent == nullptr)
        {
            return "";
        }
        map<string, unique_ptr<Class> *>::iterator parent_pair = classesByName.find(*(current_class->parent));
        return getMethodType(*(parent_pair->second), classesByName, methodName);
    }
}

void Call::setExpressionClasses(string name)
{
    class_name = name;
    args->setExpressionClasses(name);
    if (objExpr != nullptr)
    {
        objExpr->setExpressionClasses(name);
    }
}

void Call::set_scope_context(map<string, string> &identifiers)
{
    scope_context = identifiers;
    if (objExpr != nullptr)
    {
        objExpr->set_scope_context(scope_context);
    }
    args->set_scope_context(scope_context);
}

/////////
// LET //
/////////

string Let::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string firstPart = "Let(" + *name + ", " + *type + ", ";
    string lastPart = "";
    if (init != nullptr)
    {
        lastPart += init->toString(c, classesByName);
        lastPart += ", ";
    }
    lastPart += scope->toString(c, classesByName) + ")";
    if (c)
        lastPart += " : " + getType(classesByName);
    return firstPart + lastPart;
}

void Let::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator a_pair;
    a_pair = classesByName.find(*type);
    if (isupper((*type)[0]) && a_pair == classesByName.end())
    {
        // not found
        printError("type " + *type + " not defined", position);
    }
    if (init != nullptr)
    {
        init->checkTypes(classesByName);
        string initType = init->getType(classesByName);
        if (initType != *type && !Utils::inheritsFrom(initType, *type, classesByName))
        {
            printError("expected type " + *type + ", but found type " + initType, init->position);
        }
    }
    scope->checkTypes(classesByName);
}

void Let::checkUndefinedIdentifiers()
{
    // check the init expression
    if (init != nullptr)
    {
        init->checkUndefinedIdentifiers();
    }
    // check in the scope
    scope->checkUndefinedIdentifiers();
}

void Let::checkCallsToUndefinedMethods(map<string, unique_ptr<Class> *> &classesByName)
{
    if (init != nullptr)
        init->checkCallsToUndefinedMethods(classesByName);
    scope->checkCallsToUndefinedMethods(classesByName);
}

string Let::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    return scope->getType(classesByName);
}

void Let::setExpressionClasses(string name)
{
    class_name = name;
    scope->setExpressionClasses(name);
    if (init != nullptr)
    {
        init->setExpressionClasses(name);
    }
}

void Let::set_scope_context(map<string, string> &identifiers)
{
    if (init != nullptr)
    {
        init->set_scope_context(identifiers);
    }
    // save the variable in the scope identifiers
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

llvm::Value *Let::codegen(LLVM &ll){
    llvm::Type* type = ll.toLLVMType(*type):
    // Push variable 
    scope->codegen(ll);
    // Pop variable
    return scope->getValue();
}

/////////
// NEW //
/////////

string New::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "New(" + *type + ")";
    if (c)
        content += " : " + *type;
    return content;
}

void New::checkTypes(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, unique_ptr<Class> *>::iterator a_pair;
    a_pair = classesByName.find(*type);
    if (isupper((*type)[0]) && a_pair == classesByName.end())
    {
        // not found
        printError("type " + *type + " not defined", position);
    }
}

llvm::Value* New::codegen(LLVM &ll){
    llvm::Function* func = ll.TheModule->getFunction(*type);
    return ll.builder->CreateCall(func, {})
}

///////////////////////
// OBJECT IDENTIFIER //
///////////////////////

string ObjectIdentifier::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = *name;
    if (c)
        content += " : " + getType(classesByName);
    return content;
}

void ObjectIdentifier::checkUndefinedIdentifiers()
{
    map<string, string>::iterator it = scope_identifiers.find(*name);
    if (it == scope_identifiers.end())
    {
        printError("object-identifier " + *name + " is undefined", position);
    }
}

string ObjectIdentifier::getType(map<string, unique_ptr<Class> *> &classesByName)
{
    map<string, string>::iterator it = scope_identifiers.find(*name);
    if (it != scope_identifiers.end())
    {
        return it->second;
    }
    return "";
}

Value *ObjectIdentifier::codegen(LLVM &ll)
{
    if (scope_identifiers.find(*name) != scope_identifiers.end())
    {
        return nullptr; // TODO: change this to get the last assignment
    }
    return nullptr;
}

//////////
// UNIT //
//////////

string UnitExpression::toString(bool c, map<string, unique_ptr<Class> *> classesByName)
{
    string content = "()";
    if (c)
        content += " : unit";
    return content;
}
