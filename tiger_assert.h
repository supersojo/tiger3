/* Coding: ANSI */
#ifndef TIGER_ASSERT_H
#define TIGER_ASSERT_H

#include <stdarg.h>
#include <stdio.h>

#include "tiger_type.h"


namespace tiger{

struct AssertInfo{
    char* file;
    s32 line;
};

extern struct AssertInfo assert_info;

void tiger_assert(bool condition,char* fmt,...);
void tiger_assert_win(bool condition,char* fmt,...);

}//namespace tiger


#ifdef _WIN32
#   define TIGER_ASSERT \
    do{ \
        tiger::assert_info.file = __FILE__;\
        tiger::assert_info.line = __LINE__;\
    }while(0);tiger::tiger_assert_win

#else //!_WIN32
    
#   define TIGER_ASSERT(condition,msg...) \
    do{ \
        if(!(condition)) \
        { \
        fprintf(stderr,"[tiger_assertion: <%s,%d> Assertion<%s> failed] ",__FILE__,__LINE__,#condition); \
        tiger::tiger_assert(condition,##msg); \
        } \
    }while(0)
        
#endif // !_WIN32 




#endif

