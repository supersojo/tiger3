#ifndef ASSEM_H
#define ASSEM_H

#include "assem.h"
#include "frame.h"
#include "tree.h"

namespace tiger{



struct TempMapNode{
    
    TempMapNode(){
        m_temp = 0;
        m_str = 0;
        prev = next = 0;
    }
    ~TempMapNode(){
        free(m_str);
    }
    Temp* m_temp;
    char* m_str;
    TempMapNode* prev;
    TempMapNode* next;
};
class TempMapList{
public:
    TempMapList(){
        m_head = 0;
        m_top = 0;
    }
    TempMapList* LayerMap(TempMapList* over){
        m_top = over;
    }
    void Enter(Temp* temp,char* str){
        TempMapNode* n;
        TempMapNode* p;
        
        p = m_head;
        while(p){
            //already exist
            if(strcmp(p->m_temp->Name(),temp->Name())==0)
                return;
            
            p = p->next;
        }
        
        n = new TempMapNode;
        n->m_temp = temp;
        n->m_str = strdup(str);
        if(m_head)
        {
            n->next = m_head;
            m_head->prev = n;
        }
        m_head = n;
    }
    char* Look(Temp* temp){
        char* ret = 0;
        if(m_top)
            ret = m_top->Look(temp);
        if(ret)
            return ret;
        
        TempMapNode* p = m_head;
        while(p){
            if(strcmp(p->m_temp->Name(),temp->Name())==0)
                return p->m_str;
            p = p->next;
        }
        return 0;
    }
    ~TempMapList(){
        TempMapNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    TempMapNode* m_head;
    TempMapList* m_top;
};

class InstrList{
};


class CodeGenerator{
public:
    InstrList* CodeGen(FrameBase* f,StatementBaseList* l);
private:
    void Munch(InstrList* il,FrameBase* f,StatementBaseList* l){
        
    }
};



}// namespace tiger

#endif
