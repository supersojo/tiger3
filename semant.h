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
    s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
class TreeBaseEx:public TreeBase{
public:
    TreeBaseEx():TreeBase(kTreeBase_Ex){m_exp=0;}
    TreeBaseEx(ExpBase* exp):TreeBase(kTreeBase_Ex){m_exp=exp;}
    ExpBase* GetExp(){return m_exp;}
    ~TreeBaseEx(){}
private:
    ExpBase* m_exp;/* ??? */
};
class TreeBaseNx:public TreeBase{
public:
    TreeBaseNx():TreeBase(kTreeBase_Nx){m_statement=0;}
    TreeBaseNx(StatementBase* statement):TreeBase(kTreeBase_Nx){m_statement=statement;}
    StatementBase* GetStatement(){return m_statement;}
    ~TreeBaseNx(){}
private:
    StatementBase* m_statement;/* ??? */
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
    void DoPatch(Label* l){
        PatchNode* p;
        p = m_head;
        while(p){
            *(p->m_alabel) = l;
            p = p->next;
        }
    }
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
        delete m_trues;
        delete m_falses;
    }
private:
    StatementBase* m_statement;/* ??? */
    PatchList* m_trues;
    PatchList* m_falses;
};
//used for type check
class ExpBaseTy{
public:
    ExpBaseTy(){m_type=0;m_tree=0;}
    ExpBaseTy(TypeBase* ty,ExpBase* tree){m_type=ty;m_tree = tree;}
    TypeBase* Type(){return m_type;}
    ExpBase*  Tree(){return m_tree;}
    ~ExpBaseTy(){
        //delete m_type;
        delete m_tree;
    }
private:
    TypeBase* m_type;
    ExpBase*  m_tree;
};

class Translator{
public:
    Translator();
    ExpBaseTy*  TransExp(SymTab* venv,SymTab* tenv,Level* level,Exp* exp);
    ExpBaseTy*  TransVar(SymTab* venv,SymTab* tenv,Level* level,Var* var);
    void        TransDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec);
    TypeBase*   TransTy(SymTab* tenv,Level* level,Ty* ty);
    Level*      OuterMostLevel();
    ~Translator();
private:
    void           TransFunctionDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec);
    TypeFieldList* MakeFormalsList(SymTab* venv,SymTab* tenv,Level* level,FieldList* params);
    void           TransTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec);
    FrameBase*     MakeNewFrame(FunDec* fundec);
    
    LevelManager* m_level_manager;
    LoggerStdio m_logger; 
    Level* m_outer_most_level;
};

}//namespace tiger

#endif
