#include <iostream>
#include "types.h"

namespace tiger{



SymNameHashTable::SymNameHashTable(){
    m_tab = new SymNameHashTableNode*[kSymNameHashTable_Size];
    for(s32 i=0;i<kSymNameHashTable_Size;i++)
        m_tab[i] = 0;
}

void SymNameHashTable::Clean()
{
    SymNameHashTableNode* p,*q;
    for(s32 i=0;i<kSymNameHashTable_Size;i++){
        p = m_tab[i];
        while(p)
        {
            q = p;
            p = p->next;
            delete q;
        }
    }    
}

s32 SymNameHashTable::hash(char* s)
{
    s32 len = strlen(s);
    s32 ret = 0;
    for(s32 i=0;i<len;i++)
        ret += *(s+i);
    ret %= kSymNameHashTable_Size;
    
    return ret;    
}

Symbol* SymNameHashTable::MakeSymbol(Symbol* s){
    TIGER_ASSERT(s!=0,"s is null");
    s32 index = hash(s->Name());
    SymNameHashTableNode* p,*n;
    p = m_tab[index];
    while(p){
        if(strcmp(p->m_name,s->Name())==0){
            return p->m_symbol;
        }
        p = p->next;
    }
    n = new SymNameHashTableNode;
    n->m_name = strdup(s->Name());// Note: memory leak
    n->m_symbol = new Symbol(s->Name());
    
    if(m_tab[index]){
        n->next = m_tab[index];
        m_tab[index]->prev = n;
    }
    
    m_tab[index] = n;
    
    return n->m_symbol;
}

SymNameHashTable::~SymNameHashTable(){
    Clean();
}

SymTab::SymTab(){
    
    m_tab = new SymTabEntryNode*[kSymTab_Size];
    for(s32 i=0;i<kSymTab_Size;i++)
        m_tab[i] = 0;
    
    m_marker = new Symbol("kSymTab_Marker");
    m_stack = new SimpleStack;
    
    m_sym_name_mapping = new SymNameHashTable;
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("SymTab");
}

s32 SymTab::hash(Symbol* key){
    u64 v = (u64)key;
    return v%kSymTab_Size;
}

void SymTab::Enter(Symbol* key,EnvEntryBase* value)
{
    m_logger.D("New SymTab entry with %s",key->Name());
    m_stack->Push(key);
    if(key==m_marker){
        m_logger.D("SymTab marker symbol push ok");
        return;
    }
    SymTabEntryNode* p,*n;
    s32 index = hash(key);
    p = m_tab[index];
    while(p){
        if(p->m_entry->GetSymbol()==key){
            m_logger.W("SymTab entry already exist with %s",key->Name());
            //return;
            /*
             * all string share Symbol, so we should insert key with many values
             */
            break;
        }
        p = p->next;
    }
    n = new SymTabEntryNode;
    n->m_entry = new SymTabEntry(key,value);
    if(m_tab[index]){
        n->next = m_tab[index];
        m_tab[index]->prev = n;
    }
    m_logger.D("New SymTab entry add ok with %s",key->Name());
    m_tab[index] = n;

}
EnvEntryBase* SymTab::Lookup(Symbol* key)
{
    m_logger.D("Lookup symbol with %s",key->Name());
    s32 index = hash(key);
    SymTabEntryNode*p;
    p = m_tab[index];
    while(p){
        if(p->m_entry->GetSymbol()==key){
            m_logger.D("Lookup symbol ok with %s",key->Name());
            return p->m_entry->GetEnvEntryBase();
        }
        p = p->next;
    }
    m_logger.W("Lookup symbol failed with %s",key->Name());
    return 0;
}

void SymTab::BeginScope()
{
        m_logger.D("BeginScope...");
        Enter(m_marker,0);
}
void SymTab::Erase(Symbol* key)
{
        m_logger.D("SymTab erase with %s",key->Name());
        s32 index = hash(key);
        SymTabEntryNode*p;
        p = m_tab[index];
        while(p){
            /*
             * only back current scope 
             */
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
                m_logger.D("SymTab erase ok with %s",key->Name());
                delete p;
                return;
            }
            p = p->next;
        }
        m_logger.W("SymTab erase failed,symbol %s not found",key->Name());
}
void SymTab::EndScope()
{
        Symbol* name;
        do{
            name = m_stack->Pop();
            if(name==m_marker){
                m_logger.D("SymTab marker symbol pop ok");
                m_logger.D("EndScope...");
                return;
            }
            /* delete from hash table */
            //std::cout<<name->Name()<<std::endl;
            Erase(name);
        }while(name!=m_marker);
        m_logger.W("Should not reach here");
}

Symbol* SymTab::MakeSymbol(Symbol* s)
{
    return m_sym_name_mapping->MakeSymbol(s);
}
Symbol* SymTab::MakeSymbolFromString(char* s)
{
    Symbol t(s);
    return MakeSymbol(&t);
}
TypeBase*  SymTab::Type(Symbol* s)
{
    EnvEntryBase* binding;
    binding = Lookup(MakeSymbol(s));
    if(binding && binding->Kind()==EnvEntryBase::kEnvEntry_Var){
        return dynamic_cast<EnvEntryVar*>(binding)->Type();
    }
    std::cout<<"wrong type "<<s->Name()<<std::endl;
    return 0;
}

SymTab::~SymTab()
{
    Clean();
    
    delete m_marker;
    delete m_stack;
    delete m_sym_name_mapping;
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
