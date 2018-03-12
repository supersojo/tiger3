/* Coding: Ansi */
#ifndef TREE_GEN_H
#define TREE_GEN_H

#include "tiger_log.h"
#include "tree.h"
#include "semant.h"
#include "assem.h"

/*
 tree generating need type info.
 type a = {x:int,y:string}
 type b = {x:int,y:a}
 var x=b{x=0,y=a{x=1,y="he"}}
 x.y.x
 when translate x.y.x, need to know x's type so we can find y's type.
 
 when a var declaration, we need it's type in venv table
*/
namespace tiger{
    
struct TreeGenResult{
    enum{
        kTreeGenResult_Ex,
        kTreeGenResult_Nx,
        kTreeGenResult_Cx,
        kTreeGenResult_Invalid
    };
    TreeGenResult(){
        m_kind = kTreeGenResult_Invalid;
        m_exp = 0;
        m_statement = 0;
        m_trues = 0;
        m_falses = 0;
    }
    TreeGenResult(TypeBase* ty,ExpBase* exp){
        m_type = ty;
        m_kind = kTreeGenResult_Ex;
        m_exp = exp;
        m_statement = 0;
        m_trues = 0;
        m_falses = 0;
    }
    TreeGenResult(TypeBase* ty,StatementBase* statement){
        m_type = ty;
        m_kind = kTreeGenResult_Nx;
        m_statement = statement;
        m_exp = 0;
        m_trues = 0;
        m_falses = 0;
    }
    TreeGenResult(TypeBase* ty,StatementBase* statement,PatchList* ts,PatchList* fs){
        m_type = ty;
        m_kind = kTreeGenResult_Cx;
        m_statement = statement;
        m_exp = 0;
        m_trues = ts;
        m_falses = fs;
    }
    TypeBase* Type(){return m_type;}
    
    s32 Kind(){return m_kind;}
    
    ~TreeGenResult(){
        //
        delete m_trues;
        delete m_falses;
    }
    
    s32 m_kind;
    // with value
    ExpBase* m_exp;
    // without value
    StatementBase* m_statement;
    //condition statement
    PatchList* m_trues;
    PatchList* m_falses;
    TypeBase* m_type;
};    
// tree code generation
class TempMapList;
class TreeGenerator{
public:
    TreeGenerator();
    TreeGenResult* TreeGen(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label){
        ProceeExternalFunctions(venv,tenv);
        return TreeGenExp(venv, tenv, level, exp, done_label);
    }
    Level* OuterMostLevel();
    FragList* GetFragList(){return m_frag_list;}
    TempMapList* TempMap(){return m_temp_map_list;}
    StatementBase* ProcessEntryExit(SymTab* venv,SymTab* tenv, Level* level,StatementBase* s);
    ~TreeGenerator();
private:
    TreeGenResult* TreeGenExp(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpCall(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpOp(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpRecord(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpArray(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpSeq(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpAssign(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpIf(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpWhile(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpFor(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenExpLet(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label);
    TreeGenResult* TreeGenDec(SymTab* venv,SymTab*tenv,Level* level,Dec* dec,Label* done_label);
    void           TreeGenTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec);
    void           TreeGenFunctionDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label);
    TypeBase*      TreeGenTy(SymTab* tenv,Level* level,Ty* ty);/* get TypeBase from absyn ty */
    TreeGenResult* TreeGenVar(SymTab* venv,SymTab*tenv,Level* level,Var* var,Label* done_label);
    
    FrameBase*     MakeNewFrame(FunDec* fundec);
    TypeFieldList* MakeFormalsList(SymTab* venv,SymTab* tenv,Level* level,FieldList* params);
    
    // get offset of member
    s32 GetMemberOffset(TypeBase* ty,Symbol* member);
    
    // for to let exp
    LetExp* For2Let(ForExp* exp);
    
    ExpBase*       UnEx(TreeGenResult* tr);
    StatementBase* UnNx(TreeGenResult* tr);
    TreeGenResult* UnCx(TreeGenResult* tr);
    
    
    
    void ProceeExternalFunctions(SymTab* venv,SymTab* tenv);
    
    
    LevelManager* m_level_manager; //level management
    
    Temp* FP(){return m_fp;}
    Temp* SP(){return m_sp;}
    Temp* RV(){return m_rv;}
    Temp* m_fp;
    Temp* m_sp;
    Temp* m_rv;
    
    
    TempMapList* m_temp_map_list;
    
    Level* m_outer_most_level;
    
    FragList* m_frag_list;// manage all function frags
    
    // literal string
    LitStringList* m_lit_string_list;
    // debug 
    LoggerStdio m_logger;
};

}//namespace tiger

#endif
