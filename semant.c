#include <iostream>
#include "semant.h"

namespace tiger{
    
Translator::Translator(){
}
Translator::~Translator(){
}

ExpBaseTy*  Translator::TransExp(SymTab* venv,SymTab* tenv,Exp* exp){
    switch(exp->Kind())
    {
        case Exp::kExp_Op:
        {
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
            ExpBaseTy* t=TransExp(venv,tenv,dynamic_cast<VarDec*>(dec)->GetExp());
            if(dynamic_cast<VarDec*>(dec)->GetType()){
                EnvEntryBase* p;
                p = tenv->Lookup(tenv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetType()));
                /* t->Type() & p check */
            }else{
                venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type()));
            }
            delete t;
            break;
        }
        case Dec::kDec_Function:
            break;
        case Dec::kDec_Type:
            break;
        default:
            break;
    }
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
