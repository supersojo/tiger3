#ifndef SYMTAB_H
#define SYMTAB_H
#include <string.h>
#include <stdlib.h>
#include "temp.h"
#include "types.h"
#include "tiger_log.h"

namespace tiger
{

/*
All occur of a string in source make only the same symbol.
All the strings in source stores in a table.
Type env has such a table, Value env also has such a table.
For example, the string "a" may be a variable,so a symbol represent "a" in venv table,
if "a" may also be a type, so a symbol represnt "a" in tenv table.
*/
struct SymNameHashTableNode{
    SymNameHashTableNode(){
        m_name = 0;
        m_symbol = 0;
        prev = 0;
        next = 0;
    }
    ~SymNameHashTableNode(){
        if(m_name)
            free(m_name);
        if(m_symbol)
            delete m_symbol;
    }
    char* m_name;
    Symbol* m_symbol;
    SymNameHashTableNode* prev;
    SymNameHashTableNode* next;
};
class SymNameHashTable{
public:
    enum{
        kSymNameHashTable_Size=32,
        kSymNameHashTable_Invalid
    };
    SymNameHashTable();
    Symbol* MakeSymbol(Symbol*);
    ~SymNameHashTable();
private:
    s32 hash(char* s);
    void Clean();
    SymNameHashTableNode** m_tab;
};

/* Env
The base env entry of all env entry types.
*/
class EnvEntryBase{
public:
    enum{
      kEnvEntry_Var,/* type environment always uses this kind */
      kEnvEntry_Fun,
      kEnvEntry_Escape,/* for escape calculating */
      kEnvEntry_Invalid
    };
    EnvEntryBase(){m_kind=kEnvEntry_Invalid;}
    EnvEntryBase(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
    virtual ~EnvEntryBase(){}
private:
    s32 m_kind;
};
/*
Type binding maybe used in type env or value env.
We use intent to distinguish them.
For value binding,when delete such entry we should not delete the type of value'bingding, because many value maybe refer to the same type.
*/
class EnvEntryVar:public EnvEntryBase{
public:
    enum{
        kEnvEntryVar_For_Type,/* used in type environment*/
        kEnvEntryVar_For_Value,/* value environment*/
        kEnvEntryVar_Invalid/* invalid */
    };
    EnvEntryVar():EnvEntryBase(kEnvEntry_Var){m_type=0;m_intent = kEnvEntryVar_Invalid;}
    EnvEntryVar(TypeBase* ty,s32 intent,VarAccess* access):EnvEntryBase(kEnvEntry_Var){m_type=ty;m_intent = intent;m_access=access;}
    s32 Intent(){return m_intent;}
    VarAccess* Access(){return m_access;}
    TypeBase* Type(){return m_type;}
    void      Update(TypeBase* n){
        TypeName*p = dynamic_cast<TypeName*>(m_type);
        p->Update(n);

    }
    ~EnvEntryVar(){
        if(m_intent==kEnvEntryVar_For_Type){
            delete m_type;
        }
        if(m_intent==kEnvEntryVar_For_Value){
            delete m_access;
        }
    }
private:
    TypeBase* m_type;
    s32 m_intent;
    /* access info */
    VarAccess* m_access;
};

class EnvEntryFun:public EnvEntryBase{
public:
    enum{
        kFunction_Internal,
        kFunction_External,
        kFunction_Invalid
    };
    EnvEntryFun():EnvEntryBase(kEnvEntry_Fun){m_formals=0;m_result=0;m_level=0;m_label=0;m_fun_kind = kFunction_Internal;}
    EnvEntryFun(TypeFieldList *formals,TypeBase* result,Level* level,Label* label,s32 kind):EnvEntryBase(kEnvEntry_Fun){
        m_formals=formals;
        m_result=result;
        m_level=level;
        m_label=label;
        m_fun_kind = kind;
        //m_level->SetEnvEntry(dynamic_cast<EnvEntryBase*>(this));//associate
    }
    TypeBase* Type(){return m_result;}
    void SetType(TypeBase* ty){m_result = ty;}
    TypeFieldList* GetList(){return m_formals;}
    Level* GetLevel(){return m_level;}
    void   SetLevel(Level* lev){
        m_level = lev;
    }
    s32 FunKind(){return m_fun_kind;}
    Label* GetLabel(){return m_label;}
    ~EnvEntryFun(){
        if(m_formals)
            delete m_formals;
        //delete m_result;
    }
private:
    TypeFieldList* m_formals;
    TypeBase* m_result;/* memory managed by tenv table */
    
    Level* m_level;// managed by level manager
    Label*     m_label;/*managed by label pool*/
    s32 m_fun_kind;
    
};

class EnvEntryEscape:public EnvEntryBase{
public:
    EnvEntryEscape():EnvEntryBase(kEnvEntry_Escape){m_depth=0;m_escape_refer=0;}
    EnvEntryEscape(s32 depth,s32* escape_refer):EnvEntryBase(kEnvEntry_Escape){m_depth=depth;m_escape_refer=escape_refer;}
    s32 GetEscape(){return *m_escape_refer;}
    s32 Depth(){return m_depth;}
    void SetEscape(s32 escape){*m_escape_refer = escape;}
    ~EnvEntryEscape(){

    }
private:
    s32  m_depth;
    s32* m_escape_refer;/* refer to symbol's escape in absyn.h*/
    
};

class SymTabEntry{
public:    
    SymTabEntry(){
        m_name = 0;
        m_binding = 0;
    }
    SymTabEntry(Symbol* name,EnvEntryBase* binding){
        m_name = name;
        m_binding = binding;
    }
    Symbol* GetSymbol(){return m_name;}
    EnvEntryBase* GetEnvEntryBase(){return m_binding;}
    ~SymTabEntry(){
        //delete m_name;
        delete m_binding;
    }
private:
    Symbol* m_name;/* var or fun name or type name. memory managed by string hash table */
    EnvEntryBase* m_binding;
};

struct SymTabEntryNode{

    SymTabEntryNode(){
        m_entry = 0;
        prev = next = 0;
    }
    ~SymTabEntryNode(){
        delete m_entry;
    }
    /* members */
    SymTabEntry* m_entry;
    SymTabEntryNode* prev;
    SymTabEntryNode* next;
};
struct SimpleStackNode{
    SimpleStackNode(){
        m_name = 0;
        next = 0;
    }
    ~SimpleStackNode(){
    }
    Symbol* m_name;
    SimpleStackNode* next;
};
class SimpleStack{
public:
    SimpleStack(){m_top = 0;}
    void Push(Symbol* name){
        SimpleStackNode* n;
        n = new SimpleStackNode;
        n->m_name = name;
        n->next = m_top;
        m_top = n;
    }
    Symbol* Pop(){
        Symbol* ret;
        SimpleStackNode* p;
        p = m_top;
        if(m_top){
            m_top = m_top->next;
        }
        if(p){
            ret = p->m_name;
            delete p;
            return ret;
        }else
            return 0;
    }
    ~SimpleStack(){
        SimpleStackNode*p;
        p = m_top;
        while(p){
            m_top = m_top->next;
            delete p;
            p = m_top;
        }
    }
private:
    SimpleStackNode* m_top;
};
class ScopeMaker{
friend class SymTab;
public:
    enum{
        kScope_Let,
        kScope_Fun,
        kScope_For,
        kScope_While,
        kScope_Invalid
    };
    ScopeMaker(){m_kind = kScope_Invalid; m_next = 0;}
    ScopeMaker(s32 kind){m_kind = kind; m_next = 0;}
    s32 Kind(){return m_kind;}
private:
    s32 m_kind;
    ScopeMaker* m_next;
};
class SymTab{
public:
    enum{
        kSymTab_Size=32,
        kSymTab_Invalid
    };
    SymTab();
    void Erase(Symbol* name);
    void BeginScope(s32 scope_kind);
    void EndScope();
    s32 Scope(){
        if(m_scope_list) 
            return m_scope_list->Kind();
        return ScopeMaker::kScope_Invalid;
    }
    void Enter(Symbol* key,EnvEntryBase* value);
    EnvEntryBase* Lookup(Symbol* key);
    void Update(Symbol*s,TypeBase* t);
    Symbol* MakeSymbol(Symbol* s);
    Symbol* MakeSymbolFromString(char* s);
    /* helper for types */
    TypeBase*  Type(Symbol* s);
    ~SymTab();
private:
    SymTabEntryNode** m_tab;
    SimpleStack* m_stack;
    s32 hash(Symbol* key);
    void Clean();
    Symbol* m_marker;
    SymNameHashTable* m_sym_name_mapping;
    LoggerStdio m_logger;
    ScopeMaker* m_scope_list;
};

}

#endif
