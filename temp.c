#include "temp.h"
#include <iostream>

namespace tiger{
    
//temp
char* TempPool::m_name_prefix="T";
s32   TempPool::m_next_id=0;

//label
char* LabelPool::m_name_prefix="L";
s32   LabelPool::m_next_id=0;

TempPool*  TempLabel::m_temp_pool = 0;
LabelPool* TempLabel::m_label_pool = 0;
s32 TempLabel::m_initialized = 0;

TempPool::TempPool(){
    m_list = 0;
    m_next_id = 0;
}
Temp* TempPool::NewTemp()
{
    TempNode* p;
    Temp* n;
    char buf[32];
    
    
    sprintf(buf,"%s%03d",m_name_prefix,m_next_id);
    m_next_id++;
    
    n = new Temp(buf);
    
    p = new TempNode;
    p->m_temp = n;
    
    p->next = m_list;
    if(m_list)
        m_list->prev = p;
    
    m_list = p;
    
    return n;
}

Temp* TempPool::FindName(char* name)
{
    TempNode* p;
    p = m_list;
    
    while(p){
        if(strcmp(p->m_temp->Name(),name)==0)
            return p->m_temp;
        p = p->next;
    }
    return 0;
}

Temp* TempPool::NewNamedTemp(char* name)
{
    TempNode* p;
    Temp* n;

    n = FindName(name);
    if(n)
        return n;
    
    n = new Temp(name);
    
    p = new TempNode;
    p->m_temp = n;
    
    p->next = m_list;
    if(m_list)
        m_list->prev = p;
    
    m_list = p;
    
    return n;
}

TempPool::~TempPool(){
    TempNode* p;
    p = m_list;
    while(p){
        m_list = m_list->next;
        delete p;
        p = m_list;
    }
    m_next_id = 0;
}

LabelPool::LabelPool(){
    m_list = 0;
    m_next_id = 0;
}
Label* LabelPool::NewLabel(){
    LabelNode* p;
    Label* n;
    char buf[32];
    
    sprintf(buf,"%s%03d",m_name_prefix,m_next_id);
    m_next_id++;
    
    n = new Label(buf);
    
    p = new LabelNode;
    p->m_label = n;
    
    p->next = m_list;
    if(m_list)
        m_list->prev = p;
    
    m_list = p;
    
    return n;
}
Label* LabelPool::FindName(char* name)
{
    LabelNode* p;
    p = m_list;
    
    while(p){
        if(strcmp(p->m_label->Name(),name)==0)
            return p->m_label;
        p = p->next;
    }
    return 0;
}
Label* LabelPool::NewNamedLabel(char* name){
    LabelNode* p;
    Label* n;
    
    n = FindName(name);
    if(n)
        return n;
    
    n = new Label(name);
    
    p = new LabelNode;
    p->m_label = n;
    
    p->next = m_list;
    if(m_list)
        m_list->prev = p;
    
    m_list = p;
    
    return n;
}
LabelPool::~LabelPool(){
    LabelNode* p;
    p = m_list;
    while(p){
        m_list = m_list->next;
        delete p;
        p = m_list;
    }
    m_next_id = 0;
}

}//namespace tiger
