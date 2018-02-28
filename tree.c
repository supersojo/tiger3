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
    
StatementJump* StatementJump::Clone(){
        StatementJump* n = new StatementJump;
        n->m_exp = m_exp?m_exp->Clone():0;
        n->m_list = m_list?m_list->Clone():0;
        return n;
}
StatementCjump* StatementCjump::Clone(){
        StatementCjump* n = new StatementCjump;
        n->m_op = m_op;
        n->m_left = m_left?m_left->Clone():0;
        n->m_right = m_left?m_right->Clone():0;
        n->m_true = m_true;
        n->m_false = m_false;
        return n;
}
StatementMove* StatementMove::Clone(){
        StatementMove* n = new StatementMove;
        n->m_left = m_left?m_left->Clone():0;
        n->m_right = m_left?m_right->Clone():0;
        return n;
}

StatementExp* StatementExp::Clone(){
        StatementExp* n = new StatementExp;
        n->m_exp = m_exp?m_exp->Clone():0;
        return n;
}
    

}//namespace tiger