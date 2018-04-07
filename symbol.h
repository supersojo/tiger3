#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include "tiger_type.h"
#include "tiger_log.h"

namespace tiger
{
class Pos{
public:
    Pos(){m_line_no = 0; m_off = 0;}
    s32 LineNo(){return m_line_no;}
    s32 Off(){return m_off;}
private:
    s32 m_line_no;
    s32 m_off;
};


class Symbol{
public:
    Symbol(){
        m_name=0;
        m_escape = 0;/*false*/
    }
    Symbol(char* name){
        m_name = strdup(name);
        m_escape = 0;/* false*/
    }
    Symbol* Clone(){
        Symbol* n = new Symbol;
        n->m_name = strdup(m_name);
        n->m_escape = m_escape;
        return n;
    }
    char* Name(){return m_name;}
    ~Symbol(){
        LoggerStdio logger;
        logger.SetLevel(LoggerBase::kLogger_Level_Error);
        logger.SetModule("absyn");
        //logger.D("~Symbol");
        free(m_name);
    }
    s32* GetEscapeRefer(){return &m_escape;}
    s32  GetEscape(){return m_escape;}
    void SetEscape(s32 escape){m_escape = escape;}
private:
    char* m_name;
    /* additional stuff */
    //...
    s32 m_escape;/* only used in for exp*/
};
}

#endif