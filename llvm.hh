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


using namespace std;
using namespace llvm;

class LLVM {
    public:
        LLVM(){
			TheContext = std::make_unique<llvm::LLVMContext>();
            TheModule = std::make_unique<llvm::Module>("TEST", *TheContext);
            Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
		}
        unique_ptr<llvm::LLVMContext> TheContext;
        unique_ptr<llvm::IRBuilder<>> Builder;
        unique_ptr<llvm::Module> TheModule;

        // On doit en premier lieu crééer un dictionnaire basé sur notre AST (alias Program).
        // Ensuite, on peut se baser sur notre ast et le dico pour créer le code IR.
};