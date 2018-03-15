#ifndef REGALLOC_H
#define REGALLOC_H

#include "tiger_assert.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "regalloc.h"

namespace tiger{

struct ColorNode{
    ColorNode()
    {
        m_temp = 0;
        m_color = 0;
        prev = next = 0;
    }
    Temp* m_temp;
    s32   m_color;
    ColorNode* prev;
    ColorNode* next;
};
class ColorList{
public:
    enum{
        kColorList_Rear,
        kColorList_Front,
        kColorList_Invalid
    };
    ColorList(){
        m_head = 0;
        m_size = 0;
    }
    s32 Size(){return m_size;}
    ColorNode* Get(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        ColorNode* p = m_head;
        while(p){
            if(i==index)
                return p;
            p = p->next;
            i++;
        }
        return 0;
    }
    void Insert(Temp* t,s32 dir){
        ColorNode* n;
        ColorNode* p;
        ColorNode* q;
        
        n = new ColorNode;
        n->m_temp = t;
        if(dir==kColorList_Rear){
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
        if(dir==kColorList_Front)
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
    ColorNode* GetByTemp(Temp* t){
        ColorNode* p = m_head;
        while(p){
            if(strcmp(t->Name(),p->m_temp->Name())==0)
                return p;
            p = p->next;
        }
        return 0;
    }
    ~ColorList(){
        ColorNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    ColorNode* m_head;
    s32 m_size;
};

ColorList* RegAlloc(LivenessResult* lr,FrameBase* f,InstrList* il);

}//namespace tiger
#endif
