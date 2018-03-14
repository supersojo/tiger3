#include "graph.h"

namespace tiger{

CGraphEdgeList* CGraphEdgeList::Clone(CGraph* g){// after clone nodes then clone edges
    CGraphEdgeList* n = new CGraphEdgeList;
    CGraphEdgeNode* p = m_head;
    CGraphEdgeNode* q = 0;
    CGraphEdgeNode* newhead=0;
    while(p){
        CGraphEdgeNode* t = new CGraphEdgeNode;
        t->m_node = g->GetByTemp( p->m_node->m_temp );
        if(newhead==0)
            newhead = t;
        if(q==0)
            q = t;
        else
        {
            q->next = t;
            t->prev = q;
            q = t;
        }
        p = p->next;
    }
    n->m_head = newhead;
    n->m_size = m_size;
    return n;
}
CGraphNode* CGraph::GetByTemp(Temp* t){
    CGraphNode* p = m_head;
    while(p){
        if( strcmp(p->m_temp->Name(),t->Name())==0)
            return p;
        p = p->next;
    }
    return 0;
}

} // namespace tiger