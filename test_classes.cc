#include <iostream>
#include <memory>
#include "tree.hh"
#include <utility>
using namespace std;

class A {
    public:
        A(){}
        virtual ~A(){}
        virtual string toStr() = 0;
};

class B: public A {
    public:
        B(){}
        string toStr() {
            return "B";
        }
};

int main(int argc, char **argv){
    
    New init = New("init");
    New scope = New("scope");
    Let c = Let("name", "type", init, scope);
    cout << c.toString();
    
    
    
}