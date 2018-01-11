#include <iostream>
#include "types.h"

namespace tiger{

SymTab::SymTab(){
    
    m_tab = new SymTabEntryNode*[kSymTab_Size];
    for(s32 i=0;i<kSymTab_Size;i++)
        m_tab[i] = 0;
    
    m_marker = new Symbol("kSymTab_Marker");
    m_stack = new SimpleStack;
}

s32 SymTab::hash(Symbol* key){
    u64 v = (u64)key;
    return v%kSymTab_Size;
}

void SymTab::Enter(Symbol* key,EnvEntryBase* value)
{
    m_stack->Push(key);
    if(key==m_marker){
        return;
    }
    SymTabEntryNode* p,*n;
    s32 index = hash(key);
    p = m_tab[index];
    while(p){
        if(p->m_entry->GetSymbol()==key)
            return;
        p = p->next;
    }
    n = new SymTabEntryNode;
    n->m_entry = new SymTabEntry(key,value);
    if(m_tab[index]){
        n->next = m_tab[index];
        m_tab[index]->prev = n;
    }
    m_tab[index] = n;

}
EnvEntryBase* SymTab::Lookup(Symbol* key)
{
    s32 index = hash(key);
    SymTabEntryNode*p;
    p = m_tab[index];
    while(p){
        if(p->m_entry->GetSymbol()==key)
            return p->m_entry->GetEnvEntryBase();
        p = p->next;
    }
    return 0;
}

void SymTab::BeginScope()
{
        Enter(m_marker,0);
}
void SymTab::Erase(Symbol* key)
{
        s32 index = hash(key);
        SymTabEntryNode*p;
        p = m_tab[index];
        while(p){
            if(p->m_entry->GetSymbol()==key){
                if(p->prev==0){
                    m_tab[index]=p->next;
                    if(m_tab[index])
                        m_tab[index]->prev=0;
                }else{
                    p->prev->next = p->next;
                    if(p->next){
                        p->next->prev = p->prev;
                    }
                } 
                delete p;
                return;
            }
            p = p->next;
        }
}
void SymTab::EndScope()
{
        Symbol* name;
        do{
            name = m_stack->Pop();
            if(name==m_marker)
                return;
            /* delete from hash table */
            std::cout<<name->Name()<<std::endl;
            Erase(name);
        }while(name!=m_marker);
}


SymTab::~SymTab()
{
    Clean();
    
    delete m_marker;
    delete m_stack;
}

void SymTab::Clean()
{
    SymTabEntryNode* p,*q;
    for(s32 i=0;i<kSymTab_Size;i++){
        p = m_tab[i];
        while(p)
        {
            q = p;
            p = p->next;
            delete q;
        }
    }    
}

}//namespace tiger
