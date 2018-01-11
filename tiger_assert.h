/* Coding: ANSI */
#ifndef TIGER_ASSERT_H
#define TIGER_ASSERT_H

#include <stdarg.h>
#include <stdio.h>

#include "tiger_type.h"

#define TIGER_ASSERT(condition,msg...) \
do{ \
    if(!(condition)) \
    { \
    fprintf(stderr,"[tiger_assertion: <%s,%d> Assertion<%s> failed] ",__FILE__,__LINE__,#condition); \
    tiger::tiger_assert(condition,##msg); \
    } \
}while(0)
namespace tiger{

void tiger_assert(bool condition,char* fmt,...);




}//namespace tiger



#endif

