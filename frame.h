/* Coding: ANSI */
#ifndef FRAME_H
#define FRAME_H

#include "temp.h"

namespace tiger{



class AccessBase{
public:
    enum{
        kAccess_Frame,/* memory access */
        kAccess_Reg, /* register access */
        kAcces_Invalid
    };
    AccessBase(){m_kind = kAcces_Invalid;m_refcnt=0;}
    AccessBase(s32 kind){m_kind = kind;m_refcnt=0;}
    virtual s32 Kind(){return m_kind;}
    virtual s32 Refcnt(){return m_refcnt;}
    void Retain(){m_refcnt++;}
    void Release(){m_refcnt--;}
    virtual ~AccessBase(){}/* MUST be virtual */
private:
    s32 m_kind;
    s32 m_refcnt;
};
class AccessFrame:public AccessBase{
public:
    AccessFrame():AccessBase(kAccess_Frame){}
    AccessFrame(s32 offset):AccessBase(kAccess_Frame){m_offset = offset;}
    s32 Offset(){return m_offset;}
private:
    s32 m_offset;/* offset in the frame*/
};

class AccessReg:public AccessBase{
public:
    AccessReg():AccessBase(kAccess_Reg){}
    AccessReg(Temp* temp):AccessBase(kAccess_Reg){m_temp = temp;}
    Temp* GetTemp(){return m_temp;}
private:
    Temp* m_temp;/* allocateed from temp pool. it is managed bet the pool. Don't delete it in the destructor */
};

struct AccessNode{
    AccessNode(){
        m_access = 0;
        prev = next = 0;
    }
    ~AccessNode(){
        if(m_access){
            m_access->Release();
            if(m_access->Refcnt()==0){
                delete m_access;
                m_access = 0;
            }
        }
    }
    /* members */
    AccessBase* m_access;
    AccessNode* prev;
    AccessNode* next;
};
class AccessList{
public:
    enum{
        kAccessList_Rear,
        kAccessList_Front,
        kAccessList_Invalid
    };
    AccessList(){m_head=0;}
    AccessList(AccessNode* head){m_head=head;}
    AccessNode* GetHead(){return m_head;}
    void Insert(AccessBase* access, s32 dir){
        AccessNode* n;
        AccessNode* p;
        AccessNode* q;
        n = new AccessNode;
        n->m_access= access;
        if(dir==kAccessList_Rear){
            p = m_head;
            q = m_head;
            while(p){
                q = p;
                p = p->next;
            }
            if(q){
                q->next = n;
                n->prev = q;
            }else{/* m_list is empty*/
                m_head = n;
            }
        }
        if(dir==kAccessList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }

    }
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
    enum{
        kBoolList_Rear,
        kBoolList_Front,
        kBoolList_Invalid
    };
    BoolList(){m_head=0;}
    void Insert(s32 v,s32 dir){
        BoolNode* n;
        n = new BoolNode;
        n->m_kind = v;
        if(dir==kBoolList_Rear)
        {
            BoolNode*p;
            BoolNode*q;
            p = m_head;
            q = m_head;
            while(p){
                q = p;
                p = p->next;
            }
            if(q){
                q->next = n;
                n->prev = q;
            }else{
                m_head = n;
            }
        }
        if(dir==kBoolList_Front)
        {
            n->next = m_head;
            if(m_head)
            {
                m_head->prev = n;
            }
            m_head = n;
        }
    }
    void NewHead(BoolNode* bn){
        bn->next = m_head;
        if(m_head){
            m_head->prev = bn;
        }
        m_head = bn;
    }
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
    
    FrameBase()
    {
        m_kind = kFrame_Invalid;
        m_formals=0;
        m_escapes=0;
        m_offset=0;
    }
    FrameBase(s32 kind){
        m_kind = kind;
        m_formals=new AccessList;
        m_escapes=new BoolList;
        m_locals=new AccessList;
        m_offset=0;
    }
    
    virtual AccessList* GetFormals(){return m_formals;}
    virtual BoolList*   GetEscapes(){return m_escapes;}
    virtual s32         Kind(){return m_kind;}
    
    virtual ~FrameBase(){
        delete m_formals;
        delete m_escapes;
        delete m_locals;
    }
    virtual AccessBase* AllocLocal(s32 escape){
        AccessBase* ret = 0;
        if(escape){
            ret = new AccessFrame(m_offset);
            m_offset = m_offset - 4;
        }else
            ret = new AccessReg(TempLabel::NewTemp());
        /* record the allocation */
        ret->Retain();
        m_locals->Insert(ret,AccessList::kAccessList_Rear);
    }
public:
    s32 m_kind;
    s32 m_offset;/* current frame offset */
    AccessList* m_formals;
    BoolList*   m_escapes;

    AccessList* m_locals;// all accesses from the frame
    
};
class FrameX86:public FrameBase{
public:
    FrameX86():FrameBase(kFrame_X86){}

};

class Level{
public:
    Level(){m_frame=0;m_parent=0;}
    Level(Level* parent,FrameBase* f){
        m_parent = parent;
        m_frame=f;
    }
    FrameBase* Frame(){return m_frame;}
    Level* Parent(){return m_parent;}
    ~Level(){delete m_frame;}
private:
    FrameBase* m_frame;//
    Level* m_parent;
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
class LevelManager{
public:
    LevelManager(){m_head = 0;}
    void NewLevel(Level* l){
        LevelNode* n;
        n->m_level = l;
        n->next = m_head;
        if(m_head)
            m_head->prev = n;
        m_head = n;
    }
    ~LevelManager(){
        LevelNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }

    }
private:
    LevelNode* m_head;

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
