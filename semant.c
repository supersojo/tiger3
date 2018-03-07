#include <iostream>
#include "semant.h"
#include "tiger_assert.h"
#include "frame.h"
#include "tree.h"

/*
 FP -------->| local vars
             | actual args
             | static link    
             | ret address
             
*/
namespace tiger{
    


LitStringList::LitStringList(){
    m_tab = new LitStringNode*[kLitStringList_Size];
    for(s32 i=0;i<kLitStringList_Size;i++){
        m_tab[i] = 0;
    }
    m_size = 0;
    
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("LitStringList");
}
void LitStringList::Insert(Label* l,char* str){
    s32 index = hash(l);
    LitStringNode* p;
    p = m_tab[index];
    while(p){
        if(l==p->m_label){
            // exist already
            m_logger.D("%s already exist",l->Name());
            return;
        }
        p = p->next;
    }
    LitStringNode* n;
    n = new LitStringNode;
    n->m_label = l;
    n->m_string = strdup(str);//memory leak
    n->next = m_tab[index];
    if(m_tab[index])
        m_tab[index]->prev = n;
    m_tab[index] = n;
    m_size++;
    m_logger.D("%s with \"%s\" insert ok",l->Name(),str);
}
char* LitStringList::Find(Label* l){
    s32 index = hash(l);
    LitStringNode* p;
    p = m_tab[index];
    while(p){
        if(l==p->m_label){
            // exist already
            return p->m_string;
        }
        p = p->next;
    }
    return 0;//not found
}
char* LitStringList::FindByLabel(Label* l){
    return Find(l);
}
Label* LitStringList::FindByString(char* str){// low performance 
    LitStringNode* p;
    for(s32 i=0;i<kLitStringList_Size;i++){
        p = m_tab[i];
        while(p){
            if(strcmp(p->m_string,str)==0){
                m_logger.D("FindByString with \"%s\" ok",str);
                return p->m_label;
            }
            p = p->next;
        }
    }
    return 0;
}
LitStringList::~LitStringList(){
    for(s32 i=0;i<kLitStringList_Size;i++){
        Clean(m_tab[i]);
    }
    
    delete[] m_tab;
}
FragList::FragList()
{
    m_tab = new FragNode*[kFragList_Size];
    for(s32 i=0;i<kFragList_Size;i++)
        m_tab[i]=0;
    m_size = 0;
}
FragList::~FragList()
{
    for(s32 i=0;i<kFragList_Size;i++)
        Clear(m_tab[i]);
    delete[] m_tab;
}
void FragList::Clear(FragNode* head)
{
    FragNode* p;
    p = head;
    while(p){
        head = head->next;
        delete p;
        p = head;
    }
}
void FragList::Insert(Label* l,Frag* frag)
{
    s32 index = hash(l);
    FragNode* p = m_tab[index];
    FragNode* n;
    while(p){
        if(strcmp(p->m_label->Name(),l->Name())==0)
            return;//already exist
        p = p->next;
    }
    n = new FragNode;
    n->m_label = l;
    n->m_frag = frag;
    n->next = m_tab[index];
    if(m_tab[index])
        m_tab[index]->prev = n;
    m_tab[index] = n;
    m_size++;
}
Frag* FragList::Find(Label* l)
{
    s32 index = hash(l);
    FragNode* p = m_tab[index];
    while(p){
        if(strcmp(p->m_label->Name(),l->Name())==0)
            return p->m_frag;
        p = p->next;
    }
    //not found
    return 0;
}
Frag* FragList::FindByLabelName(char* str)
{
    s32 index = 0;
    FragNode* p;
    for(index = 0; index < kFragList_Size; index++)
    {
        p = m_tab[index];
        while(p){
            if(strcmp(p->m_label->Name(),str)==0)
                return p->m_frag;
            p = p->next;
        }
    }
    
    
    //not found
    return 0;
}

}//namespace tiger
