#include <iostream>
#include <list> 
#include <string>
#include <iterator>
#include <memory>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

class LLVM {
    public:
        LLVM(){
			TheContext = std::make_unique<llvm::LLVMContext>();
            TheModule = std::make_unique<llvm::Module>("vsop", *TheContext);
            Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
		}
        std::unique_ptr<llvm::LLVMContext> TheContext;
        std::unique_ptr<llvm::IRBuilder<>> Builder;
        std::unique_ptr<llvm::Module> TheModule;
        std::map<std::string, std::vector<llvm::Value*>> declared_identifiers;

        llvm::Type* toLLVMType(std::string type) {
            switch (type){
                case "int32":
                    return llvm::Type::getInt32Ty(*TheContext);
                case "bool":
                    return llvm::Type::getInt1Ty(*TheContext);
                case "string":
                    return llvm::Type::getInt8PtrTy(*TheContext);
        }

        void pushValue(std::string name, llvm::Value* val) {
            if (declared_identifiers[name]) {
                declared_identifiers[name].push_back(val);
            } else {
                std::vector<llvm::Value*> vals;
                vals.push_back(val);
                declared_identifiers[name] = vals;
            }
        }

        void popValue(std::string name) {
            if (declared_identifiers[name]) {
                declared_identifiers[name].pop_back(val);
            }
            if (declared_identifiers[name].size() == 0) {
                declared_identifiers.erase(name);
            }
        }
};