#include "regalloc.h"
#include "tiger_log.h"

namespace tiger{



void Color_(ColorNode* cn,ColorList* cl,CGraph* cg){
    LoggerStdio m_logger;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("color");
    s32 i = 0;
    s32 color[7] = {0};
    s32 c = 0;
    if(cn->m_color!=0)
        return;
    for(i=0;i<cg->GetByTemp(cn->m_temp)->m_links->Size();i++){
        if(cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp )->m_color!=0)
            color[cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp )->m_color]=1;
    }
    for(i=1;i<7;i++)
        if(color[i]==0){
            break;
        }
    if(i==7)
        i=1;
    m_logger.D("color %s %d",cn->m_temp->Name(),i);
    cn->m_color = i;
    for(i=0;i<cg->GetByTemp(cn->m_temp)->m_links->Size();i++){
        if(cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp )->m_color==0)
            Color_(cl->GetByTemp( cg->GetByTemp(cn->m_temp)->m_links->Get(i)->m_temp ),cl,cg);
    }
}
ColorList* Color(TempList* list,CGraph* cg){
    LoggerStdio m_logger;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("color");
    ColorList* cl = new ColorList;
    s32 i = 0;
    s32 c = 0;
    s32 color[7] = {0};
    for(i=0;i<list->Size();i++){
        m_logger.D("%s into color list",list->Get(i)->Name());
        cl->Insert( list->Get(i), ColorList::kColorList_Rear);
    }
    for(i=0;i<cl->Size();i++){
        Color_(cl->Get(i),cl,cg);
    }
    for(i=0;i<cl->Size();i++){
        if(cg->GetByTemp( cl->Get(i)->m_temp )->Degree()!=0)
            color[ cl->Get(i)->m_color ] = 1; 
    }
    for(i=1;i<7;i++)
        if(color[i]==0){
            c = i;
            break;
        }
    for(i=0;i<cl->Size();i++){
        if(cg->GetByTemp( cl->Get(i)->m_temp )->Degree()==0)
            cl->Get(i)->m_color = c; 
    }
    //all degree==0 's node use one color
    return cl;
}
ColorList* RegAlloc(LivenessResult* lr,FrameBase* f,InstrList* il){
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
    
    return Color(q,cg_bak);
}



}//namespace tiger