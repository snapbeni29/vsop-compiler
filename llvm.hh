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


        llvm::Value* getDefaultValue(llvm::Type* type) {
			if (type->getPointerElementType()->isIntegerTy(8))
				return Builder->CreateGlobalStringPtr("");
            if (type->isVoidTy() || type->isIntegerTy(32) || type->isIntegerTy(1))
				return llvm::Constant::getNullValue(type);
			if (type->getPointerElementType()->isStructTy())
				return llvm::ConstantPointerNull::get((llvm::PointerType*) type);
			return nullptr;
		}
        

        static bool compareTypes(llvm:Type* type1, llvm::Type* type2){
            if (type1->isVoidTy()) // unit
                return type2->isVoidTy();
            if (type1->isIntegerTy(32)) // int32
                return type2->isIntegerTy(32);
            if (type1->isIntegerTy(1)) // bool
                return type2->isIntegerTy(1);
            if (type1->getPointerElementType()->isIntegerTy(8)) // string
                return type2->getPointerElementType()->isIntegerTy(8);
            if (type1->getPointerElementType()->isStructTy()) // class
                type2->getPointerElementType()->isStructTy();
            return nullptr;
        }

        static std::string toVSOPType(llvm::Type* type) {
            if (type->isIntegerTy(32))
                return "int32";
            if (type->isIntegerTy(1))
                return "bool";
            if (type->getPointerElementType()->isIntegerTy(8))
                return "string";
            if (type->getPointerElementType()->isStructTy()) // class
                std::string str = type->getPointerElementType()->getStructName().str();
                return str.substr(str.find_last_of('.') + 1);
            return "";
        }

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
                declared_identifiers[name].pop_back();
            }
            if (declared_identifiers[name].size() == 0) {
                declared_identifiers.erase(name);
            }
        }

        llvm::Value* find(std:string name) {
            std::vector<llvm::Value*> v = declared_identifiers[name];
            if (v) {
                return v.back();
            }
            return nullptr;
        }
};