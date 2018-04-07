/* Coding: ANSI */
#ifndef ESCAPE_H
#define ESCAPE_H

#include "tiger_type.h"
#include "absyn.h"
#include "types.h"
#include "symtab.h"
#include "tiger_log.h"

namespace tiger{

class EscapeHelper{
public:
    EscapeHelper(){
        m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
        m_logger.SetModule("escape");
    }
    void FindEscape(Exp* exp);
private:
    void TransExp(SymTab* venv,s32 depth,Exp* exp);
    void TransDec(SymTab* venv,s32 depth,Dec* dec);
    void TransVar(SymTab* venv,s32 depth,Var* var);
    void TransFunctionDec(SymTab* venv, s32 depth, Dec* dec);

    LoggerStdio m_logger;
};



}//namespace tiger


#endif
