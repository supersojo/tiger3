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
    return 0;
}

StatementBaseList* Canon::TraceSchedule(CanonBlockList* list)
{
    return 0;
}

}//namespace tiger