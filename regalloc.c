#include "regalloc.h"
#include "tiger_log.h"

namespace tiger{

/*
rax
rbx
rcx
rdx
rdi
rsi
*/
void RegAlloc(TempMapList* map, LivenessResult* lr, FrameBase* f, InstrList* il){
    // use collision graph to coloring
    // only for k-coloring 
    // Temp* ---- s32  
    s32 r[7]={-1/* not used */,1,1,1,1,1,1};
    char* rs[7]={"","%RAX","%RBX","%RCX","%RDX","%RDI","%RSI"};
    LoggerStdio m_logger;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("regalloc");
    CGraph* cg = lr->m_graph;
    CGraphNode* cgn;
    CGraph* cg_bak;
    
    s32 i = 0;
    
    //preallocated?
    while(1){
        s32 c = 0;
        for(i=0;i<cg->Size();i++){
            cgn = cg->Get(i);
            if(map->Look(cgn->m_temp))//preallocated?
            {
                //m_logger.D("get prealloc %s",cgn->m_temp->Name());
                cg->RemoveNode(cgn);
                c = 1;
                break;
            }
        }
        if(c==0)
            break;
    }
    
    cg_bak = cg->Clone();
    TempList* q = new TempList;//save the k'th color
    
    while(1){
        s32 c = 0;
        for(i=0;i<cg->Size();i++){
            cgn = cg->Get(i);
            if(cgn->Degree()<6){
                //m_logger.D("%s into q",cgn->m_temp->Name());
                q->Insert(cgn->m_temp,TempList::kTempList_Rear);
                cg->RemoveNode(cgn);
                c = 1;// simple
                break;
            }else{
                //can't color for simplicity
                ;
            }
        }
        if(cg->Size()>0 && c==0){
            //m_logger.D("can't simple");
            return;
        }
        if(cg->Size()==0){
            //m_logger.D("simple done");
            break;
        }
    }
    for(s32 i=0;i<q->Size();i++){

        //init r
        r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = 1;
        //select color
        cgn = cg_bak->GetByTemp( q->Get(i) );
        for(s32 j=0;j<cgn->m_links->Size();j++)
        {
            if(cgn->m_links->Get(j)->m_color!=-1){
                r[ cgn->m_links->Get(j)->m_color ] = 0;//not available
            }
        }
        for(s32 j = 1; j<7; j++){
            if(r[j]==1){
                cgn->m_color = j;
                //m_logger.D("%s with %d",cgn->m_temp->Name(),cgn->m_color);
                map->Enter(cgn->m_temp,(char*)rs[j]);
                break;
            }
        }
    }
    
    return;
}



}//namespace tiger