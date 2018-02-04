/* Coding: ANSI */
#ifndef TREE_H
#define TREE_H

#include "tiger_type.h"
#include "temp.h"
#include "absyn.h"

namespace tiger{

class ExpBase;

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
    virtual ~StatementBase(){}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
struct StatementBaseNode{
    StatementBaseNode(){
        m_statement = 0;
        prev = next = 0;
    }
    ~StatementBaseNode(){
        delete m_statement;
    }
    /* members */
    StatementBase* m_statement;
    StatementBaseNode* prev;
    StatementBaseNode* next;
};
class StatementBaseList{
public:
    enum{
        kStatementBaseList_Rear,
        kStatementBaseList_Front,
        kStatementBaseList_Invalid
    };
    StatementBaseList(){m_head = 0;m_size=0;}
    StatementBaseList(StatementBaseNode* head){m_head = head;}
    void Insert(StatementBase* statement,s32 dir){
        StatementBaseNode* n;
        StatementBaseNode* p;
        StatementBaseNode* q;
        
        n = new StatementBaseNode;
        n->m_statement = statement;
        
        if(dir==kStatementBaseList_Rear){
            p = m_head;
            q = m_head;
            while(p){
                q = p;
                p = p->next;
            }
            if(q){
                q->next = n;
                n->prev = q;
            }else{
                m_head = n;
            }
        }
        if(dir==kStatementBaseList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    s32 Size(){return m_size;}
    ~StatementBaseList(){
        StatementBaseNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    StatementBaseNode* m_head;
    s32 m_size;
};
class StatementSeq:public StatementBase{
public:
    StatementSeq():StatementBase(kStatement_Seq){m_left=0;m_right=0;}
    StatementSeq(StatementBase* left,StatementBase* right):StatementBase(kStatement_Seq){m_left=left;m_right=right;}
    StatementBase* Left(){return m_left;}
    StatementBase* Right(){return m_right;}
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
    StatementLabel(Label* l):StatementBase(kStatement_Label){m_label = l;}
    Label* GetLabel(){return m_label;}
    ~StatementLabel(){
       
    }
private:
    Label* m_label;/* managed by TempLabel */
};
struct ALabelNode{
    ALabelNode(){
        m_label = 0;
        prev = next = 0;
    }
    ~ALabelNode(){
        
    }
    Label* m_label;/* managed by TempLabel */
    ALabelNode* prev;
    ALabelNode* next;
};
class LabelList{
public:
    enum{
        kLabelList_Rear,
        kLabelList_Front,
        kLabelList_Invalid
    };
    LabelList(){m_head=0;m_size=0;}
    LabelList(ALabelNode* head){m_head = head;}
    Label* GetHeadLabel(){return m_head->m_label;}
    void Insert(Label* l,s32 dir){
        ALabelNode* n;
        ALabelNode* p;
        ALabelNode* q;
        
        n = new ALabelNode;
        n->m_label = l;
        if(dir==kLabelList_Rear){
            p = m_head;
            q = m_head;
            while(p){
                q = p;
                p = p->next;
            }
            if(q){
                q->next = n;
                n->prev = q;
            }else{
                m_head = n;
            }
        }
        if(dir==kLabelList_Front)
        {
            n->next = m_head;
            if(m_head)
            {
                m_head->prev = n;
            }
            m_head = n;
        }
        m_size++;
    }
    s32 Size(){
        return m_size;
    }
    ~LabelList(){
        ALabelNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    ALabelNode* m_head;
    s32 m_size;
};
class StatementJump:public StatementBase{
public:
    StatementJump():StatementBase(kStatement_Jump){}
    StatementJump(ExpBase* exp,LabelList* list):StatementBase(kStatement_Jump){
        m_exp = exp;
        m_list = list;
    }
    ExpBase* GetExp(){return m_exp;}
    LabelList* GetList(){return m_list;}
    ~StatementJump();
private:
    ExpBase* m_exp;
    LabelList* m_list;
};
class RelationOp{
public:
    enum{
        kRelationOp_Eq,
        kRelationOp_Ne,
        
        /* signed */
        kRelationOp_Lt,
        kRelationOp_Le,
        kRelationOp_Gt,
        kRelationOp_Ge,
        
        /* unsigned */
        kRelationOp_Ult,
        kRelationOp_Ule,
        kRelationOp_Ugt,
        kRelationOp_Uge,
        kRelationOp_Invalid
    };
    static s32 ToRelationOp(s32 k){
        if(k==Oper::kOper_Lt)
            return kRelationOp_Lt;
        if(k==Oper::kOper_Le)
            return kRelationOp_Le;
        if(k==Oper::kOper_Gt)
            return kRelationOp_Gt;
        if(k==Oper::kOper_Ge)
            return kRelationOp_Ge;
        
        return kRelationOp_Invalid;
    }
};
class BinaryOp{
public:
    enum{
        kBinaryOp_Add,
        kBinaryOp_Sub,
        kBinaryOp_Mul,
        kBinaryOp_Div,
        
        kBinaryOp_And,
        kBinaryOp_Or,
        
        kBinaryOp_Lshift,
        kBinaryOp_Rshift,
        
        kBinaryOp_Arshift,
        
        kBinaryOp_Xor,
        
        kBinaryOp_Invalid
        
    };
    static s32 ToBinaryOp(s32 k){
        if(k==Oper::kOper_Add)
            return kBinaryOp_Add;
        if(k==Oper::kOper_Sub)
            return kBinaryOp_Sub;
        if(k==Oper::kOper_Mul)
            return kBinaryOp_Mul;
        if(k==Oper::kOper_Div)
            return kBinaryOp_Div;
        
        return kBinaryOp_Invalid;
    }
};
class StatementCjump:public StatementBase{
public:
    StatementCjump():StatementBase(kStatement_Cjump){}
    StatementCjump(s32 op,ExpBase* left,ExpBase* right,Label* t,Label* f):StatementBase(kStatement_Cjump){
        m_op = op;
        m_left = left;
        m_right = right;
        m_true = t;
        m_false = f;
        
    }
    ExpBase* Left(){return m_left;}
    ExpBase* Right(){return m_right;}
    Label** GetATrueLabel(){return &m_true;}
    Label** GetAFalseLabel(){return &m_false;}
    Label* GetTrueLabel(){return m_true;}
    Label* GetFalseLabel(){return m_false;}
    ~StatementCjump();
private:
    s32 m_op;
    
    ExpBase* m_left;
    ExpBase* m_right;
    
    Label* m_true;/* managed by TempLabel */
    Label* m_false;/* managed by TempLabel */
};
class StatementMove:public StatementBase{
public:
    StatementMove():StatementBase(kStatement_Move){}
    StatementMove(ExpBase* l,ExpBase* r):StatementBase(kStatement_Move){
        m_left = l;
        m_right = r;
    }
    ExpBase* Left(){return m_left;}
    ExpBase* Right(){return m_right;}
    ~StatementMove();
private:
    ExpBase* m_left;
    ExpBase* m_right;
};
class StatementExp:public StatementBase{
public:
    StatementExp():StatementBase(kStatement_Exp){}
    StatementExp(ExpBase* exp):StatementBase(kStatement_Exp){
        m_exp = exp;
    }
    ExpBase* GetExp(){return m_exp;}
    ~StatementExp();
private:
    ExpBase* m_exp;
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
    s32 Kind(){return m_kind;}
    virtual ~ExpBase();
private:
    s32 m_kind;
};
struct ExpBaseNode{
    
    ExpBaseNode(){
        m_exp = 0;
        prev = next = 0;
    }
    ~ExpBaseNode(){
        delete m_exp;
    }
    
    /* members */
    ExpBase* m_exp;
    ExpBaseNode* prev;
    ExpBaseNode* next;
};
class ExpBaseList{
public:
    enum{
        kExpBaseList_Rear,
        kExpBaseList_Front,
        kExpBaseList_Invalid
    };
    ExpBaseList(){m_head = 0;m_size=0;}
    void Insert(ExpBase* exp,s32 dir){
        ExpBaseNode* n;
        ExpBaseNode* p;
        ExpBaseNode* q;
        
        n = new ExpBaseNode;
        n->m_exp = exp;
        
        if(dir==kExpBaseList_Rear){
            p = m_head;
            q = m_head;
            while(p){
                q = p;
                p = p->next;
            }
            if(q){
                q->next = n;
                n->prev = q;
            }else{
                m_head = n;
            }
        }
        if(dir==kExpBaseList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    s32 Size(){return m_size;}
    ~ExpBaseList(){
        ExpBaseNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    ExpBaseNode* m_head;
    s32 m_size;
};
class ExpBaseBinop:public ExpBase{
public:
    ExpBaseBinop():ExpBase(kExpBase_Binop){}
    ExpBaseBinop(s32 op,ExpBase* l,ExpBase* r):ExpBase(kExpBase_Binop){
        m_op = op;
        m_left = l;
        m_right = r;
    }
    ExpBase* Left(){return m_left;}
    ExpBase* Right(){return m_right;}
    ~ExpBaseBinop(){
        delete m_left;
        delete m_right;
    }
private:
    s32      m_op;
    ExpBase* m_left;
    ExpBase* m_right;
};
class ExpBaseMem:public ExpBase{
public:
    ExpBaseMem():ExpBase(kExpBase_Mem){}
    ExpBaseMem(ExpBase* exp):ExpBase(kExpBase_Mem){
        m_exp = exp;
    }
    ExpBase* GetExp(){return m_exp;}
    ~ExpBaseMem(){
        delete m_exp;
    }
private:
    ExpBase* m_exp;
};
class ExpBaseTemp:public ExpBase{
public:
    ExpBaseTemp():ExpBase(kExpBase_Temp){}
    ExpBaseTemp(Temp* t):ExpBase(kExpBase_Temp){
        m_temp = t;
    }
    Temp* GetTemp(){return m_temp;}
    ~ExpBaseTemp(){
    }
private:
    Temp* m_temp;/* managed by TempLabel */
};
class ExpBaseEseq:public ExpBase{
public:
    ExpBaseEseq():ExpBase(kExpBase_Eseq){}
    ExpBaseEseq(StatementBase* s,ExpBase* exp):ExpBase(kExpBase_Eseq){
        m_statement = s;
        m_exp = exp;
    }
    StatementBase* GetStatement(){return m_statement;}
    ExpBase* GetExp(){return m_exp;}
    ~ExpBaseEseq(){
        delete m_statement;
        delete m_exp;
    }
private:
    StatementBase* m_statement;
    ExpBase*       m_exp;
};
class ExpBaseName:public ExpBase{
public:
    ExpBaseName():ExpBase(kExpBase_Name){}
    Label* GetLabel(){return m_label;}
    ExpBaseName(Label* l):ExpBase(kExpBase_Name){
        m_label = l;
    }
    ~ExpBaseName(){
    }
private:
    Label* m_label;/* managed by TempLabel */
};
class ExpBaseConst:public ExpBase{
public:
    ExpBaseConst():ExpBase(kExpBase_Const){}
    ExpBaseConst(s32 val):ExpBase(kExpBase_Const){
        m_val = val;
    }
    s32 GetValue(){return m_val;}
private:
    s32 m_val;
};
class ExpBaseCall:public ExpBase{
public:
    ExpBaseCall():ExpBase(kExpBase_Call){}
    ExpBaseCall(ExpBase* exp,ExpBaseList* list):ExpBase(kExpBase_Call){
        m_exp = exp;
        m_explist = list;
    }
    ~ExpBaseCall(){
        delete m_exp;
        delete m_explist;
    }
private:
    ExpBase*     m_exp;
    ExpBaseList* m_explist;
};

}//namespace tiger



#endif
