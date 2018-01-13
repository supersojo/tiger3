/* Coding: ANSI */
#ifndef FRAME_H
#define FRAME_H

#include "temp.h"

namespace tiger{



class AccessBase{
public:
    enum{
        kAccess_Frame,
        kAccess_Reg,
        kAcces_Invalid
    };
    AccessBase(){m_kind = kAcces_Invalid;}
    AccessBase(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
    virtual ~AccessBase(){}
private:
    s32 m_kind;
};
class AccessFrame:public AccessBase{
public:
    AccessFrame():AccessBase(kAccess_Frame){}
    AccessFrame(s32 offset):AccessBase(kAccess_Frame){m_offset = offset;}
    s32 Offset(){return m_offset;}
private:
    s32 m_offset;
};

class AccessReg:public AccessBase{
public:
    AccessReg():AccessBase(kAccess_Reg){}
    AccessReg(Temp* temp):AccessBase(kAccess_Reg){m_temp = temp;}
    Temp* GetTemp(){return m_temp;}
private:
    Temp* m_temp;
};

struct AccessNode{
    AccessNode(){
        m_access = 0;
        prev = next = 0;
    }
    ~AccessNode(){
        delete m_access;
    }
    /* members */
    AccessBase* m_access;
    AccessNode* prev;
    AccessNode* next;
};
class AccessList{
public:
    AccessList(){m_head=0;}
    AccessList(AccessNode* head){m_head=head;}
    AccessNode* GetHead(){return m_head;}
    ~AccessList(){
        AccessNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    AccessNode* m_head;
};
struct BoolNode{
    enum{
        kBool_True,
        kBool_False,
        kBool_Invalid
    };
    BoolNode(){
        m_kind = kBool_Invalid;
    }
    BoolNode(s32 v){
        m_kind = v;
    }
    /* members */
    s32 m_kind;
    BoolNode* prev;
    BoolNode* next;
};
class BoolList{
public:
    BoolList(){m_head=0;}
    BoolList(BoolNode* head){m_head = head;}
    BoolNode* GetHead(){return m_head;}
    ~BoolList(){
        BoolNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    BoolNode* m_head;
};
class FrameBase{
public:
    enum{
        kFrame_X86,
        kFrame_Arm,
        kFrame_Mips,
        kFrame_X86_64,
        kFrame_Invalid
    };
    
    FrameBase(){m_kind = kFrame_Invalid;m_formals=0;m_escapes=0;m_offset=0;}
    FrameBase(s32 kind){m_kind = kind;m_formals=0;m_escapes=0;m_offset=0;}
    
    virtual AccessList* GetFormals(){return m_formals;}
    virtual BoolList*   GetEscaps(){return m_escapes;}
    virtual void        SetFormals(AccessList* list){m_formals = list;}
    virtual void        SetEscapes(BoolList* list){m_escapes = list;}
    virtual s32         Kind(){return m_kind;}
    
    virtual ~FrameBase(){
        delete m_formals;
        delete m_escapes;
    }
    virtual AccessBase* AllocLocal(s32 escape){
        if(escape){
            return new AccessFrame(m_offset);
            m_offset = m_offset - 4;
        }else
            return new AccessReg(TempLabel::NewTemp());
    }
public:
    s32 m_kind;
    s32 m_offset;/* current frame offset */
    AccessList* m_formals;
    BoolList* m_escapes;
    
    AccessList* m_list;//track all local variables 
};
class FrameX86:public FrameBase{
public:
    FrameX86():FrameBase(kFrame_X86){}

};

class Level{
public:
    Level(){m_frame=0;}
    Level(FrameBase* f){m_frame=f;}
    FrameBase* Frame(){return m_frame;}
    ~Level(){delete m_frame;}
private:
    FrameBase* m_frame;//
};

struct LevelNode{
    LevelNode(){
        m_level=0;
        prev=next=0;
    }
    ~LevelNode(){
        delete m_level;
    }
    Level* m_level;
    LevelNode* prev;
    LevelNode* next;
};

class VarAccess{
public:
    VarAccess(){m_level=0;m_access=0;}
    VarAccess(LevelNode* level,AccessBase* access){m_level=level;m_access=access;}
    Level* GetLevel(){return m_level->m_level;}
    AccessBase* GetAccess(){return m_access;}
    ~VarAccess(){}
private:
    LevelNode* m_level;
    AccessBase * m_access;
};

}// namespace tiger


#endif
