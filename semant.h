/* Coding: ANSI */
#ifndef SEMANT_H
#define SEMANT_H

#include "tiger_type.h"
#include "tiger_log.h"
#include "types.h"// symbol table 
#include "absyn.h"// sbstract syntax tree
#include "tree.h"//immediate representation tree
#include "frame.h"

namespace tiger{

//used for type check
class ExpBaseTy{
public:
    ExpBaseTy(){m_type=0;m_tree=0;}
    ExpBaseTy(TypeBase* ty,ExpBase* tree){m_type=ty;m_tree = tree;}
    TypeBase* Type(){return m_type;}
    ExpBase*  Tree(){return m_tree;}
    ~ExpBaseTy(){
        //delete m_type;
        delete m_tree;
    }
private:
    TypeBase* m_type;
    ExpBase*  m_tree;
};

class Translator{
public:
    Translator();
    ExpBaseTy*  TransExp(SymTab* venv,SymTab* tenv,Exp* exp);
    ExpBaseTy*  TransVar(SymTab* venv,SymTab* tenv,Var* var);
    void        TransDec(SymTab* venv,SymTab* tenv,Dec* dec);
    TypeBase*   TransTy(SymTab* tenv,Ty* ty);
    ~Translator();
private:
    void           TransFunctionDec(SymTab* venv,SymTab* tenv,Dec* dec);
    TypeFieldList* MakeFormalsList(SymTab* venv,SymTab* tenv,FieldList* params);
    void           TransTypeDec(SymTab* venv,SymTab* tenv,Dec* dec);
    FrameBase*     MakeNewFrame(FunDec* fundec);
    
    LoggerStdio m_logger; 
};

}//namespace tiger

#endif
