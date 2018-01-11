#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tiger_assert.h"

namespace tiger{

struct  AssertInfo  assert_info={0};

void tiger_assert(bool condition,char* fmt,...)
{
    if(condition)
        return;
    va_list args;
    va_start(args,fmt);
    vfprintf(stderr,fmt,args);
    fprintf(stderr,"\nAbort\n");
    va_end(args);
    exit(-1);
}
void tiger_assert_win(bool condition,char* fmt,...)
{
    if(condition)
        return;
    
    fprintf(stderr,"[tiger_assertion <%s,%d> Asserion failed] ",assert_info.file,assert_info.line);
    va_list args;
    va_start(args,fmt);
    vfprintf(stderr,fmt,args);
    fprintf(stderr,"\nAbort\n");
    va_end(args);
    exit(-1);
}

}//namespace tiger
