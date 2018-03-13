#ifndef GRAPH_H
#define GRAPH_H

#include "tiger_type.h"
#include "tiger_assert.h"
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
        prev = next = 0;
    }
    ~GraphNode(){
        delete m_succ;
        delete m_pred;
    }
    
    /* members */
    InstrBase* m_instr;
    EdgeList* m_succ;
    EdgeList* m_pred;
    
    GraphNode* next;
    GraphNode* prev;
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
    ~Graph(){
        GraphNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    GraphNode* m_head;
    s32 m_size;
};
class FLowGraph{
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
        // process range from 1 to n-1
        for(i=0;i<il->Size()-1;i++){
             p = g->Get(i);
             q = g->Get(i+1);
             ll = p->m_instr->Jump();
             if(ll!=0){
                s32 j = 0;
                //ll->Get(j)
                
             }else{
                g->AddEdge(p,q);
             }
        }
        
        // process the last one
        p = q;
        ll = p->m_instr->Jump();
        if(ll!=0){
            s32 j = 0;
            // ll->Get(j);
        }
        
        return 0;
    };
    TempList* Def(GraphNode* n){return n->m_instr->Dst();}
    TempList* Use(GraphNode* n){return n->m_instr->Src();}
    s32 IsMove(GraphNode* n);
};


} // namespace tiger

#endif
