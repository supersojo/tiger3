#include <iostream>
#include "semant.h"

namespace tiger{
    
Translator::Translator(){
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("semant");
}
Translator::~Translator(){
}

ExpBaseTy*  Translator::TransExp(SymTab* venv,SymTab* tenv,Exp* exp){
    switch(exp->Kind())
    {
        case Exp::kExp_Op:
        {
            m_logger.D("type check with kExp_Op");
            Oper* op = dynamic_cast<OpExp*>(exp)->GetOper();
            ExpBaseTy* left,*right;
            left = TransExp(venv,tenv,dynamic_cast<OpExp*>(exp)->GetLeft());
            right = TransExp(venv,tenv,dynamic_cast<OpExp*>(exp)->GetRight());
            if(op->Kind()==Oper::kOper_Add){
                if(left->Type()->Kind()!=TypeBase::kType_Int)
                    std::cout<<"type error"<<std::endl;
                if(right->Type()->Kind()!=TypeBase::kType_Int)
                    std::cout<<"type error"<<std::endl;
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            }
            delete left;
            delete right;
            break;
        }
        case Exp::kExp_Int:
        {
            m_logger.D("type check with kExp_Int");
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
        }
        case Exp::kExp_String:
        {
            Symbol t("string");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
        }
        case Exp::kExp_Let:
        {
            ExpBaseTy* ret=0;
            DecList* declist;
            Exp* body;
            declist = dynamic_cast<LetExp*>(exp)->GetDecList();
            body = dynamic_cast<LetExp*>(exp)->GetBody();
            
            venv->BeginScope();
            tenv->BeginScope();
            // dec list
            DecNode* p;
            if(declist){
                p = declist->GetHead();
                while(p){
                    TransDec(venv,tenv,p->m_dec);
                    p = p->next;
                }
            }
            if(body)
                ret = TransExp(venv,tenv,body);
            
            tenv->EndScope();
            venv->EndScope();
            
            return ret;
        }
        // to be continue
        default:
            break;
    }
    std::cout<<"should not reach here"<<std::endl;
    return 0;
}
void        Translator::TransDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            ExpBaseTy* t=TransExp(venv,tenv,dynamic_cast<VarDec*>(dec)->GetExp());
            if(dynamic_cast<VarDec*>(dec)->GetType()){
                EnvEntryBase* p;
                p = tenv->Lookup(tenv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetType()));
                /* t->Type() & p check */
                if(!t->Type()->Equal(dynamic_cast<EnvEntryVar*>(p)->Type()))
                {
                    std::cout<<"type not match"<<std::endl;
                }else{
                    venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type()));
                }
            }else{
                venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type()));
            }
            delete t;
            break;
        }
        case Dec::kDec_Function:
            break;
        case Dec::kDec_Type:{
            m_logger.D("type check with kDec_Type");
            TypeDec* p;
            NameTyPairNode* head;
            head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
            while(head){
                TypeBase* t = TransTy(tenv,head->m_nametypair->Type());
                m_logger.D("New type with %s",head->m_nametypair->Name()->Name());
                tenv->Enter(tenv->MakeSymbol(head->m_nametypair->Name()),new EnvEntryVar(t));
                head = head->next;
            }
            break;
        }
        default:
            break;
    }
}
TypeBase* Translator::TransTy(SymTab* tenv,Ty* ty)
{
    switch(ty->Kind())
    {
        case Ty::kTy_Name:
        {
            m_logger.D("type check with kTy_Name");
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<NameTy*>(ty)->Name())));
            return t->Type();
            break;
        }
        case Ty::kTy_Record:
        {
            m_logger.D("type check with kTy_Record");
            FieldNode* head;
            TypeFieldNode* n=0,*ret=0,*cur=0;
            EnvEntryVar* p;
            head = dynamic_cast<RecordTy*>(ty)->GetList()->GetHead();
            while(head){
                //head->m_field->Name()
                //head->m_field->Type()
                n = new TypeFieldNode;

                p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())));

                n->m_field = new TypeField(tenv->MakeSymbol(head->m_field->Name()),p->Type());
                if(ret==0)
                    ret = n;
                if(cur==0)
                { 
                    cur = n;
                }
                else
                {
                    cur->next = n;
                    n->prev = cur;
                    cur = n;
                }
                head = head->next;
            }
            return new TypeRecord(new TypeFieldList(ret));
            break;
        }
        case Ty::kTy_Array:
        {
            m_logger.D("type check with kTy_Array");
            EnvEntryVar* p;
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayTy*>(ty)->Name())));
            return new TypeArray(p->Type());
            break;
        }
        default:
            break;
    }
    return 0;
}
ExpBaseTy*  Translator::TransVar(SymTab* venv,SymTab* tenv,Var* var)
{
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryBase* binding;
            binding = venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol()));
            
            if(binding && binding->Kind()==EnvEntryBase::kEnvEntry_Var){
                return new ExpBaseTy(dynamic_cast<EnvEntryVar*>(binding)->Type(),0);
            }else{
            //undefined symbol
            }
            break;
        }
        case Var::kVar_Field:
        {
            break;
        }
        case Var::kVar_Subscript:
        {
            break;
        }
        default:
            break;
    }
    return 0;
}
    
    
}//namespace tiger
