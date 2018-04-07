#ifndef IRGENCONTEXT_H
#define IRGENCONTEXT_H

#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>

namespace tiger
{
class IRGenContext{
public:
    
    static IRGenContext* Get(){
        if(m_context==nullptr){
            llvm::LLVMContext* c = new llvm::LLVMContext;
            llvm::IRBuilder<>* b = new llvm::IRBuilder<>(*c);
            llvm::Module* m = new llvm::Module("theModule",*c);
            m_context = new IRGenContext(c,b,m);
        }
        return m_context;
        
    }
    llvm::LLVMContext* C()const{return m_llvm_context;}
    llvm::IRBuilder<>* B()const{return m_builder;}
    llvm::Module*      M()const{return m_module;}
private:
    static IRGenContext* m_context;
    llvm::LLVMContext* m_llvm_context;
    llvm::IRBuilder<>* m_builder;
    llvm::Module*      m_module;
    
    IRGenContext(llvm::LLVMContext* c,llvm::IRBuilder<>* b,llvm::Module* m){
        m_llvm_context = c;
        m_builder = b;
        m_module = m;
    }
};
}

#endif
