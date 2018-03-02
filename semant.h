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

class TreeBaseCx;
// tree representation
class TreeBase{
public:
    enum{
        kTreeBase_Ex,/* with value*/
        kTreeBase_Nx,/* without value, such as while ... */
        kTreeBase_Cx,/* condition exp */
        kTreeBase_Invalid
    };
    TreeBase(){m_kind = kTreeBase_Invalid;}
    TreeBase(s32 kind){m_kind = kind;}
    
    static ExpBase*       UnEx(TreeBase* tree);
    static StatementBase* UnNx(TreeBase* tree);
    static TreeBaseCx*    UnCx(TreeBase* tree);
    
    virtual ~TreeBase(){}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
class TreeBaseEx:public TreeBase{
public:
    TreeBaseEx():TreeBase(kTreeBase_Ex){m_exp=0;}
    TreeBaseEx(ExpBase* exp):TreeBase(kTreeBase_Ex){m_exp=exp;}
    ExpBase* GetExp(){return m_exp;}
    ~TreeBaseEx(){
        //delete m_exp;
    }
private:
    ExpBase* m_exp;/* managed by tree */
};
class TreeBaseNx:public TreeBase{
public:
    TreeBaseNx():TreeBase(kTreeBase_Nx){m_statement=0;}
    TreeBaseNx(StatementBase* statement):TreeBase(kTreeBase_Nx){m_statement=statement;}
    StatementBase* GetStatement(){return m_statement;}
    ~TreeBaseNx(){
        //delete m_statement;
    }
private:
    StatementBase* m_statement;/* managed by tree */
};
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
class TreeBaseCx:public TreeBase{
public:
    TreeBaseCx():TreeBase(kTreeBase_Cx){m_statement=0;}
    TreeBaseCx(StatementBase* statement,PatchList* ts,PatchList* fs):TreeBase(kTreeBase_Cx){
        m_statement=statement;
        m_trues = ts;
        m_falses = fs;
    }
    StatementBase* GetStatement(){return m_statement;}
    PatchList* GetTrues(){return m_trues;}
    PatchList* GetFalses(){return m_falses;}
    ~TreeBaseCx(){
        //delete m_statement;
        delete m_trues;
        delete m_falses;
    }
private:
    StatementBase* m_statement;/* managed by tree */
    PatchList* m_trues;
    PatchList* m_falses;
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


//used for type check
class ExpBaseTy{
public:
    ExpBaseTy(){m_type=0;m_tree=0;}
    ExpBaseTy(TypeBase* ty,TreeBase* tree){m_type=ty;m_tree = tree;}
    TypeBase* Type(){return m_type;}
    TreeBase* Tree(){return m_tree;}
    ~ExpBaseTy(){
        //delete m_type;
        delete m_tree;
    }
private:
    TypeBase* m_type;// managed by type table, not here
    TreeBase*  m_tree;// ???
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
class FragList;
// translator absyn to tree
class Translator{
public:
    Translator();
    
    // exp translate
    ExpBaseTy*  TransExp(SymTab* venv,SymTab* tenv,Level* level,Exp* exp,Label* done_label);
    
    // var access translate
    ExpBaseTy*  TransVar(SymTab* venv,SymTab* tenv,Level* level,Var* var,Label* done_label);
    
    // dec translate
    TreeBase*   TransDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label);
    
    // type translate
    TypeBase*   TransTy(SymTab* tenv,Level* level,Ty* ty);
    
    Level*      OuterMostLevel();
    
    // frame pointer
    Temp* FP(){
        if(m_fp==0)
            m_fp = TempLabel::NewTemp();
        return m_fp;
    }
    
    // traverse the tree
    void Traverse(TreeBase* tree);
    
    ~Translator();
    
    //traverse utils
    void TraverseEx(ExpBase* exp);
    void TraverseNx(StatementBase* statement);
    void TraverseCx(StatementBase* statement);
    
    // traverse function flag utils
    void TraverseFragList();
    
private:
    void           TransFunctionDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label);
    TypeFieldList* MakeFormalsList(SymTab* venv,SymTab* tenv,Level* level,FieldList* params);
    void           TransTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec);
    
    // array or record 
    ExpBaseTy*     TransArrayExp(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Exp* exp,Label* done_label);
    ExpBaseTy*     TransRecordExp(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Exp* exp,Label* done_label);
    
    FrameBase*     MakeNewFrame(FunDec* fundec);
    
    // for exp to let exp
    LetExp*        For2Let(ForExp* exp);
    
    
    void           ReleaseTree(TreeBase* tree){
        switch(tree->Kind()){
            case TreeBase::kTreeBase_Ex:
                delete dynamic_cast<TreeBaseEx*>(tree)->GetExp();
                break;
            case TreeBase::kTreeBase_Nx:
                delete dynamic_cast<TreeBaseNx*>(tree)->GetStatement();
                break;
            case TreeBase::kTreeBase_Cx:
                delete dynamic_cast<TreeBaseCx*>(tree)->GetStatement();
                break;
        }
    }
    
    LevelManager* m_level_manager; //level management
    LoggerStdio m_logger; // log util
    Level* m_outer_most_level;
    
    LitStringList* m_lit_string_list;// manage all literal strings
    FragList* m_frag_list;// manage all function frags
    // frame pointer
    Temp* m_fp;
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
    s32 Size(){return m_size;}
    void Walk(Translator* trans) {
        FragNode* p;
        for(s32 i=0;i<kFragList_Size;i++){
            p = m_tab[i];
            while(p){
                trans->TraverseNx( p->m_frag->GetStatement() );
                p = p->next;
            }
        }
    }
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
