#ifndef REGS_H
#define REGS_H
#include "temp.h"

// record all reg info in system
/*
 RegList* rl = new RegList;
 SpecialRegs
 ArgRegs
 CalleeRegs
 CallerRegs
 
*/
namespace tiger{

class RegEntry{
public:
    RegEntry(){m_str=0;m_temp=0;}
    RegEntry(char* str,Temp* t){m_str=strdup(str);m_temp = t;}
    Temp* Reg(){return m_temp;}
    char* Name(){return m_str;}
    ~RegEntry(){
        free(m_str);
    }
private:
    char* m_str;
    Temp* m_temp;
};

struct RegEntryNode{

    RegEntryNode(){
        m_reg = 0;
        prev = next = 0;
    }
    ~RegEntryNode(){
        delete m_reg;
    }
    RegEntry* m_reg;
    RegEntryNode* prev;
    RegEntryNode* next;
};
class RegList{
public:
    enum{
        kRegList_Rear,
        kRegList_Front,
        kRegList_Invalid
    };
    RegList(){ m_head = 0; m_size = 0;}
    RegEntryNode* GetHead(){return m_head;}
    RegEntry* GetByIdx(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        RegEntryNode* p = m_head;
        while(p){
            if(i==index)
                return p->m_reg;
            p = p->next;
            i++;
        }
        return 0;
    }
    RegEntry* GetByName(char* name){
        RegEntryNode* p = m_head;
        while(p){
            if(strcmp(p->m_reg->Name(),name)==0)
                return p->m_reg;
            p = p->next;
        }
        return 0;
    }
    ~RegList(){
        RegEntryNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
    void Insert(RegEntry* reg,s32 dir){
        RegEntryNode* n;
        RegEntryNode* p;
        RegEntryNode* q;
        
        n = new RegEntryNode;
        n->m_reg = reg;
        
        if(dir==kRegList_Rear){
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
        if(dir==kRegList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    s32 Size(){return m_size;}
private:
    RegEntryNode* m_head;
    s32 m_size;
};

}//namespace tiger

#endif