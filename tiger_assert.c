#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tiger_assert.h"

namespace tiger{

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

}//namespace tiger
