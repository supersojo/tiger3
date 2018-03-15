#include "regalloc.h"
#include "tiger_log.h"

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


void Color_(ColorNode* cn,ColorList* cl,CGraph* cg){
    LoggerStdio m_logger;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("color");
    s32 i = 0;
    s32 color[5] = {0};
    s32 c = 0;
    if(cn->m_color!=0)
        return;
    for(i=0;i<cg->GetByTemp(cn->m_temp)->m_links->Size();i++){
        if(cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp )->m_color!=0)
            color[cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp )->m_color]=1;
    }
    for(i=1;i<5;i++)
        if(color[i]==0){
            break;
        }
    if(i==5)
        i=1;
    m_logger.D("color %s %d",cn->m_temp->Name(),i);
    cn->m_color = i;
    for(i=0;i<cg->GetByTemp(cn->m_temp)->m_links->Size();i++){
        if(cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp )->m_color==0)
            Color_(cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp ),cl,cg);
    }
}
void Color(TempList* list,CGraph* cg){
    LoggerStdio m_logger;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("color");
    ColorList* cl = new ColorList;
    s32 i = 0;
    for(i=0;i<list->Size();i++){
        m_logger.D("%s into color list",list->Get(i)->Name());
        cl->Insert( list->Get(i), ColorList::kColorList_Rear);
    }
    for(i=0;i<cl->Size();i++){
        Color_(cl->Get(i),cl,cg);
    }
}
TempMapList* RegAlloc(LivenessResult* lr,FrameBase* f,InstrList* il){
    // use collision graph to coloring
    // only for k-coloring 
    // Temp* ---- s32  
    LoggerStdio m_logger;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("regalloc");
    CGraph* cg = lr->m_graph;
    CGraphNode* cgn;
    CGraph* cg_bak = cg->Clone();
    TempList* q = new TempList;
    s32 i = 0;
    while(1){
        for(i=0;i<cg->Size();i++){
            cgn = cg->Get(i);
            if(cgn->Degree()<4){
                m_logger.D("%s into q",cgn->m_temp->Name());
                q->Insert(cgn->m_temp,TempList::kTempList_Rear);
                cg->RemoveNode(cgn);
                break;
            }else{
                //can't color for simplicity
                ;
            }
        }
        if(cg->Size()==0)
            break;
    }
    Color(q,cg_bak);
    
    return 0;
}



}//namespace tiger