#ifndef CANON_H
#define CANON_H

#include "tiger_log.h"
#include "tree.h"
#include "temp.h"

namespace tiger{

class CanonBlock{
public:
    CanonBlock(){m_list=0;m_label=0;}
    CanonBlock(StatementBaseList* list,Label* label){
        m_list = list;
        m_label = label;
    }
    StatementBaseList* GetStatementList(){return m_list;}
    Label* GetLabel(){return m_label;}
    void UpdateLabel(Label* l){m_label = l;}
    void Dump(char* o){
        if(m_list)
            m_list->Dump(o);
    }
    ~CanonBlock(){
        delete m_list;
        // delete m_label; //managed by TempLabel
    }
private:
    StatementBaseList* m_list;
    Label* m_label;
};
struct CanonBlockNode{
    CanonBlockNode(){
        m_block = 0;
        next = prev = 0;
    }
    ~CanonBlockNode(){
        delete m_block;
    }
    /* members*/
    CanonBlock* m_block;
    CanonBlockNode* next;
    CanonBlockNode* prev;
};
class CanonBlockList{
public:
    enum{
        kCanonBlockList_Rear,
        kCanonBlockList_Front,
        kCanonBlockList_Invalid
    };
    CanonBlockList(){m_head = 0;m_size=0;}
    CanonBlockNode* GetHead(){return m_head;}
    s32 Size(){return m_size;}
    void Insert(CanonBlock* block,s32 dir){
        CanonBlockNode* n;
        CanonBlockNode* p;
        CanonBlockNode* q;
        
        n = new CanonBlockNode;
        n->m_block = block;
        
        if(dir==kCanonBlockList_Rear){
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
        if(dir==kCanonBlockList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    void Dump(char* o){
        CanonBlockNode* p;
        s32 i_offset = 0;
        p = m_head;
        char t[1024]={0};
        while(p){
            if(p->m_block){
                p->m_block->Dump(t);
            }
            i_offset += sprintf(i_offset+o,"%s",t);
            p = p->next;
        }
    }
    ~CanonBlockList(){
        CanonBlockNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    CanonBlockNode* m_head;
    s32 m_size;
};
struct StatementExp_{
    StatementExp_(StatementBase* s_,ExpBase* e_){
        s = s_;
        e = e_;
    }
    StatementExp_(){
        s = 0;
        e = 0;
    }
    StatementBase* s;
    ExpBase* e;
};
struct ExpBaseRefNode{
    ExpBaseRefNode(){
        m_exp_ref = 0;
        next = prev = 0;
    }
    ~ExpBaseRefNode(){
        
    }
    ExpBase** m_exp_ref;
    ExpBaseRefNode* next;
    ExpBaseRefNode* prev;
};
class ExpBaseRefList{
public:
    enum{
        kExpBaseRefList_Rear,
        kExpBaseRefList_Front,
        kExpBaseRefList_Invalid
    };
    ExpBaseRefList(){m_size=0;m_head=0;}
    s32 Size(){return m_size;}
    ExpBaseRefNode* GetHead(){return m_head;}
    void Insert(ExpBase** exp_ref,s32 dir){
        ExpBaseRefNode* n;
        ExpBaseRefNode* p;
        ExpBaseRefNode* q;
        
        n = new ExpBaseRefNode;
        n->m_exp_ref = exp_ref;
        
        if(dir==kExpBaseRefList_Rear){
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
        if(dir==kExpBaseRefList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    ~ExpBaseRefList(){
        ExpBaseRefNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    ExpBaseRefNode* m_head;
    s32 m_size;
};
// canonicalize util 
class Canon{
public:
    Canon(){
        m_logger.SetModule("canon");
        m_logger.SetLevel(tiger::LoggerBase::kLogger_Level_Error);
    }
    // cancel all eseqs in statement, we get a tree without eseq
    StatementBase*     Statementize(StatementBase* statement);
    
    // use the statement without eseq, we get a list of statement without seq statement
    StatementBaseList* Linearize(StatementBase* statement);
    
    // we got a list of basic blocks 
    CanonBlockList* BasicBlocks(StatementBaseList* list);
    
    // schedule all canon blocks ,we got a statement list 
    StatementBaseList* TraceSchedule(CanonBlockList* list);
    
private:
    LoggerStdio m_logger;
    
    void _Linearize(StatementBaseList* l,StatementBase* statement);
    
    s32 IsNop(StatementBase* s){
        return ((s->Kind()==StatementBase::kStatement_Exp) &&
                   (dynamic_cast<StatementExp*>(s)->GetExp()->Kind()==ExpBase::kExpBase_Const));
    }
    s32 CanCommute(StatementBase* s,ExpBase* e){
        return (IsNop(s) ||
                   (e->Kind()==ExpBase::kExpBase_Name) ||
                      (e->Kind()==ExpBase::kExpBase_Const));
    }
    StatementBase* Reorder(ExpBaseRefList* exp_ref_list){
        StatementExp_ tmp;
        StatementBase* s = 0;
        ExpBaseRefNode* p = 0;
        ExpBase* e;
        if(exp_ref_list->Size()==0){
            delete exp_ref_list;
            return new StatementExp(new ExpBaseConst(0));// the dummy statementexp will be delete from Seq()
        }else{
            p = exp_ref_list->GetHead();
            while(p){
                e = *(p->m_exp_ref);
                if( e->Kind() == ExpBase::kExpBase_Call){
                    // rewrite call to eseq(mov,call)
                    Temp* t = TempLabel::NewTemp();
                    *(p->m_exp_ref) = new ExpBaseEseq(new StatementMove(new ExpBaseTemp(t),e),new ExpBaseTemp(t));
                    e = *(p->m_exp_ref);
                }
                tmp = DoExp( e );// clone what we changed 
                if(e->Kind()==ExpBase::kExpBase_Eseq){
                    e->Clean();//release what we refer to
                    delete e;
                }
                *(p->m_exp_ref) = tmp.e;
                if(s==0){
                    s = tmp.s;
                }else{
                    s = Seq( s, tmp.s);
                }
                p = p->next;
            }
            delete exp_ref_list;
            return s;
        }
    }
    StatementBase* DoStatement(StatementBase* s){
        ExpBaseRefList* exp_ref_list = new ExpBaseRefList;
        switch(s->Kind()){
            case StatementBase::kStatement_Seq:{
                StatementBase* tmp;
                
                delete exp_ref_list;
                
                tmp =  Seq( DoStatement(dynamic_cast<StatementSeq*>(s)->Left()), DoStatement(dynamic_cast<StatementSeq*>(s)->Right()) );
                s->Clean();//delete seq itself
                delete s;
                
                return tmp;
            }
            case StatementBase::kStatement_Jump:
                exp_ref_list->Insert( dynamic_cast<StatementJump*>(s)->GetExpRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                return Seq( Reorder(exp_ref_list), s ); 
            case StatementBase::kStatement_Cjump:
                exp_ref_list->Insert( dynamic_cast<StatementCjump*>(s)->LeftRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                exp_ref_list->Insert( dynamic_cast<StatementCjump*>(s)->RightRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                return Seq( Reorder(exp_ref_list), s ); 
            case StatementBase::kStatement_Move:
            {
                if((dynamic_cast<StatementMove*>(s)->Left()->Kind()==ExpBase::kExpBase_Temp) &&
                    (dynamic_cast<StatementMove*>(s)->Right()->Kind()==ExpBase::kExpBase_Call)){
                    Get_Call_Rlist(exp_ref_list,dynamic_cast<StatementMove*>(s)->Right());
                    return Seq( Reorder(exp_ref_list), s ); 
                }else if(dynamic_cast<StatementMove*>(s)->Left()->Kind()==ExpBase::kExpBase_Temp){
                    m_logger.D("in move");
                    exp_ref_list->Insert( dynamic_cast<StatementMove*>(s)->RightRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                    return Seq( Reorder(exp_ref_list), s ); 
                }else if(dynamic_cast<StatementMove*>(s)->Left()->Kind()==ExpBase::kExpBase_Mem){
                    dynamic_cast<ExpBaseMem*>(dynamic_cast<StatementMove*>(s)->Left())->GetExpRef();
                    exp_ref_list->Insert( dynamic_cast<ExpBaseMem*>(dynamic_cast<StatementMove*>(s)->Left())->GetExpRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                    exp_ref_list->Insert( dynamic_cast<StatementMove*>(s)->RightRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                    return Seq( Reorder(exp_ref_list), s ); 
                }else if(dynamic_cast<StatementMove*>(s)->Left()->Kind()==ExpBase::kExpBase_Eseq){
                    ExpBase* tmp;
                    delete exp_ref_list;
                    StatementBase* s1 = dynamic_cast<ExpBaseEseq*>(dynamic_cast<StatementMove*>(s)->Left())->GetStatement();
                    tmp = *(dynamic_cast<StatementMove*>(s)->LeftRef());
                    *(dynamic_cast<StatementMove*>(s)->LeftRef()) = dynamic_cast<ExpBaseEseq*>(dynamic_cast<StatementMove*>(s)->Left())->GetExp();
                    tmp->Clean();//free s,e 's reference 
                    delete tmp;// free eseq itself
                    return DoStatement(new StatementSeq(s1,s));
                }
            }
            case StatementBase::kStatement_Exp:
                if(dynamic_cast<StatementExp*>(s)->GetExp()->Kind()==ExpBase::kExpBase_Call){
                    Get_Call_Rlist(exp_ref_list,dynamic_cast<StatementExp*>(s)->GetExp());
                    return Seq( Reorder(exp_ref_list), s);
                }else{
                    exp_ref_list->Insert( dynamic_cast<StatementExp*>(s)->GetExpRef(), ExpBaseRefList::kExpBaseRefList_Rear );
                    return Seq( Reorder(exp_ref_list), s);
                }
            default: //StatementLabel
                delete exp_ref_list;
                return s;
        }
    }
    void Get_Call_Rlist(ExpBaseRefList* l,ExpBase* e){
        ExpBaseNode* p;
        ExpBaseList* list =
            dynamic_cast<ExpBaseCall*>(e)->GetList();
        p = list->GetHead();
        while(p){
            l->Insert( &(p->m_exp), ExpBaseRefList::kExpBaseRefList_Rear);
            p = p->next;
        }
    }
    StatementBase* Seq(StatementBase* s1,StatementBase* s2){
        if(IsNop(s1)) { delete s1; return s2; }
        if(IsNop(s2)) { delete s2; return s1; }
        return new StatementSeq(s1,s2);
    }
    StatementExp_ DoExp(ExpBase* e){
        ExpBaseRefList* exp_ref_list = new ExpBaseRefList;
        switch(e->Kind()){
            case ExpBase::kExpBase_Binop:
                exp_ref_list->Insert(dynamic_cast<ExpBaseBinop*>(e)->LeftRef(),ExpBaseRefList::kExpBaseRefList_Rear);
                exp_ref_list->Insert(dynamic_cast<ExpBaseBinop*>(e)->RightRef(),ExpBaseRefList::kExpBaseRefList_Rear);
                return StatementExp_(Reorder(exp_ref_list),e);
            case ExpBase::kExpBase_Mem:
                exp_ref_list->Insert(dynamic_cast<ExpBaseMem*>(e)->GetExpRef(),ExpBaseRefList::kExpBaseRefList_Rear);
                return StatementExp_(Reorder(exp_ref_list),e);
            case ExpBase::kExpBase_Eseq:{
                delete exp_ref_list;
                m_logger.D("in eseq");
                StatementExp_ tmp = DoExp(dynamic_cast<ExpBaseEseq*>(e)->GetExp());
                return StatementExp_(Seq(
                                        DoStatement(dynamic_cast<ExpBaseEseq*>(e)->GetStatement()),
                                        tmp.s
                                        ),
                                        tmp.e
                                    );
            }
            case ExpBase::kExpBase_Call:
                Get_Call_Rlist(exp_ref_list,e);
                return StatementExp_(Reorder(exp_ref_list),e);;
            default: //ExpBaseTemp ExpBaseName ExpBaseConst
                return StatementExp_(Reorder(exp_ref_list),e);
        }
    }
};	
	
} //namespace tiger

#endif
