#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "absyn.h"

namespace tiger{

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