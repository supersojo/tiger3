#include "reglist.h"

namespace tiger{

class RegInfo{

public:
    RegInfo(){
        m_special = new RegList;
        m_special->Insert( new RegEntry("ebp",TempLabel::NewTemp()), RegList::kRegList_Rear);
        m_special->Insert( new RegEntry("esp",TempLabel::NewTemp()), RegList::kRegList_Rear);
        
        m_arg = new RegList;
        
        m_callee = new RegList;
        
        m_caller = new RegList;
        m_caller->Insert( new RegEntry("eax",TempLabel::NewTemp()), RegList::kRegList_Rear);
        m_caller->Insert( new RegEntry("ebx",TempLabel::NewTemp()), RegList::kRegList_Rear);
        m_caller->Insert( new RegEntry("ecx",TempLabel::NewTemp()), RegList::kRegList_Rear);
        m_caller->Insert( new RegEntry("edx",TempLabel::NewTemp()), RegList::kRegList_Rear);
        
    }
    RegList* GetSpecialRegs(){return m_special;}
    RegList* GetArgRegs(){return m_arg;}
    RegList* GetCalleeRegs(){return m_callee;}
    RegList* GetCallerRegs(){return m_caller;}
    ~RegInfo(){
        delete m_special;
        delete m_arg;
        delete m_callee;
        delete m_caller;
    }
private:
    RegList* m_special;
    RegList* m_arg;
    RegList* m_callee;
    RegList* m_caller;

};

}//namespace tiger