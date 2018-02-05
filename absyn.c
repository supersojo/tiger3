#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "absyn.h"

namespace tiger{

char* Exp::ExpKindStrings[]={
        "kExp_Var",
        "kExp_Nil",
        "kExp_Int",
        "kExp_String",
        "kExp_Call",
        "kExp_Op",
        "kExp_Record",
        "kExp_Seq",
        "kExp_Assign",
        "kExp_If",
        "kExp_While",
        "kExp_Break",
        "kExp_For",
        "kExp_Let",
        "kExp_Array",
        "kExp_Invalid"
};

SubscriptVar::~SubscriptVar(){
        delete m_var;
        delete m_exp;
}
SubscriptVar* SubscriptVar::Clone(){
    SubscriptVar* n = new SubscriptVar;
    n->m_var = m_var?m_var->Clone():0;
    n->m_exp = m_exp?m_exp->Clone():0;
    return n;
}
RecordExp::~RecordExp(){
        delete m_type;
        delete m_fields;
}
RecordExp* RecordExp::Clone(){
    RecordExp* n = new RecordExp;
    n->m_type = m_type?m_type->Clone():0;
    n->m_fields = m_fields?m_fields->Clone():0;
    return n;
}
LetExp::~LetExp(){
        delete m_decs;
        delete m_body;
}
LetExp* LetExp::Clone(){
    LetExp* n = new LetExp;
    n->m_decs = m_decs?m_decs->Clone():0;
    n->m_body = m_body?m_body->Clone():0;
    return n;
}

FunDec::~FunDec(){
        delete m_name;
        delete m_params;
        delete m_result;
        delete m_body;
}
FunDec* FunDec::Clone(){
    FunDec* n = new FunDec;
    n->m_name = m_name?m_name->Clone():0;
    n->m_params = m_params?m_params->Clone():0;
    n->m_result = m_result?m_result->Clone():0;
    n->m_body = m_body?m_body->Clone():0;
    return n;
}

TypeDec::~TypeDec(){
    delete m_nametylist;
}
TypeDec* TypeDec::Clone(){
    TypeDec* n = new TypeDec;
    n->m_nametylist = m_nametylist?m_nametylist->Clone():0;
    return n;
}

RecordTy::~RecordTy(){
        delete m_list;
}
RecordTy* RecordTy::Clone(){
    RecordTy* n = new RecordTy;
    n->m_list = m_list?m_list->Clone():0;
    return n;
}
    
}//namespace tiger