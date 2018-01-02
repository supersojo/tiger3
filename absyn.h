/* Coding: ANSI */
#include "tiger_type.h"

namespace tiger{
    
class Pos{
public:
    Pos(){m_line_no = 0; m_off = 0;}
    s32 LineNo(){return m_line_no;}
    s32 Off(){return m_off;}
private:
    s32 m_line_no;
    s32 m_off;
};

class Symbol{
public:
    Symbol(){
        m_name=0;
    }
    Symbol(char* name){
        m_name = strdup(name);
    }
    char* Name(){return m_name;}
    ~Symbol(){
        free(m_name);
    }
private:
    char* m_name;
    /* additional stuff */
    //...
};
    
class Var{
public:
    enum{
        kVar_Simple,
        kVar_Field,
        kVar_Subscript,
        kVar_Invalid
    };
    Var(){m_kind = kVar_Invalid;}
    Var(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
class SimpleVar:public Var{
public:
    SimpleVar():Var(kVar_Simple){
        m_sym = 0;
    }
    SimpleVar(Symbol* sym):Var(kVar_Simple){
        m_sym = sym;
    }
    ~SimpleVar(){delete m_sym;}
public:
    Symbol* m_sym;
};
class FieldVar:public Var{
public:
    FieldVar():Var(kVar_Field){
        
    }
    FieldVar(Var* var,Symbol* sym):Var(kVar_Field){
        m_var = var;
        m_sym = sym;
    }
    Var* GetVar(){return m_var;}
    Symbol* GetSym(){return m_sym;}
    ~FieldVar(){
        delete m_var;
        delete m_sym;
    }
private:
    Var* m_var;
    Symbol* m_sym;
};

/* proto declaration */
class Exp;

class SubscriptVar:public Var{
public:
    SubscriptVar():Var(kVar_Subscript){
    }
    SubscriptVar(Var* var,Exp* exp):Var(kVar_Subscript){
        m_var = var;
        m_exp = exp;
    }
    Var* GetVar(){return m_var;}
    Exp* GetExp(){return m_exp;}
    ~SubscriptVar(){
        delete m_var;
        delete m_exp;
    }
private:
    Var* m_var;
    Exp* m_exp;
};
class Exp{
public:
    enum{
        kExp_Var,
        kExp_Nil,
        kExp_Int,
        kExp_String,
        kExp_Call,
        kExp_Op,
        kExp_Record,
        kExp_Seq,
        kExp_Assign,
        kExp_If,
        kExp_While,
        kExp_Break,
        kExp_For,
        kExp_Let,
        kExp_Array,
        kExp_Invalid
    };
    Exp(){m_kind = kExp_Invalid;}
    Exp(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
struct ExpNode{
    ExpNode(){
        exp = 0;
        prev = next = 0;
    }
    ~ExpNode(){
        delete exp;
    }
    
    /* members */
    Exp* exp;
    ExpNode* prev;
    ExpNode* next;
};
class ExpList{
public:
    ExpList(){
        m_head = 0;
    }
    ExpList(ExpNode* exp_node){
        m_head = exp_node;
    }
    ~ExpList(){
        ExpNode* p = m_head;
        while(p){
             m_head = m_head->next;
             delete p;
             p = m_head;
        }
    }
private:
    ExpNode* m_head;
};
class VarExp:public Exp{
public:
    VarExp():Exp(kExp_Var){
        m_var = 0;
    }
    VarExp(Var* var):Exp(kExp_Var){
        m_var = var;
    }
    Var* GetVar(){return m_var;}
    ~VarExp(){
        delete m_var;
    }
private:
    Var* m_var;
};
class NilExp:public Exp{
public:
    NilExp():Exp(kExp_Nil){
    }
};
class IntExp:public Exp{
public:
    IntExp():Exp(kExp_Int){
        m_ival = 0;
    }
    IntExp(s32 v):Exp(kExp_Int){
        m_ival = v;
    }
    s32 GetInt(){return m_ival;}
private:
    s32 m_ival;
};
class StringExp:public Exp{
public:
    StringExp():Exp(kExp_String){
        m_sval = 0;
    }
    StringExp(char* s):Exp(kExp_String){
        m_sval = strdup(s);
    }
    char* GetString(){return m_sval;}
    ~StringExp(){
        free(m_sval);
    }
private:
    char* m_sval;
};
class CallExp:public Exp{
public:
    CallExp():Exp(kExp_Call){
        m_sym = 0;
        m_explist = 0;
    }
    CallExp(Symbol* sym,ExpList* explist):Exp(kExp_Call){
        m_sym = sym;
        m_explist = explist;
    }
    ~CallExp(){
        delete m_sym;
        delete m_explist;
    }
private:
    Symbol* m_sym;/* func */
    ExpList* m_explist;
    
};
class Oper{
public:
    enum{
        kOper_Add,
        kOper_Sub,
        kOper_Mul,
        kOper_Div,
        kOper_Invalid
    };
    Oper(){
        m_kind = kOper_Invalid;
    }
    s32 Kind(){return m_kind;}
    Oper(s32 kind){m_kind = kind;}
private:
    s32 m_kind;
};
class OpExp:public Exp{
public:
    OpExp():Exp(kExp_Op){
        m_oper = 0;
        m_left = 0;
        m_right = 0;
    }
    OpExp(Oper* oper,Exp* left,Exp* right):Exp(kExp_Op){
        m_oper = oper;
        m_left = left;
        m_right = right;
    }
    ~OpExp(){
        delete m_oper;
        delete m_left;
        delete m_right;
    }
private:
    Oper* m_oper;
    Exp* m_left;
    Exp* m_right;
};

class EFieldList;
class RecordExp:public Exp{
public:
    RecordExp():Exp(kExp_Record){
        m_type = 0;
        m_fields = 0;
    }
    RecordExp(Symbol* type,EFieldList* fields):Exp(kExp_Record){
        m_type = type;
        m_fields = fields;
    }
    ~RecordExp(){
        delete m_type;
        delete m_fields;
    }
private:
    Symbol* m_type;
    EFieldList* m_fields;
};    
class SeqExp:public Exp{
public:
    SeqExp():Exp(kExp_Seq){
        m_list = 0;
    }
    SeqExp(ExpList* explist):Exp(kExp_Seq){
        m_list = explist;
    }
    ~SeqExp(){
        delete m_list;
    }
private:
    ExpList* m_list;
};
class AssignExp:public Exp{
public:
    AssignExp():Exp(kExp_Assign){
        m_var = 0;
        m_exp = 0;
    }
    AssignExp(Var* var,Exp* exp):Exp(kExp_Assign){
        m_var = var;
        m_exp = exp;
    }
    ~AssignExp(){
        delete m_var;
        delete m_exp;
    }
private:
    Var* m_var;
    Exp* m_exp;
};
class IfExp:public Exp{
public:
    IfExp():Exp(kExp_If){
        m_test = 0;
        m_then = 0;
        m_elsee = 0;
    }
    IfExp(Exp* test,Exp* then,Exp* elsee):Exp(kExp_If){
        m_test = test;
        m_then = then;
        m_elsee = elsee;
    }
    ~IfExp(){
        delete m_test;
        delete m_then;
        delete m_elsee;
    }
private:
    Exp* m_test;
    Exp* m_then;
    Exp* m_elsee;
};
class WhileExp:public Exp{
public:
    WhileExp():Exp(kExp_While){
        m_test = 0;
        m_body = 0;
    }
    WhileExp(Exp* test,Exp* body):Exp(kExp_While){
        m_test = test;
        m_body = body;
    }
    ~WhileExp(){
        delete m_test;
        delete m_body;
    }
private:
    Exp* m_test;
    Exp* m_body;
};
class BreakExp:public Exp{
public:
    BreakExp():Exp(kExp_Break){}
};
class ForExp:public Exp{
public:
    ForExp():Exp(kExp_For){
        m_var = 0;
        m_lo = 0;
        m_hi = 0;
        m_body = 0;
    }
    ForExp(Symbol* var,Exp* lo,Exp* hi,Exp* body):Exp(kExp_For){
        m_var = var;
        m_lo = lo;
        m_hi = hi;
        m_body = body;
    }
    ~ForExp(){
        delete m_var;
        delete m_lo;
        delete m_hi;
        delete m_body;
    }
private:
    Symbol* m_var;
    Exp* m_lo;
    Exp* m_hi;
    Exp* m_body;
};

class DecList;
class LetExp:public Exp{
public:
    LetExp():Exp(kExp_Let){
        m_decs = 0;
        m_body = 0;
    }
    LetExp(DecList* decs,Exp* body):Exp(kExp_Let){
        m_decs = decs;
        m_body = body;
    }
    ~LetExp(){
        delete m_decs;
        delete m_body;
    }
private:
    DecList* m_decs;
    Exp* m_body;
};
class ArrayExp:public Exp{
public:
    ArrayExp():Exp(kExp_Array){
        m_type = 0;
        m_size = 0;
        m_init = 0;
    }
    ArrayExp(Symbol* type,Exp* size,Exp* init):Exp(kExp_Array){
        m_type = type;
        m_size = size;
        m_init = init;
    }
    ~ArrayExp(){
        delete m_type;
        delete m_size;
        delete m_init;
    }
private:
    Symbol* m_type;
    Exp* m_size;
    Exp* m_init;
};

class Dec{
public:
    enum{
        kDec_Function,
        kDec_Var,
        kDec_Type,
        kDec_Invalid
    };
    Dec(){m_kind = kDec_Invalid;}
    Dec(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
    
private:
    s32 m_kind;
};
struct DecNode{
    DecNode(){
        m_dec = 0;
    }
    ~DecNode(){
        delete m_dec;
    }
    
    /*members*/
    Dec* m_dec;
    
    DecNode* prev;
    DecNode* next;
    
};
class DecList{
public:
    DecList(){m_head=0;}
    DecList(DecNode* head){m_head = head;}
    ~DecList(){
        DecNode* p;
        p = m_head;
        while(p)
        {
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    DecNode* m_head;
};

class FieldList;
class FunDec{
public:
    FunDec(){
        m_name = 0;
        m_params = 0;
        m_result = 0;
        m_body = 0;
    }
    FunDec(Symbol* name,FieldList* params,Symbol* result,Exp* body){
        m_name = name;
        m_params = params;
        m_result = result;
        m_body = body;
    }
    ~FunDec(){
        delete m_name;
        delete m_params;
        delete m_result;
        delete m_body;
    }
private:
    Symbol* m_name;
    FieldList* m_params;
    Symbol* m_result;
    Exp* m_body;
};
struct FunDecNode{
    FunDecNode(){
        m_fundec = 0;
    }
    ~FunDecNode(){
        delete m_fundec;
    }
    
    /*members*/
    FunDec* m_fundec;
    FunDecNode* prev;
    FunDecNode* next;
};
class FunDecList{
public:
    FunDecList(){m_head=0;}
    FunDecList(FunDecNode* head){m_head=head;}
    ~FunDecList(){
        FunDecNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    FunDecNode* m_head;
};
class FunctionDec:public Dec{
public:
    FunctionDec():Dec(kDec_Function){
        m_fundeclist = 0;
    }
    FunctionDec(FunDecList* funs):Dec(kDec_Function){
        m_fundeclist = funs;
    }
    ~FunctionDec(){
        delete m_fundeclist;
    }
private:
    FunDecList* m_fundeclist;
};
class VarDec:public Dec{
    VarDec():Dec(kDec_Var){
        m_var = 0;
        m_type = 0;
        m_init = 0;
    }
    VarDec(Symbol* var,Symbol* type,Exp* init){
        m_var = var;
        m_type = type;
        m_init = init;
    }
    ~VarDec(){
        delete m_var;
        delete m_type;
        delete m_init;
    }
private:
    Symbol* m_var;
    Symbol* m_type;
    Exp* m_init;
};

class NameTyPairList;
class TypeDec:public Dec{
public:
    TypeDec(){m_nametylist = 0;}
    TypeDec(NameTyPairList* nametylist){m_nametylist = nametylist;}
    ~TypeDec(){delete m_nametylist;}
private:
    NameTyPairList* m_nametylist;
};


class Ty{
public:
    enum{
        kTy_Name,
        kTy_Record,
        kTy_Array,
        kTy_Invalid
    };
    Ty(){m_kind = kTy_Invalid;}
    Ty(s32 kind){m_kind = kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
class NameTy:public Ty{
public:
    NameTy():Ty(kTy_Name){
        m_sym = 0;
    }
    NameTy(Symbol* sym):Ty(kTy_Name){
        m_sym = sym;
    }
    ~NameTy(){
        delete m_sym;
    }
private:
        Symbol* m_sym;
};

class FieldList;
class RecordTy:public Ty{
public:
    RecordTy():Ty(kTy_Record){
        m_list = 0;
    }
    RecordTy(FieldList* list):Ty(kTy_Record){
        m_list = list;
    }
    ~RecordTy(){
        delete m_list;
    }
private:
    FieldList* m_list;
};
class ArrayTy:public Ty{
public:
    ArrayTy():Ty(kTy_Array){
        m_name = 0;
    }
    ArrayTy(Symbol* name):Ty(kTy_Array){
        m_name = name;
    }
    ~ArrayTy(){
        delete m_name;
    }
private:
    Symbol* m_name;
};

class NameTyPair{
public:
    NameTyPair(){m_name=0;m_ty=0;}
    NameTyPair(Symbol* name,Ty* a_ty){m_name = name; m_ty = a_ty;}
    ~NameTyPair(){
        delete m_name;
        delete m_ty;
    }
private:
    Symbol* m_name;
    Ty* m_ty;
};
struct NameTyPairNode{
    NameTyPairNode(){
        m_nametypair = 0;
    }
    ~NameTyPairNode(){
        delete m_nametypair;
    }
    
    /*members*/
    NameTyPair* m_nametypair;
    NameTyPairNode* prev;
    NameTyPairNode* next;
};
class NameTyPairList{
public:
    NameTyPairList(){m_head=0;}
    NameTyPairList(NameTyPairNode* head){m_head=head;}
    ~NameTyPairList(){
        NameTyPairNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    NameTyPairNode* m_head;
};
class Field{
public:
    Field(){m_name = 0;m_type = 0;}
    Field(Symbol* name,Symbol* type){
        m_name = name;
        m_type = type;
    }
    ~Field(){delete m_name;delete m_type;}
private:
    Symbol* m_name;
    Symbol* m_type;
};
struct FieldNode{
    FieldNode(){
        m_field = 0;
        next = prev = 0;
    }
    ~FieldNode(){
        delete m_field;
    }
    
    /*members*/
    Field* m_field;
    
    FieldNode* prev;
    FieldNode* next;
};
class FieldList{
public:
    FieldList(){
        m_head = 0;
    }
    FieldList(FieldNode* head){
        m_head = head;
    }
    ~FieldList(){
        FieldNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    FieldNode* m_head;
};
class EField{
public:
    EField(){m_name=0;m_exp=0;}
    EField(Symbol* name,Exp* exp){
        m_name = name;
        m_exp = exp;
    }
    ~EField(){delete m_name;delete m_exp;}
private:
    Symbol* m_name;
    Exp* m_exp;
};
struct EFieldNode{
    EFieldNode(){
        m_efield = 0;
        next = prev = 0;
    }
    ~EFieldNode(){
        delete m_efield;
    }
    
    /* members*/
    EField* m_efield;
    
    EFieldNode* prev;
    EFieldNode* next;
    
};
class EFieldList{
public:
    EFieldList(){m_head=0;}
    EFieldList(EFieldNode* head){
        m_head = head;
    }
    ~EFieldList(){
        EFieldNode* p;
        p = m_head;
        while(p){
            m_head = m_head->next;
            delete p;
            p = m_head;
        }
    }
private:
    EFieldNode* m_head;
};


}// namespace tiger 