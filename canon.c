#include "canon.h"

namespace tiger{

StatementBase*     Canon::Statementize(StatementBase* statement)
{
    return 0;
}

StatementBaseList* Canon::Linearize(StatementBase* statement)
{
    return 0;
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