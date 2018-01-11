/* Coding: ANSI */
#ifndef TIGER_INTERNAL_TYPES_H
#define TIGER_INTERNAL_TYPES_H

#include "tiger_type.h"

#include "absyn.h" //Symbol

namespace tiger{

class TypeBase{
public:
    enum{
        kType_Int,
        kType_String,
        kType_Nil,
        kType_Void,
        kType_Array,
        kType_Record,
        kType_Name,
        kType_Invalid
    };
    TypeBase(){m_kind = kType_Invalid;}
    TypeBase(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};

class TypeInt:public TypeBase{
public:
    TypeInt():TypeBase(kType_Int){}
};
class TypeString:public TypeBase{
public:
    TypeString():TypeBase(kType_String){}
};

class TypeNil:public TypeBase{
public:
    TypeNil():TypeBase(kType_Nil){}
};

class TypeVoid:public TypeBase{
public:
    TypeVoid():TypeBase(kType_Void){}
};

class TypeArray:public TypeBase{
public:
    TypeArray():TypeBase(kType_Array){m_array=0;}
    TypeArray(TypeBase* array):TypeBase(kType_Array){m_array = array;}
    ~TypeArray(){delete m_array;}
private:
    TypeBase* m_array;
};

class TypeField{
public:
    TypeField(){m_name=0;m_type=0;}
    TypeField(Symbol* name,TypeBase* ty){
        m_name = name;
        m_type = ty;
    }
    ~TypeField(){
        delete m_name;
        delete m_type;
    }
private:
    Symbol* m_name;
    TypeBase* m_type;
};

struct TypeFieldNode{

    TypeFieldNode(){
        m_field = 0;
        prev = 0;
        next = 0;
    }
    ~TypeFieldNode(){
        delete m_field;
    }
    /* members */
    TypeField* m_field;
    TypeFieldNode* prev;
    TypeFieldNode* next;
};
class TypeFieldList{
public:
    TypeFieldList(){m_head = 0;}
    TypeFieldList(TypeFieldNode* head){m_head = head;}
    ~TypeFieldList(){
        TypeFieldNode*p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    TypeFieldNode* m_head;
};

class TypeRecord:public TypeBase{
public:
    TypeRecord():TypeBase(kType_Record){m_record=0;}
    TypeRecord(TypeFieldList* record):TypeBase(kType_Record){m_record = record;}
    ~TypeRecord(){delete m_record;}
private:
    TypeFieldList* m_record;
};

class TypeName:public TypeBase{
public:
    TypeName():TypeBase(kType_Name){m_name=0;m_type=0;}
    TypeName(Symbol* name,TypeBase* ty):TypeBase(kType_Name){
        m_name = name;
        m_type = ty;
    }
    ~TypeName(){
        delete m_name;
        delete m_type;
    }
private:
    Symbol* m_name;
    TypeBase* m_type;
};

/* Env*/
class EnvEntryBase{
public:
    enum{
      kEnvEntry_Var,/* type environment always uses this kind */
      kEnvEntry_Fun,
      kEnvEntry_Invalid
    };
    EnvEntryBase(){m_kind=kEnvEntry_Invalid;}
    EnvEntryBase(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};

class EnvEntryVar:public EnvEntryBase{
public:
    EnvEntryVar():EnvEntryBase(kEnvEntry_Var){m_type=0;}
    EnvEntryVar(TypeBase* ty):EnvEntryBase(kEnvEntry_Var){m_type=ty;}
    ~EnvEntryVar(){
        delete m_type;
    }
private:
    TypeBase* m_type;
};

class EnvEntryFun:public EnvEntryBase{
public:
    EnvEntryFun():EnvEntryBase(kEnvEntry_Fun){m_formals=0;m_result=0;}
    EnvEntryFun(TypeFieldList *formals,TypeBase* result):EnvEntryBase(kEnvEntry_Fun){m_formals=formals;m_result=result;}
    ~EnvEntryFun(){
        delete m_formals;
        delete m_result;
    }
private:
    TypeFieldList* m_formals;
    TypeBase* m_result;
    
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
        delete m_name;
        delete m_binding;
    }
private:
    Symbol* m_name;// var or fun name or type name
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
        SimpleStackNode* p;
        p = m_top;
        if(m_top){
            m_top = m_top->next;
        }
        if(p)
            return p->m_name;
        else
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
class SymTab{
public:
    enum{
        kSymTab_Size=32,
        kSymTab_Invalid
    };
    SymTab();
    void Erase(Symbol* name);
    void BeginScope();
    void EndScope();
    void Enter(Symbol* key,EnvEntryBase* value);
    EnvEntryBase* Lookup(Symbol* key);
    ~SymTab();
private:
    SymTabEntryNode** m_tab;
    SimpleStack* m_stack;
    s32 hash(Symbol* key);
    void Clean();
    Symbol* m_marker;
};

}//namespace tiger


#endif
