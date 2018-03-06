#include "canon.h"

namespace tiger{

StatementBase*     Canon::Statementize(StatementBase* statement)
{
    return DoStatement( statement );
}

StatementBaseList* Canon::Linearize(StatementBase* statement)
{
    StatementBaseList* l = new StatementBaseList;
    _Linearize(l,statement);
    delete statement;// free the intermediate tree
    return l;
}
void Canon::_Linearize(StatementBaseList* l,StatementBase* statement)
{
    
    if(statement->Kind()==StatementBase::kStatement_Seq){
        _Linearize( l, dynamic_cast<StatementSeq*>(statement)->Left() );
        _Linearize( l, dynamic_cast<StatementSeq*>(statement)->Right() );
    }else{
        l->Insert(statement->Clone(),StatementBaseList::kStatementBaseList_Rear);
    }
}

CanonBlockList* Canon::BasicBlocks(StatementBaseList* list)
{
    StatementBaseNode* p;
    StatementBase* s;
    CanonBlockList* cl = new CanonBlockList;
    CanonBlock* cur = new CanonBlock(new StatementBaseList,0);//dummy block
    p = list->GetHead();
    while(p){
        s = p->m_statement;
        if(s->Kind()==StatementBase::kStatement_Label){
            if(cur){//end prev 
                //cur->UpdateLabel();
                if(cur->GetStatementList()->Size()==0)//dummy block
                {
                    delete cur;
                }else{
                    cl->Insert(cur,CanonBlockList::kCanonBlockList_Rear);
                }
            }
            // start new
            cur = new CanonBlock(new StatementBaseList,dynamic_cast<StatementLabel*>(s)->GetLabel());
            
            cur->GetStatementList()->Insert( s->Clone(), StatementBaseList::kStatementBaseList_Rear);
        }else if(s->Kind()==StatementBase::kStatement_Jump || s->Kind()==StatementBase::kStatement_Cjump){
            cur->GetStatementList()->Insert( s->Clone(), StatementBaseList::kStatementBaseList_Rear);
            //end prev
            cl->Insert(cur,CanonBlockList::kCanonBlockList_Rear);
            cur = 0;
            //start new
            cur = new CanonBlock(new StatementBaseList,0);// without a label
            
        }else{
            cur->GetStatementList()->Insert( s->Clone(), StatementBaseList::kStatementBaseList_Rear);
        }
        p = p->next;
    }
    if(cur->GetStatementList()->Size()==0)
        delete cur;
    else
        cl->Insert(cur,CanonBlockList::kCanonBlockList_Rear);
    // process non good blocks
    CanonBlockNode* t;
    t = cl->GetHead();
    while(t){
        if(t->m_block->GetLabel()==0)
        {
            Label* label = TempLabel::NewLabel();
            t->m_block->UpdateLabel(label);
            t->m_block->GetStatementList()->Insert(new StatementLabel(label),StatementBaseList::kStatementBaseList_Front);
        }
        
        if((t->m_block->GetStatementList()->Get( t->m_block->GetStatementList()->Size()-1 )->Kind()!=StatementBase::kStatement_Jump) &&
           (t->m_block->GetStatementList()->Get( t->m_block->GetStatementList()->Size()-1 )->Kind()!=StatementBase::kStatement_Cjump)){
            if(t->next){
                LabelList* ll = new LabelList;
                ll->Insert(t->next->m_block->GetLabel(),LabelList::kLabelList_Rear);
                t->m_block->GetStatementList()->Insert(new StatementJump( new ExpBaseName(t->next->m_block->GetLabel()),ll),StatementBaseList::kStatementBaseList_Rear);
            }else{// we are the last block but without a jump , use a done label instead
                LabelList* ll = new LabelList;
                Label* done = TempLabel::NewNamedLabel("done");
                ll->Insert(done,LabelList::kLabelList_Rear);
                t->m_block->GetStatementList()->Insert(new StatementJump( new ExpBaseName(done),ll),StatementBaseList::kStatementBaseList_Rear);
            }
        }
        
        t = t->next;
    }
    return cl;
}

StatementBaseList* Canon::TraceSchedule(CanonBlockList* list)
{
    return 0;
}

}//namespace tiger