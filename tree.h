/* Coding: ANSI */
#ifndef TREE_H
#define TREE_H

#include "tiger_type.h"

namespace tiger{

class StatementBase{
public:
    enum{
        kStatement_Seq,
        kStatement_Label,
        kStatement_Jump,
        kStatement_Cjump,
        kStatement_Move,
        kStatement_Exp,
        kStatement_Invalid
    };
    StatementBase(){m_kind=kStatement_Invalid;}
    StatementBase(s32 kind){m_kind=kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};

class StatementSeq:public StatementBase{
public:
    StatementSeq():StatementBase(kStatement_Seq){m_left=0;m_right=0;}
    StatementSeq(StatementBase* left,StatementBase* right):StatementBase(kStatement_Seq){m_left=left;m_right=right;}
    ~StatementSeq(){
        delete m_left;
        delete m_right;
    }
private:
    StatementBase* m_left;
    StatementBase* m_right;
};
class StatementLabel:public StatementBase{
public:
    StatementLabel():StatementBase(kStatement_Label){}
    ~StatementLabel(){
       
    }
private:

};
class StatementJump:public StatementBase{
public:
    StatementJump():StatementBase(kStatement_Jump){}
    ~StatementJump(){
       
    }
private:

};
class StatementCjump:public StatementBase{
public:
    StatementCjump():StatementBase(kStatement_Cjump){}
    ~StatementCjump(){
       
    }
private:

};
class StatementMove:public StatementBase{
public:
    StatementMove():StatementBase(kStatement_Move){}
    ~StatementMove(){
       
    }
private:

};
class StatementExp:public StatementBase{
public:
    StatementExp():StatementBase(kStatement_Exp){}
    ~StatementExp(){
       
    }
private:

};
class ExpBase{
public:
    enum{
        kExpBase_Binop,
        kExpBase_Mem,
        kExpBase_Temp,
        kExpBase_Eseq,
        kExpBase_Name,
        kExpBase_Const,
        kExpBase_Call,
        kExpBase_Invalid
    };
    ExpBase(){m_kind=kExpBase_Invalid;}
    ExpBase(s32 kind){m_kind=kind;}
private:
    s32 m_kind;
};
class ExpBaseBinop:public ExpBase{
};
class ExpBaseMem:public ExpBase{
};
class ExpBaseTemp:public ExpBase{
};
class ExpBaseEseq:public ExpBase{
};
class ExpBaseName:public ExpBase{
};
class ExpBaseConst:public ExpBase{
};
class ExpBaseCall:public ExpBase{
};

}//namespace tiger



#endif
