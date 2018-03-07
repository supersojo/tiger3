/* Coding: ANSI */
#ifndef SEMANT_H
#define SEMANT_H

#include "tiger_type.h"
#include "tiger_log.h"
#include "types.h"// symbol table 
#include "absyn.h"// sbstract syntax tree
#include "tree.h"//immediate representation tree
#include "frame.h"

namespace tiger{

struct PatchNode{
    PatchNode(){
        m_alabel = 0;
        prev = next = 0;
    }
    /* members */
    Label** m_alabel;/* label address */
    PatchNode* prev;
    PatchNode* next;
};
// label patch utils
class PatchList{
public:
    enum{
        kPatchList_Rear,
        kPatchList_Front,
        kPatchList_Invalid
    };
    PatchList(){m_head = 0;m_size=0;}
    void Insert(Label** alabel,s32 dir){
        PatchNode* n;
        PatchNode* p;
        PatchNode* q;
        
        n = new PatchNode;
        n->m_alabel = alabel;
        if(dir==kPatchList_Rear){
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
        if(dir==kPatchList_Front){
            n->next = m_head;
            if(m_head)
                m_head->prev = n;
            m_head = n;
        }
        m_size++;
    }
    // do real patch work
    // use label l to patch label node
    void DoPatch(Label* l){
        PatchNode* p;
        p = m_head;
        while(p){
            *(p->m_alabel) = l;
            p = p->next;
        }
    }
    // patch labels count
    s32 Size(){return m_size;}
    ~PatchList(){
        PatchNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    PatchNode* m_head;
    s32 m_size;
};

/* 
 * function defination 
 * store function tree representation
*/
class Frag{
public:
    Frag(){m_statement = 0;m_frame=0;}
    Frag(StatementBase* statement,FrameBase* frame){
        m_statement = statement;
        m_frame = frame;
    }
    StatementBase* GetStatement(){return m_statement;}
    FrameBase* Frame(){return m_frame;}
    ~Frag(){
        delete m_statement;
    }
    
private:
    StatementBase* m_statement;
    FrameBase* m_frame;/* managed by level */
};
struct FragNode{
    FragNode(){
        m_frag = 0;
        prev = next = 0;
    }
    ~FragNode(){
        delete m_frag;
    }
    /* members */
    Label* m_label;/* key */
    Frag* m_frag;
    FragNode* prev;
    FragNode* next;
};

struct LitStringNode{
    LitStringNode(){
        m_label = 0;
        m_string = 0;
        prev = next = 0;
    }
    ~LitStringNode(){
        free(m_string);
    }
    /* members */
    Label* m_label;/* managed by TempLabel */
    char*  m_string;
    LitStringNode* prev;
    LitStringNode* next;
    
};
/* hash table for label/string mapping 
 * 
 * string vars allocated by c functions not tiger itself
*/
class LitStringList{
public:
    enum{
        kLitStringList_Size=32
    };
    LitStringList();
    ~LitStringList();
    void Insert(Label* l,char* str);
    char* Find(Label* l);
    char* FindByLabel(Label* l);
    Label* FindByString(char* str);
private:
    s32 hash(Label* l){
        return reinterpret_cast<u64>(l)%kLitStringList_Size;
    }
    void Clean(LitStringNode* list){
        LitStringNode* p;
        p = list;
        while(p){
            list = list->next;
            delete p;
            p = list;
        }
    }
    LitStringNode** m_tab;
    s32 m_size;

    LoggerStdio m_logger; 
};

class FragList{
public:
    enum{
        kFragList_Size=32
    };
    FragList();
    ~FragList();
    void Insert(Label* l,Frag* frag);
    Frag* Find(Label* l);
    Frag* FindByLabelName(char* str);
    s32 Size(){return m_size;}
private:
    void Clear(FragNode* head);
    s32 hash(Label* l){
        return ( reinterpret_cast<u64>(l) )%kFragList_Size;
    }
    FragNode** m_tab;
    s32 m_size;
};
    
    
}//namespace tiger

#endif
