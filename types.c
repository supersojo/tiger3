#include "types.h"


namespace tiger{

SymTab::SymTab(){
    
    m_tab = new SymTabEntryNode*[kSymTab_Size];
    for(s32 i=0;i<kSymTab_Size;i++)
        m_tab[i] = 0;
    
    m_marker = new Symbol("kSymTab_Marker");
}

s32 SymTab::hash(Symbol* key){
    s32 v = (s32)key;
    return v%kSymTab_Size;
}


SymTab::~SymTab()
{
    Clean();
    
    delete m_marker;
}

void SymTab::Clean()
{
    SymTabEntryNode* p;
    
}

}//namespace tiger