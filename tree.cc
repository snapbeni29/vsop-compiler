#include <iostream>
#include <list> 
#include <string>
#include <iterator>
#include <memory>
#include <map>
#include <algorithm>
#include "tree.hh"

using namespace std;

bool Utils::inheritsFrom(string child_class, string parent_class, map<string, unique_ptr<Class> *> & classesByName) {
    map<string, unique_ptr<Class> *>::iterator it = classesByName.find(child_class);
    if (it != classesByName.end()) {
        if ((*(it->second))->parent != nullptr) {
            if (*((*(it->second))->parent) == parent_class)
                return true;
            else
                return Utils::inheritsFrom(*((*(it->second))->parent), parent_class, classesByName);
        } else return false;
    }
    return false;
}