/* Coding: Ansi */
#ifndef TYPE_CHECK_H
#define TYPE_CHECK_H

#include "tiger_type.h"
#include "tiger_log.h"
#include "types.h"

namespace tiger{
    
struct TypeCheckResult{
    TypeCheckResult(){
        m_type = 0;
    }
    TypeCheckResult(TypeBase* t){
        m_type = t;
    }
    ~TypeCheckResult(){
    }
    TypeBase* Type(){return m_type;}
    TypeBase* m_type;
};
class TypeChecker{
public:
    TypeChecker(){
        m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
        m_logger.SetModule("type check");
    }
    TypeCheckResult* TypeCheck(SymTab* venv,SymTab* tenv,Exp* exp){
        return TypeCheckExp(venv, tenv, exp);
    }
private:
    TypeCheckResult* TypeCheckExp(SymTab* venv,SymTab* tenv,Exp* exp);
    TypeCheckResult* TypeCheckVar(SymTab* venv,SymTab* tenv,Var* var);
    void             TypeCheckDec(SymTab* venv,SymTab* tenv,Dec* dec);
    TypeCheckResult* TypeCheckTy(SymTab* venv,SymTab* tenv,Ty* ty);
    TypeFieldList*   MakeFormalsList(SymTab* venv,SymTab* tenv,FieldList* params);
    void             TypeCheckFunctionDec(SymTab* venv,SymTab* tenv,Dec* dec);
    void             TypeCheckTypeDec(SymTab* venv,SymTab* tenv,Dec* dec);
    TypeBase*        TypeCheckTy(SymTab* tenv,Ty* ty);
    
    LoggerStdio m_logger; // log util
};    
    
    
}//namespace tiger


#endif
