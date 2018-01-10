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

RecordExp::~RecordExp(){
        delete m_type;
        delete m_fields;
}

LetExp::~LetExp(){
        delete m_decs;
        delete m_body;
}

FunDec::~FunDec(){
        delete m_name;
        delete m_params;
        delete m_result;
        delete m_body;
}

TypeDec::~TypeDec(){
    delete m_nametylist;
}

RecordTy::~RecordTy(){
        delete m_list;
}
    
}//namespace tiger