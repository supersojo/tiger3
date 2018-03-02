/* Coding: ANSI */
/*
temp -> T001
label ->L001

TempPool::NewTemp(); ->Temp

LabelPool::NewLabel(); ->Label

LabelPool::NewNamedLabel(""); ->Label
*/
#ifndef TEMP_H
#define TEMP_H

#include "tiger_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * temp/label utils
 *
 * Before use it, call TempLabel::Init() first.
 *
 * TempLabel::NewLabel()
 *
 * After used, call TempLabel::Exit(). 
*/
namespace tiger{
    
class Temp{
public:
    Temp(){m_name=0;}
    Temp(char* name){m_name=strdup(name);/* memory leak */}
    char* Name(){return m_name;}
    ~Temp(){free(m_name);m_name = 0;}
private:
    char* m_name;
};
struct TempNode{
    TempNode(){
        m_temp = 0;
        prev = next = 0;
    }
    ~TempNode(){
        delete m_temp;
    }
    Temp* m_temp;
    TempNode* prev;
    TempNode* next;
};
class TempPool{
public:
    Temp* NewTemp();
    TempPool();
    ~TempPool();
private:
    static char* m_name_prefix;
    static s32   m_next_id;
    TempNode* m_list;
};
class Label{
public:
    Label(){m_name=0;}
    Label(char* name){m_name=strdup(name);/* memory leak */}
    char* Name(){return m_name;}
    ~Label(){free(m_name);m_name = 0;}
private:
    char* m_name;
};
struct LabelNode{
    LabelNode(){
        m_label = 0;
        prev = next = 0;
    }
    ~LabelNode(){
        delete m_label;
    }
    Label* m_label;
    LabelNode* prev;
    LabelNode* next;
};
class LabelPool{
public:
    LabelPool();
    Label* NewLabel();
    Label* NewNamedLabel(char* name);
    ~LabelPool();
private:
    static char* m_name_prefix;
    static s32   m_next_id;
    LabelNode* m_list;
};    
    
class TempLabel{
public:
    static void Init(){
        if(m_temp_pool==0)
            m_temp_pool = new TempPool;
        if(m_label_pool==0)
            m_label_pool = new LabelPool;
        m_initialized = 1;
    }
    static void Exit(){
        if(m_temp_pool)
            delete m_temp_pool;
        if(m_label_pool)
            delete m_label_pool;
        
        m_temp_pool = 0;
        m_label_pool = 0;
    }
    static Temp* NewTemp(){
        if(m_initialized==0)
            Init();
        return m_temp_pool->NewTemp();
    }
    static Label* NewLabel(){
        if(m_initialized==0)
            Init();
        return m_label_pool->NewLabel();
    }
    static Label* NewNamedLabel(char* name){
        if(m_initialized==0)
            Init();
        return m_label_pool->NewNamedLabel(name);
    }
private:
    TempLabel(){}
    static TempPool*  m_temp_pool;
    static LabelPool* m_label_pool;
    static s32 m_initialized;
};
    
}//namespace tiger


#endif

