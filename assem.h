#ifndef ASSEM_H
#define ASSEM_H

#include "assem.h"
#include "frame.h"
#include "tree.h"

namespace tiger{

class InstrBase{
public:
    enum{
        kInstr_Oper,
        kInstr_Label,
        kInstr_Move,
        kInstr_Invalid
    };
    InstrBase(){m_kind = kInstr_Invalid;}
    InstrBase(s32 kind){m_kind = kind;}
    virtual void Dump(char* o){
    }
    ~InstrBase(){
    }
private:
    s32 m_kind;
    
};

class InstrOper : public InstrBase{
public:
    InstrOper():InstrBase(kInstr_Oper){
        m_str = 0;
        m_dst = 0;
        m_src = 0;
        m_jump = 0;
    }
    InstrOper(char* str,TempList* dst,TempList* src,LabelList* jump):InstrBase(kInstr_Oper){
        m_str = strdup(str);
        m_dst = dst;
        m_src = src;
        m_jump = jump;
    }
    virtual void Dump(char* o){
        s32 i_offset = 0;
        i_offset += sprintf(i_offset+o,"%s",m_str);
        if(m_dst->Size()){// only 1 support
            i_offset += sprintf(i_offset+o," dst(%s)",m_dst->Get(0)->Name());
        }
        if(m_src->Size()){// only 1 support
            i_offset += sprintf(i_offset+o," src(%s)",m_src->Get(0)->Name());
        }
    }
    ~InstrOper(){
        free(m_str);
        delete m_dst;
        delete m_src;
        delete m_jump;
    }
private:
    char* m_str;
    TempList* m_dst;
    TempList* m_src;
    LabelList* m_jump;
};
class InstrLabel : public InstrBase{
public:
    InstrLabel():InstrBase(kInstr_Label){
        m_label = 0;
    }
    InstrLabel(Label* l):InstrBase(kInstr_Label){
        m_label = l;
    }
    virtual void Dump(char* o){
    }
    ~InstrLabel(){
    }
private:
    Label* m_label;
};
class InstrMove : public InstrBase{
public:
    InstrMove():InstrBase(kInstr_Move){
        m_str = 0;
        m_dst = 0;
        m_src = 0;
    }
    InstrMove(char* str, TempList *dst,TempList *src):InstrBase(kInstr_Move){
        m_str = strdup(str);
        m_dst = dst;
        m_src = src;
    }
    virtual void Dump(char* o){
        s32 i_offset = 0;
        i_offset += sprintf(i_offset+o,"%s",m_str);
        if(m_dst->Size()){// only 1 support
            i_offset += sprintf(i_offset+o," dst(%s)",m_dst->Get(0)->Name());
        }
        if(m_src->Size()){// only 1 support
            //i_offset += sprintf(i_offset+o," src(%s)",m_src->Get(0)->Name());
        }
    }
    ~InstrMove(){
        free(m_str);
        delete m_dst;
        delete m_src;
    }
private:
    char* m_str;
    TempList* m_dst;
    TempList* m_src;
};
struct TempMapNode{
    
    TempMapNode(){
        m_temp = 0;
        m_str = 0;
        prev = next = 0;
    }
    ~TempMapNode(){
        free(m_str);
    }
    Temp* m_temp;
    char* m_str;
    TempMapNode* prev;
    TempMapNode* next;
};
class TempMapList{
public:
    TempMapList(){
        m_head = 0;
        m_top = 0;
    }
    TempMapList* LayerMap(TempMapList* over){
        m_top = over;
    }
    void Enter(Temp* temp,char* str){
        TempMapNode* n;
        TempMapNode* p;
        
        p = m_head;
        while(p){
            //already exist
            if(strcmp(p->m_temp->Name(),temp->Name())==0)
                return;
            
            p = p->next;
        }
        
        n = new TempMapNode;
        n->m_temp = temp;
        n->m_str = strdup(str);
        if(m_head)
        {
            n->next = m_head;
            m_head->prev = n;
        }
        m_head = n;
    }
    char* Look(Temp* temp){
        char* ret = 0;
        if(m_top)
            ret = m_top->Look(temp);
        if(ret)
            return ret;
        
        TempMapNode* p = m_head;
        while(p){
            if(strcmp(p->m_temp->Name(),temp->Name())==0)
                return p->m_str;
            p = p->next;
        }
        return 0;
    }
    ~TempMapList(){
        TempMapNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    TempMapNode* m_head;
    TempMapList* m_top;
};

struct InstrNode{
    InstrNode(){
        m_instr = 0;
        prev = next = 0;
    }
    ~InstrNode(){
        delete m_instr;
    }
    InstrBase* m_instr;
    InstrNode* prev;
    InstrNode* next;
};
class InstrList{
public:
    enum{
        kInstrList_Rear,
        kInstrList_Front,
        kInstrList_Invalid
    };
    InstrList(){
        m_head = 0;
        m_size = 0;
    }
    InstrNode* GetHead(){return m_head;}
    InstrBase* Get(s32 index){
        if(index>=m_size)
            return 0;
        s32 i = 0;
        InstrNode* p = m_head;
        while(p){
            if(i==index)
                return p->m_instr;
            p = p->next;
            i++;
        }
        return 0;
    }
    void Insert(InstrBase* instr,s32 dir){
        InstrNode* n;
        InstrNode* p;
        InstrNode* q;
        
        n = new InstrNode;
        n->m_instr = instr;
        
        if(dir==kInstrList_Rear){
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
        if(dir==kInstrList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    void Dump(char* o){
        InstrNode* p = m_head;
        s32 i_offset = 0;
        char s[1024]={0};
        while(p){
            if(p->m_instr)
                p->m_instr->Dump(s);
            i_offset += sprintf(i_offset+o,"%s\n",s);
            p = p->next;
        }
    }
    ~InstrList(){
        InstrNode* p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    InstrNode* m_head;
    s32 m_size;
};


class CodeGenerator{
public:
    InstrList* CodeGen(FrameBase* f,StatementBaseList* l);
private:
    void _MunchStatement(InstrList* il,StatementBase *s);
    void _MunchStatementMove(InstrList* il,StatementMove *s);
    void _MunchStatementJump(InstrList* il,StatementJump *s);
    void _MunchStatementCjump(InstrList* il,StatementCjump *s);
    Temp* _MunchExpBase(InstrList *il,ExpBase *e);
    Temp* _MunchExpBaseMem(InstrList *il,ExpBaseMem *e);
    Temp* _MunchExpBaseBinop(InstrList *il,ExpBaseBinop *e);
    Temp* _MunchExpBaseConst(InstrList *il,ExpBaseConst *e);
    Temp* _MunchExpBaseTemp(InstrList *il,ExpBaseTemp *e);
    Temp* _MunchExpBaseCall(InstrList* il,ExpBaseCall *e);
    void Munch(InstrList* il,FrameBase* f,StatementBaseList* l);
};



}// namespace tiger

#endif
