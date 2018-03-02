#ifndef CANON_H
#define CANON_H

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
// canonicalize util 
class Canon{
public:
    // cancel all eseqs in statement, we get a tree without eseq
    StatementBase*     Statementize(StatementBase* statement);
    
    // use the statement without eseq, we get a list of statement without seq statement
    StatementBaseList* Linearize(StatementBase* statement);
    
    // we got a list of basic blocks 
    CanonBlockList* BasicBlocks(StatementBaseList* list);
    
    // schedule all canon blocks ,we got a statement list 
    StatementBaseList* TraceSchedule(CanonBlockList* list);
    
private:
};	
	
} //namespace tiger

#endif
