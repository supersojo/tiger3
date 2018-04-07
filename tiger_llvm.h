#ifndef TIGER_LLVM_H
#define TIGER_LLVM_H

#include "absyn.h"
#include "tiger_assert.h"
#include "types.h"
#include "symtab.h"

#include <list>

#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include "irgencontext.h"
/*
 generate llvm ir for tiger's AST
 
 INPUT: tiger's AST tree
 OUTPUT:
    LLVM IR
 NOTES:
    escape - 
    type - 
    symbol table -
    
 --------------------------    
 let
 function foo()=
    let
       var a:=10 // escaped 
       function bar()=//after compile var,we knows all escaped vars 
       let
       a=10//escaped
       in
       end
    in
    bar()
    end
 in
 end
*/
namespace tiger{

class EnvEntryVarLLVM:public EnvEntryBase
{
public:
    enum{
        kEnvEntryVarLLVM_For_Type,/* used in type environment*/
        kEnvEntryVarLLVM_For_Value,/* value environment*/
        kEnvEntryVarLLVM_Invalid/* invalid */
    };
    EnvEntryVarLLVM():EnvEntryBase(kEnvEntry_Var){
        m_type = 0;
        m_intent = kEnvEntryVarLLVM_Invalid;
        m_llvm_type = 0;
    }
    EnvEntryVarLLVM(TypeBase* ty,s32 intent,llvm::Type* llvm_type,llvm::Value* v):EnvEntryBase(kEnvEntry_Var){
        m_type = ty;
        m_intent = intent;
        m_llvm_type = llvm_type;
        m_value = v;
    }
    s32 Intent(){return m_intent;}
    llvm::Type* GetLLVMType(){return m_llvm_type;}
    llvm::Value* GetLLVMValue(){return m_value;}
    TypeBase* Type(){return m_type;}
    void      Update(TypeBase* n){
        TypeName*p = dynamic_cast<TypeName*>(m_type);
        p->Update(n);
        //construct structtype of llvm
        std::vector<llvm::Type*> vv;
        //vv.insert()
        //m_llvm_type = llvm::StructType(array);

    }
    ~EnvEntryVarLLVM(){
        if(m_intent==kEnvEntryVarLLVM_For_Type){
            delete m_type;
        }
        if(m_intent==kEnvEntryVarLLVM_For_Value){
            //delete m_access;
        }
    }
private:
    TypeBase* m_type;//
    s32 m_intent;
    
    /*
    if type can associate llvm type then record iter_swap
    int -- Type::getInt32Ty(ctx)
    string -- PointerType::get(Type::getInt8Ty(ctx))
    */
    llvm::Type* m_llvm_type;
    llvm::Value* m_value;
    LoggerStdio m_logger;
};
/*
    function foo(){//escape list: 
    
        var a = 1
        var b = 2
        function bar(){ //escape list: a->b
            a = 2 // get index in escape list, 
            b = 3 // get index in escape list
        }
        
        bar()
    }
*/
class EnvEntryFunLLVM:public EnvEntryBase
{
public:
    enum{
        kFunction_Internal,
        kFunction_External,
        kFunction_Invalid
    };
    EnvEntryFunLLVM():EnvEntryBase(kEnvEntry_Fun){m_formals=0;m_result=0;m_level=0;m_label=0;m_fun_kind = kFunction_Internal;}
    EnvEntryFunLLVM(TypeFieldList *formals,TypeBase* result,Level* level,Label* label,s32 kind):EnvEntryBase(kEnvEntry_Fun){m_formals=formals;m_result=result;m_level=level;m_label=label;m_fun_kind = kind;}
    TypeBase* Type(){return m_result;}
    void SetType(TypeBase* ty){m_result = ty;}
    TypeFieldList* GetList(){return m_formals;}
    Level* GetLevel(){return m_level;}
    void   SetLevel(Level* lev){
        m_level = lev;
    }
    s32 FunKind(){return m_fun_kind;}
    std::list<Symbol*>& EscapeList(){return m_escape_list;}
    Label* GetLabel(){return m_label;}
    ~EnvEntryFunLLVM(){
        if(m_formals)
            delete m_formals;
        //delete m_result;
        //m_escape_list.erase();
    }
private:
    TypeFieldList* m_formals;// parm info 
    TypeBase* m_result;/* memory managed by tenv table */
    Level* m_level;// managed by level manager
    Label*     m_label;/*managed by label pool*/
    s32 m_fun_kind;
    std::list<Symbol*> m_escape_list;//for escape
};

class IRGen{
public:
    IRGen(){
        m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
        m_logger.SetModule("ir gen");
        m_top_level = 0;
        Init();
    }
    Level* OutmostLevel();
    void Dump(){
        m_context->M()->dump();
    }
    void Gen(SymTab* venv,SymTab* tenv,Exp* e){
        IRGenExp(venv,tenv,OutmostLevel(),e,0);
        m_context->B()->CreateRetVoid();
    }
    IRGenContext* GetContext(){return m_context;}
    llvm::Value* IRGenExp(SymTab* venv,SymTab* tenv,Level* level,Exp* e,llvm::BasicBlock* dest_bb);
    llvm::Value* IRGenExpLet(SymTab* venv,SymTab* tenv,Level* level,Exp* e,llvm::BasicBlock* dest_bb);
    void         IRGenDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,llvm::BasicBlock* dest_bb);
    llvm::Value* IRGenVar(SymTab* venv,SymTab* tenv,Level* level,Var* var,llvm::BasicBlock* dest_bb);
    TypeBase* IRGenTy(SymTab* tenv,Level* level,Ty* ty);
    void IRGenTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec);
private:
    IRGenContext* m_context;
    
    void Init();
    Level* m_top_level;
    LoggerStdio m_logger;
    
};


}//namespace tiger


#endif
