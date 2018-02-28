#ifndef CANON_H
#define CANON_H

#include "tree.h"
#include "temp.h"

namespace tiger{

class CanonBlock{
public:
    CanonBlock(){m_list=0;m_label=0;}
    CanonBlock(StatementBaseList* list,Label* label){
        m_list = list;
        m_label = label;
    }
    StatementBaseList* GetStatementList(){return m_list;}
    Label* GetLabel(){return m_label;}
    ~CanonBlock(){
        delete m_list;
        delete m_label;
    }
private:
    StatementBaseList* m_list;
    Label* m_label;
};

class Canon{
public:
    StatementBaseList* Linearize(StatementBase* statement);
    CanonBlock* BasicBlocks(StatementBaseList* list);
private:
};	
	
} //namespace tiger

#endif
