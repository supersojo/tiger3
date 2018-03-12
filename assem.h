#ifndef ASSEM_H
#define ASSEM_H

#include <list>
#include "assem.h"
#include "frame.h"
#include "tree.h"
#include "tree_gen.h"
/*

 For each statement in statementlist
 Select proper instruction for it
 Here first select instruction then select reg
 
 Generating format string
 
 For example
 
 LABEL(L005)  => L005:
 MOV(TEMP(T002),CONST(1)) => "mov $1,%s"    
 MOV(TEMP(T003),CONST(2)) => "mov $2,%s"
 MOV(TEMP(T004),CONST(0)) => "mov $0,%s"
 MOV(TEMP(T005),CONST(1)) => "mov $1,%s"
 MOV(TEMP(T006),CONST(10))=> "mov $10,%s"
 JUMP(NAME(L000),(L000))  => jmp L000
 LABEL(L000)              => L000:
 MOV(TEMP(T007),CONST(1)) => "mov $1,%s"
 CJUMP OP TEMP(T005),TEMP(T006),L003,L004 => cmp %s,%s jle L003
 LABEL(L004)                              =>L004:
 MOV(TEMP(T007),CONST(0)) =>"mov $0,%s"
 JUMP(NAME(L003),(L003))  => jmp L003
 LABEL(L003)              => L003:
 CJUMP OP TEMP(T007),CONST(1),L001,L002 =>cmp %s,$1 je L001
 LABEL(L002)             =>L002:
 JUMP(NAME(done),(done)) =>jmp done
 LABEL(L001)             =>L001:
 MOV(TEMP(T002),BINOP(ADD,TEMP(T002),TEMP(T005))) => add %s,%s
 MOV(TEMP(T005),BINOP(ADD,TEMP(T005),CONST(1)))   =>add %s,%s
 JUMP(NAME(L000),(L000)) => jmp L000
                         => done:



 mov
*/
namespace tiger{

class TempMapList;
class ColorList;
class InstrBase{
public:
    enum{
        kInstr_Oper, // +,-,*/, etc
        kInstr_Label,//label
        kInstr_Move, //move
        kInstr_Invalid
    };
    InstrBase(){m_kind = kInstr_Invalid;}
    InstrBase(s32 kind){m_kind = kind;}
    virtual void Dump(char* o){
    }
    virtual void Output(TempMapList* map,char* o){
        // parse m_str
    }
    virtual s32 Kind(){return m_kind;}
    virtual TempList* Dst(){return 0;}
    virtual TempList* Src(){return 0;}
    virtual LabelList* Jump(){return 0;}
    virtual ~InstrBase(){
    }
    virtual void InstrStr(char* o){
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
        if(m_dst->Size()==0 && m_src->Size()==0){
            sprintf(o,"%s",m_str);
        }
        if(m_dst->Size()==0 && m_src->Size()!=0){
            s32 i = 0;
            if(m_src->Size()==1)
                sprintf(o,m_str,m_src->Get(0)->Name());
            if(m_src->Size()==2)
                sprintf(o,m_str,m_src->Get(0)->Name(),m_src->Get(1)->Name());
        }
        if(m_dst->Size()!=0 && m_src->Size()==0){
            sprintf(o,m_str,m_dst->Get(0)->Name());
        }
        if(m_dst->Size()!=0 && m_src->Size()!=0){
            sprintf(o,(const char*)m_str,m_src->Get(0)->Name(),m_dst->Get(0)->Name());
        }
    }
    virtual void Output(TempMapList* map,char* o);
    TempList* Dst(){return m_dst;}
    TempList* Src(){return m_src;}
    void InstrStr(char* o){
        char* s = m_str;
        while(*s!=' '){
            *o = *s;
            o++;
            s++;
        }
        *o = '\0';
    }
    LabelList* Jump(){return m_jump;}
    virtual ~InstrOper(){
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
        m_str = 0;
    }
    InstrLabel(char* str,Label* l):InstrBase(kInstr_Label){
        m_str = strdup(str);
        m_label = l;
    }
    Label* GetLabel(){return m_label;}
    virtual void Dump(char* o){
        sprintf(o,"%s",m_str);
    }
    virtual void Output(TempMapList* map,char* o){
        // parse m_str
        sprintf(o,"%s",m_str);
    }
    virtual ~InstrLabel(){
        free(m_str);
    }
    virtual void InstrStr(char* o){
    }
private:
    char* m_str;
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
        if(m_dst->Size()==0 && m_src->Size()==0){
            sprintf(o,"%s",m_str);
        }
        if(m_dst->Size()==0 && m_src->Size()!=0){
            sprintf(o,m_str,m_src->Get(0)->Name());
        }
        if(m_dst->Size()!=0 && m_src->Size()==0){
            sprintf(o,m_str,m_dst->Get(0)->Name());
        }
        if(m_dst->Size()!=0 && m_src->Size()!=0){
            sprintf(o,m_str,m_src->Get(0)->Name(),m_dst->Get(0)->Name());
        }
    }
    virtual void Output(TempMapList* map,char* o);
    TempList* Dst(){return m_dst;}
    TempList* Src(){return m_src;}
    virtual ~InstrMove(){
        free(m_str);
        delete m_dst;
        delete m_src;
    }
    virtual void InstrStr(char* o){
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
/*
TempMapList for register allocation.
one map can over other map
*/
class TempMapList{
public:
    TempMapList(){
        m_head = 0;
        m_top = 0;
    }
    TempMapList* LayerMap(TempMapList* over){
        m_top = over;
    }
    /*
    {t0->sp,t1->fp}<----{t2->r1,t3->r2}
                             ||
                             cur map
    */
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
    s32 Size(){return m_size;}
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
    void Output(TempMapList* map,char* o);
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
// assembly language generator
class CodeGenerator{
public:
    CodeGenerator(){
        m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
        m_logger.SetModule("code gen");
        m_temp_map_list = 0;
    }
    InstrList* CodeGen(FrameBase* f,StatementBaseList* l);
    void Output(TempMapList* map,InstrList* il,FILE* f){
        //
        //
        char buf[1024]={0};
        
        il->Output(map,buf);
        
        //prologue
        fprintf(f,"#\tcode generated by tiger compiler v0.1\n");
        fprintf(f,".globl _tiger_entry\n");
        fprintf(f,"_tiger_entry:\n");
        
        
        fprintf(f,"%s",buf);
        
        //epilogue
        fprintf(f,"done:\n");
        fprintf(f,"jmp _tiger_resume\n");
    }
private:
    void _MunchStatement(InstrList* il,StatementBase *s);
    void _MunchStatementMove(InstrList* il,StatementMove *s);
    void _MunchStatementExp(InstrList* il,StatementExp *s);
    void _MunchStatementJump(InstrList* il,StatementJump *s);
    void _MunchStatementCjump(InstrList* il,StatementCjump *s);
    Temp* _MunchExpBase(InstrList *il,ExpBase *e);
    Temp* _MunchExpBaseMem(InstrList *il,ExpBaseMem *e);
    Temp* _MunchExpBaseBinop(InstrList *il,ExpBaseBinop *e);
    Temp* _MunchExpBaseConst(InstrList *il,ExpBaseConst *e);
    Temp* _MunchExpBaseTemp(InstrList *il,ExpBaseTemp *e);
    Temp* _MunchExpBaseCall(InstrList* il,ExpBaseCall *e);
    TempList* _MunchArgs(ExpBaseList* el);
    void Munch(InstrList* il,FrameBase* f,StatementBaseList* l);
    
    LoggerStdio m_logger;
    TempMapList* m_temp_map_list;
};



}// namespace tiger

#endif
