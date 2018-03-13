#ifndef GRAPH_H
#define GRAPH_H

#include "tiger_type.h"
#include "tiger_assert.h"
#include "tiger_log.h"
#include "assem.h"

namespace tiger{

class GraphNode;

struct EdgeNode{
    EdgeNode(){
        m_node = 0;
        prev = next = 0;
    }
    ~EdgeNode(){
    }
    GraphNode* m_node;
    
    EdgeNode* prev;
    EdgeNode* next;
    
};
class EdgeList{
public:
    enum{
        kEdgeList_Rear,
        kEdgeList_Front,
        kEdgeList_Invalid
    };
    EdgeList(){m_head = 0;m_size=0;}
    s32 Size(){return m_size;}
    s32 Has(GraphNode* n){
        EdgeNode* p = m_head;
        while(p)
        {
            if(p->m_node == n)
                return 1;
            p = p->next;
        }
        return 0;
    }
    GraphNode* Get(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        EdgeNode* p = m_head;
        while(p){
            if(i==index)
                return p->m_node;
            p = p->next;
            i++;
        }
        return 0;
    }
    void Remove(GraphNode* n){
        // n1->n2->n3
        EdgeNode* p;
        EdgeNode* q;//pointer to parent's of p
        p = m_head;
        q = 0;
        while(p){
            if(p->m_node ==n){
                break;
            }
            q = p;
            p = p->next;
        }
        // not found
        if(p==0){
            return;
        }
        // head
        if(q==0){
            m_head = m_head->next;
            if(m_head)
                m_head->prev = 0;
            delete p;
            return;
        }
        // mid
        q->next = p->next;
        if(p->next)
            p->next->prev = q;
        delete p;
    }
    void Insert(GraphNode* n_,s32 dir){
        EdgeNode* n;
        EdgeNode* p;
        EdgeNode* q;
        
        n = new EdgeNode;
        n->m_node = n_;
        
        if(dir==kEdgeList_Rear){
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
        if(dir==kEdgeList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    ~EdgeList(){
        EdgeNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    EdgeNode* m_head;
    s32 m_size;
};
struct GraphNode{
    GraphNode()
    {
        m_instr = 0;
        m_succ = new EdgeList;
        m_pred = new EdgeList;
        m_in = new TempList;
        m_out = new TempList;
        prev = next = 0;
    }
    s32 Degree(){
        return m_succ->Size() + m_pred->Size();
    }
    ~GraphNode(){
        delete m_succ;
        delete m_pred;
        delete m_in;
        delete m_out;
    }
    
    /* members */
    InstrBase* m_instr;
    //for edge
    EdgeList* m_succ;
    EdgeList* m_pred;
    //for node
    GraphNode* next;
    GraphNode* prev;
    //for liveness
    TempList* m_in;
    TempList* m_out;
};

class Graph{
public:
    enum{
        kGraph_Rear,
        kGraph_Front,
        kGraph_Invalid
    };
    Graph(){m_head = 0;m_size=0;}
    s32 Size(){return m_size;}
    void Insert(GraphNode* n_,s32 dir){
        GraphNode* n;
        GraphNode* p;
        GraphNode* q;
        
        n = n_;
        
        if(dir==kGraph_Rear){
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
        if(dir==kGraph_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    void AddNode(GraphNode* n){
        Insert(n,kGraph_Rear);
    }
    void AddEdge(GraphNode* from,GraphNode* to){
        from->m_succ->Insert(to,EdgeList::kEdgeList_Rear);
        to->m_pred->Insert(from,EdgeList::kEdgeList_Rear);
    }
    void RemoveEdge(GraphNode* from,GraphNode* to){
        from->m_succ->Remove(to);
        to->m_pred->Remove(from);
    }
    s32 HasEdge(GraphNode* from,GraphNode* to){
        return ( from->m_succ->Has(to) && to->m_pred->Has(from) );
    }
    GraphNode* Get(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        GraphNode* p = m_head;
        while(p){
            if(i==index)
                return p;
            p = p->next;
            i++;
        }
        return 0;
    }
    GraphNode* GetByLabel(Label*l){
        GraphNode* p = m_head;
        while(p){
            if(p->m_instr->Kind()==InstrBase::kInstr_Label){
                if(dynamic_cast<InstrLabel*>(p->m_instr)->GetLabel() == l)
                    return p;
            }
            p = p->next;
        }
        return 0;
    }
    ~Graph(){
        GraphNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
    GraphNode* GetHead(){return m_head;}
private:
    GraphNode* m_head;
    s32 m_size;
};
class FlowGraph{
public:
    Graph* AssemFlowGraph(InstrList* il){
        InstrBase* instr;
        GraphNode* gn;
        Graph* g = new Graph;
        if(il->Size()==1){
            instr = il->Get(0);
            gn = new GraphNode;
            gn->m_instr = instr;
            g->AddNode(gn);
            return g;
        }
        GraphNode*p,*q;
        s32 i = 0;
        for(i=0;i<il->Size();i++){
             p = new GraphNode;
             p->m_instr = il->Get(i);
             g->AddNode(p);
        }
        
        LabelList* ll;
        char b[1024]={0};
        // process range from 1 to n-1
        for(i=0;i<il->Size()-1;i++){
             p = g->Get(i);
             q = g->Get(i+1);
             ll = p->m_instr->Jump();
             if(ll!=0){
                //add edge
                p->m_instr->InstrStr(b);
                if(strcmp(b,"jmp")==0){
                    g->AddEdge( p, g->GetByLabel(ll->Get(0)) );
                }else{
                    // condition branch
                    g->AddEdge( p, g->GetByLabel(ll->Get(0)) );
                    g->AddEdge( p, q );//false brance
                }
                
             }else{
                g->AddEdge(p,q);
             }
        }
        
        // process the last one
        p = q;
        ll = p->m_instr->Jump();
        if(ll!=0){
            if(strcmp(ll->Get(0)->Name(),"done")==0){
                //eof
            }else{
                g->AddEdge( p, g->GetByLabel(ll->Get(0)) );
            }
        }else{
            ;
        }
        
        return g;
    };
    TempList* Def(GraphNode* n){return n->m_instr->Dst();}
    TempList* Use(GraphNode* n){return n->m_instr->Src();}
    s32 IsMove(GraphNode* n){ return n->m_instr->Kind()==InstrBase::kInstr_Move;}
};
//collision graph
class CGraphNode;
struct CGraphEdgeNode{
    CGraphEdgeNode(){
        m_node = 0;
        prev = next = 0;
    }
    ~CGraphEdgeNode(){
        
    }
    CGraphNode* m_node;
    CGraphEdgeNode* prev;
    CGraphEdgeNode* next;
};
class CGraphEdgeList{
public:
    enum{
       kCGraphEdgeList_Rear,
       kCGraphEdgeList_Front,
       kCGraphEdgeList_Invalid
    };
    CGraphEdgeList(){
        m_head = 0;
        m_size = 0;
    }
    s32 Size(){return m_size;}
    CGraphNode* Get(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        CGraphEdgeNode* p = m_head;
        while(p){
            if(i==index)
                return p->m_node;
            p = p->next;
            i++;
        }
        return 0;
    }
    s32 Has(CGraphNode* n){
        CGraphEdgeNode* p = m_head;
        while(p)
        {
            if(p->m_node == n)
                return 1;
            p = p->next;
        }
        return 0;
    }
    void Insert(CGraphNode* n_,s32 dir){
        CGraphEdgeNode* n;
        CGraphEdgeNode* p;
        CGraphEdgeNode* q;
        if(Has(n_))
            return;
        
        n = new CGraphEdgeNode;
        n->m_node = n_;
        
        if(dir==kCGraphEdgeList_Rear){
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
        if(dir==kCGraphEdgeList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    void Remove(CGraphNode* n){
        // n1->n2->n3
        CGraphEdgeNode* p;
        CGraphEdgeNode* q;//pointer to parent's of p
        p = m_head;
        q = 0;
        while(p){
            if(p->m_node ==n){
                break;
            }
            q = p;
            p = p->next;
        }
        // not found
        if(p==0){
            return;
        }
        // head
        if(q==0){
            m_head = m_head->next;
            if(m_head)
                m_head->prev = 0;
            delete p;
            return;
        }
        // mid
        q->next = p->next;
        if(p->next)
            p->next->prev = q;
        delete p;
    }
    ~CGraphEdgeList(){
        CGraphEdgeNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    CGraphEdgeNode* m_head;
    s32 m_size;
};
struct CGraphNode{
    CGraphNode(){
        m_temp = 0;
        m_links = new CGraphEdgeList;
        prev = next = 0;
    }
    ~CGraphNode(){
        delete m_links;
    }
    // members
    Temp* m_temp;
    CGraphEdgeList* m_links;
    
    CGraphNode* prev;
    CGraphNode* next;
};
class CGraph{
public:
    enum{
        kCGraph_Rear,
        kCGraph_Front,
        kCGraph_Invalid
    };
    CGraph(){
        m_head = 0;
        m_size = 0;
    }
    s32 Size(){return m_size;}
    CGraphNode* Get(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        CGraphNode* p = m_head;
        while(p){
            if(i==index)
                return p;
            p = p->next;
            i++;
        }
        return 0;
    }
    CGraphNode* GetByTemp(Temp* t){
        CGraphNode* p = m_head;
        while(p){
            if(p->m_temp == t)
                return p;
            p = p->next;
        }
        return 0;
    }
    void Insert(CGraphNode* n_,s32 dir){
        CGraphNode* n;
        CGraphNode* p;
        CGraphNode* q;
        
        n = n_;
        
        if(dir==kCGraph_Rear){
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
        if(dir==kCGraph_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    s32 Has(Temp* t){
        CGraphNode* p = m_head;
        while(p){
            if(p->m_temp == t)
                return 1;
            p = p->next;
        }
        return 0;
        
    }
    void AddNode(CGraphNode* n){
        Insert(n,kCGraph_Rear);
    }
    void AddEdge(CGraphNode* from,CGraphNode* to){
        from->m_links->Insert(to,CGraphEdgeList::kCGraphEdgeList_Rear);
        to->m_links->Insert(from,CGraphEdgeList::kCGraphEdgeList_Rear);
    }
    void RemoveEdge(CGraphNode* from,CGraphNode* to){
        from->m_links->Remove(to);
        to->m_links->Remove(from);
    }
    s32 HasEdge(CGraphNode* from,CGraphNode* to){
        return ( from->m_links->Has(to) && to->m_links->Has(from) );
    }
    ~CGraph(){
        CGraphNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    CGraphNode* m_head;
    s32 m_size;
};
// liveness
class Liveness{
public:
    Liveness(){
        m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
        m_logger.SetModule("liveness");
    }
    void LivenessCalc(Graph* g){
        GraphNode* gn;
        Calc(g);
        m_logger.D("Show liveness:");
        s32 i = 0;
        for(i=0;i<g->Size();i++){
            gn = g->Get(i);
            if(gn->m_instr->Kind()==InstrBase::kInstr_Label)
                continue;
            s32 j = 0;
            m_logger.D("node %d:",i);
            for(j=0;j<gn->m_in->Size();j++){
                m_logger.D("\t%s",gn->m_in->Get(j)->Name() );
            }
        }
        
    }
private:
    LoggerStdio m_logger;
    void Calc(Graph* g){
        GraphNode* p = 0;
        while(1){
            s32 c = 0;
            p = g->GetHead();
            while(p){
                c += Merge(p,p->m_in,p->m_out,p->m_instr->Src(),p->m_instr->Dst());
                p = p->next;
                
            }
            if(c==0)
                break;
        }
    }
    s32 Merge(GraphNode* n,TempList *in,TempList *out, TempList *use, TempList* def){
        s32 ret = 0;
        
        //skip label
        if(n->m_instr->Kind()==InstrBase::kInstr_Label){
            return 0;
        }
        ret += in->Merge(use);
        {
            TempList* tmp = new TempList;
            tmp->Merge(out);
            s32 i = 0;
            for(i=0;i<def->Size();i++){
                tmp->Remove( def->Get(i) );
            }
            ret += in->Merge(tmp);
            delete tmp;
        }
        {
            s32 i = 0;
            for(i=0;i<n->m_succ->Size();i++){
                ret += out->Merge( n->m_succ->Get(i)->m_in );
            }
            
        }
        return ret;
    }
};

} // namespace tiger

#endif
