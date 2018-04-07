/* Coding: ANSI */
#ifndef FRAME_H
#define FRAME_H
#include <iostream>
#include "temp.h"
#include "tree.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
//#include "types.h" // for EnvEntryBase
namespace tiger{

class EnvEntryBase;


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
    AccessList(){m_head=0;m_size=0;}
    /*AccessList(AccessNode* head){m_head=head;m_size=0;}*/
    AccessNode* GetHead(){return m_head;}
    AccessBase* Get(s32 index){
        if(m_size==0)
            return 0;
        if(index<0)
            return 0;
        if(index>=m_size)//overflow
            return 0;
        s32 i=0;
        AccessNode*p=m_head;
        while(p){
            if(i==index)
                return p->m_access;
            p = p->next;
            i++;
        }
        return 0;
    }
    void Insert(AccessBase* access, s32 dir){
        AccessNode* n;
        AccessNode* p;
        AccessNode* q;
        
        n = new AccessNode;
        
        n->m_access = access;
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
        m_size++;

    }
    s32 Size(){return m_size;}
    ~AccessList(){
        
        AccessNode* p;
        p = m_head;
        s32 i=0;
        while(p){
            m_head = m_head->next;
            delete p;
            i++;
            p = m_head;
        }
        
    }
private:
    AccessNode* m_head;
    s32 m_size;/* element num */
};
struct BoolNode{
    enum{
        kBool_True,
        kBool_False,
        kBool_Invalid
    };
    BoolNode(){
        m_kind = kBool_Invalid;
        prev = next = 0;
    }
    BoolNode(s32 v){
        m_kind = v;
        prev = next = 0;
    }
    /* members */
    s32 m_kind;
    BoolNode* prev;
    BoolNode* next;
};

class BoolListIterator;

class BoolList{
friend class BoolListIterator;
public:
    enum{
        kBoolList_Rear,
        kBoolList_Front,
        kBoolList_Invalid
    };
    BoolList(){m_head=0;m_size=0;}
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
        m_size++;
    }
    s32 Size(){return m_size;}
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
    s32 m_size;
};
class BoolListIterator{
public:
    BoolListIterator(){m_cur = 0;m_origin = 0;}
    BoolListIterator(BoolList* l){
        m_cur = l->m_head;
        m_origin = l->m_head;
    }
    s32 Next(){
        BoolNode* ret = 0;
        if(m_cur){
            ret = m_cur;
            m_cur = m_cur->next;
        }
        if(ret)
            return ret->m_kind;
        return BoolNode::kBool_Invalid;
    }
    void Reset(){
        m_cur = m_origin;    
    }
private:
    BoolNode* m_cur;
    BoolNode* m_origin;
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
        m_size = 0;
    }
    FrameBase(s32 kind){
        m_kind = kind;
        m_formals=new AccessList;
        m_escapes=new BoolList;
        m_refill_list = new ExpBaseList(1);//only reference to expbase ,not free them
        m_locals=new AccessList;
        m_offset=0;
        m_size = 0;
    }
    AccessBase* GetAccess(s32 index){
        return m_locals->Get(index);
    }
    virtual AccessList* GetFormals(){return m_formals;}
    virtual BoolList*   GetEscapes(){return m_escapes;}
    ExpBaseList*  GetRefillList(){return m_refill_list;}
    virtual s32         Kind(){return m_kind;}
    
    virtual ~FrameBase(){
        if(m_formals)
            delete m_formals;
        if(m_escapes)
            delete m_escapes;
        if(m_locals)
            delete m_locals;
        if(m_refill_list)
            delete m_refill_list;
    }
    virtual AccessBase* AllocLocal(s32 escape){
        AccessBase* ret = 0;
        if(escape){
            ret = new AccessFrame(m_offset);
            m_offset = m_offset + 4;
            m_size += 4;
            
            //update expbase in m_refill_list;
            Refill();
            
        }else
            ret = new AccessReg(TempLabel::NewTemp());
        /* record the allocation */
        ret->Retain();
        m_locals->Insert(ret,AccessList::kAccessList_Rear);
        
        
        return ret;/* MARK */
    }
    s32 Size(){return m_size;}
public:
    s32 m_kind;
    s32 m_offset;/* current frame offset */
    s32 m_size;
    AccessList* m_formals;
    BoolList*   m_escapes;
    ExpBaseList* m_refill_list;
    void Refill(){
        ExpBaseNode* p;
        p = m_refill_list->GetHead();
        while(p){
            TIGER_ASSERT(p->m_exp->Kind()==ExpBase::kExpBase_Const, (char*)"refill item should be ExpBaseConst");
            dynamic_cast<ExpBaseConst*>(p->m_exp)->SetValue(Size());
            p = p->next;
        }
    }

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
    void SetEnvEntry(EnvEntryBase* env_entry)
    {
        m_env_entry = env_entry;
    }
    EnvEntryBase* GetEnvEntry(){// for access escape list
        return m_env_entry;
    }
    void SetFunc(llvm::Function* f){
        m_func = f;
    }
    llvm::Function* GetFunc(){
        return m_func;
    }
    ~Level(){
        if(m_frame)
            delete m_frame;
    }
private:
    FrameBase* m_frame;//
    Level* m_parent;
    EnvEntryBase* m_env_entry;//only EnvEntryFun used !!
    llvm::Function* m_func;
};

struct LevelNode{
    LevelNode(){
        m_level=0;
        prev=next=0;
    }
    ~LevelNode(){
        if(m_level)
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
        
        n = new LevelNode;/* MARK */
        
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
    VarAccess(Level* level,AccessBase* access){m_level=level;m_access=access;}
    Level* GetLevel(){return m_level;}
    AccessBase* GetAccess(){return m_access;}
    ~VarAccess(){}
private:
    Level* m_level;// managed by level manager
    AccessBase * m_access;// managed by level
};

}// namespace tiger


#endif
