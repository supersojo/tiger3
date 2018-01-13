/* Coding: ANSI */
#include "tiger_type.h"
#include "tree.h"

namespace tiger{

ExpBase::~ExpBase(){
}
StatementJump::~StatementJump(){
       delete m_exp;
       delete m_list;
}
StatementCjump::~StatementCjump(){
        delete m_left;
        delete m_right;
}
StatementMove::~StatementMove(){
        delete m_left;
        delete m_right;
}
StatementExp::~StatementExp(){
        delete m_exp;
}
    
    
    
}//namespace tiger