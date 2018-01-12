#include <iostream>
#include "semant.h"
#include "tiger_assert.h"
namespace tiger{
    
Translator::Translator(){
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("semant");
}
Translator::~Translator(){

}

ExpBaseTy*  Translator::TransVar(SymTab* venv,SymTab* tenv,Var* var){
    m_logger.D("TransVar with kind %d",var->Kind());
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryVar* t;
            m_logger.D("aaa");
            m_logger.D("111 %s",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            if(dynamic_cast<SimpleVar*>(var)->GetSymbol()==0){
                m_logger.D("~~~");
            }
            m_logger.D("%s,%d",__FILE__,__LINE__);
            t = dynamic_cast<EnvEntryVar*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol())));
            m_logger.D("%s,%d",__FILE__,__LINE__);
            TIGER_ASSERT(t!=0,"var %s not found",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            return new ExpBaseTy(t->Type(),0);
        }
        case Var::kVar_Field:
        {
            ExpBaseTy* p;
            TypeFieldNode* head;
            p = TransVar(venv,tenv,dynamic_cast<FieldVar*>(var)->GetVar());
            if(p->Type()->Kind()!=TypeBase::kType_Name){
                m_logger.W("name type needed");
            }
            if(dynamic_cast<TypeName*>(p->Type())->Type()->Kind()!=TypeBase::kType_Record)
            {
                m_logger.W("record type needed");
            }
            head = dynamic_cast<TypeRecord*>(dynamic_cast<TypeName*>(p->Type())->Type())->GetList()->GetHead();
            while(head){
                if(head->m_field->Name()==tenv->MakeSymbol(dynamic_cast<FieldVar*>(var)->GetSym())){
                    /* ok */
                    delete p;
                    return new ExpBaseTy(head->m_field->Type(),0);
                }
                head = head->next;
            }
            TIGER_ASSERT(0,"%s not found in record type",dynamic_cast<FieldVar*>(var)->GetSym()->Name());
            break;

        }
        case Var::kVar_Subscript:
        {
            ExpBaseTy* p;
            ExpBaseTy* t;
            ExpBaseTy* ret;
            p = TransVar(venv,tenv,dynamic_cast<SubscriptVar*>(var)->GetVar());
            if(p->Type()->Kind()!=TypeBase::kType_Name){
                m_logger.W("name type needed");
            }
            TIGER_ASSERT(p!=0,"name type needed");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type()
            
            t = TransExp(venv,tenv,dynamic_cast<SubscriptVar*>(var)->GetExp());
            if(t->Type()->Kind()!=TypeBase::kType_Int){
                m_logger.W("array index should be int");
            }
            TIGER_ASSERT(p!=0,"array index should be int");
            
            ret = new ExpBaseTy(dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),0); 
            
            delete p;
            delete t;
            return ret;
            
        }
        default:
            break;
    }
    m_logger.W("shoud not reach here %s,%d",__FILE__,__LINE__);
    return 0;
}

ExpBaseTy*  Translator::TransExp(SymTab* venv,SymTab* tenv,Exp* exp){
    switch(exp->Kind())
    {
        case Exp::kExp_Var:
        {
            return TransVar(venv,tenv,dynamic_cast<VarExp*>(exp)->GetVar());
            break;
        }
        case Exp::kExp_Nil:
        {
            m_logger.D("type check with kExp_Nil");
            Symbol t("nil");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
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
        case Exp::kExp_Call:
        {
            m_logger.D("type check with kExp_Call");
            ExpNode* head;
            TypeFieldNode* p;
            ExpBaseTy* t;
            m_logger.D("%s,%d",__FILE__,__LINE__);
            EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<CallExp*>(exp)->Name())));
            m_logger.D("%s,%d",__FILE__,__LINE__);
            TIGER_ASSERT(f!=0,"function name not found",dynamic_cast<CallExp*>(exp)->Name());
            /* function foo() */
            if(f->GetList()->GetHead()==0){
                TIGER_ASSERT(dynamic_cast<CallExp*>(exp)->GetList()->GetHead()==0,"function actuals should be empty");
                return new ExpBaseTy(f->Type(),0);
            }
            p = f->GetList()->GetHead();
            TIGER_ASSERT(p!=0,"%s formals is null",dynamic_cast<CallExp*>(exp)->Name());
            head = dynamic_cast<CallExp*>(exp)->GetList()->GetHead();
            TIGER_ASSERT(head!=0,"actuals is null");
            while(head){
                t = TransExp(venv,tenv,head->m_exp);
                if(p->m_field->Type()!=t->Type()){
                    TIGER_ASSERT(0,"type mismatch");
                }
                head = head->next;
                p = p->next;
            }
            return new ExpBaseTy(f->Type(),0);
            break;
        }
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
                delete left;
                delete right;
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            }
            delete left;
            delete right;
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            break;
        }
        case Exp::kExp_Record:
        {
            m_logger.D("type check with kExp_Record");
            EnvEntryVar* p;
            EFieldNode* head;
            TypeFieldNode* n;
            m_logger.D("%s,%d",__FILE__,__LINE__);
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<RecordExp*>(exp)->Name())));
            m_logger.D("%s,%d",__FILE__,__LINE__);
            TIGER_ASSERT(p!=0,"type %s not found",dynamic_cast<RecordExp*>(exp)->Name()->Name());
            TIGER_ASSERT(p->Type()->Kind()==TypeBase::kType_Name,"type name is needed",dynamic_cast<RecordExp*>(exp)->Name()->Name());
            
            TIGER_ASSERT(dynamic_cast<TypeName*>(p->Type())->Type()->Kind()==TypeBase::kType_Record,"record type needed");
            
            /* id{} */
            head = dynamic_cast<RecordExp*>(exp)->GetList()->GetHead();
            if(head==0){
                m_logger.D("record exp is {}");
                return new ExpBaseTy(p->Type(),0);
            }
            
            n = dynamic_cast<TypeRecord*>(dynamic_cast<TypeName*>(p->Type())->Type())->GetList()->GetHead();
            
            while(head){
                ExpBaseTy* a;
                TIGER_ASSERT(n->m_field->Name()==tenv->MakeSymbol(head->m_efield->Name()),"member mismatch");
                
                a = TransExp(venv,tenv,head->m_efield->GetExp());
                
                if(a->Type()->Kind()!=TypeBase::kType_Nil){
                    m_logger.D("expected type:%s",n->m_field->Type()->TypeString());
                    m_logger.D("provided type %s",a->Type()->TypeString());
                    TIGER_ASSERT(n->m_field->Type()==a->Type(),"type mismatch");
                }
                
                delete a;
                
                head = head->next;
                n = n->next;
            }
            return new ExpBaseTy(p->Type(),0);
            break;
        }
        case Exp::kExp_Seq:
        {
            m_logger.D("type check with kExp_Seq");
            ExpBaseTy* tmp=0;
            ExpNode* p = dynamic_cast<SeqExp*>(exp)->GetList()->GetHead();
            if(p==0){
                m_logger.D("empty seq exp");
            }
            // return value ignore for now
            while(p){
                tmp = TransExp(venv,tenv,p->m_exp);
                delete tmp;
                p = p->next;
            }
            
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            break;
        }
        case Exp::kExp_Assign:
        {
            ExpBaseTy* a;
            ExpBaseTy* b;
            
            m_logger.D("type check with kExp_Assign");
            
            a = TransVar(venv,tenv,dynamic_cast<AssignExp*>(exp)->GetVar());
            b = TransExp(venv,tenv,dynamic_cast<AssignExp*>(exp)->GetExp());
            
            TIGER_ASSERT(a!=0,"var type is null");
            TIGER_ASSERT(b!=0,"exp type is null");
            
            m_logger.D("var type:%s",a->Type()->TypeString());
            m_logger.D("var type:%s",b->Type()->TypeString());
            
            TIGER_ASSERT(a->Type()==b->Type(),"type mismatch");
            
            
            delete a;

            return b;
            break;
        }
        case Exp::kExp_If:
        {
            m_logger.D("type check with kExp_If");
            Exp* if_exp;
            Exp* then_exp;
            Exp* else_exp;
            
            ExpBaseTy* a;
            ExpBaseTy* b;
            ExpBaseTy* c;
            
            if_exp = dynamic_cast<IfExp*>(exp)->GetTest();
            then_exp = dynamic_cast<IfExp*>(exp)->GetThen();
            else_exp = dynamic_cast<IfExp*>(exp)->GetElsee();
            
            TIGER_ASSERT(if_exp!=0,"if exp is null");
            TIGER_ASSERT(then_exp!=0,"then exp is null");
            
            a = TransExp(venv,tenv,if_exp);
            b = TransExp(venv,tenv,then_exp);
            if(else_exp)
                c = TransExp(venv,tenv,else_exp);
            
            TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"if exp should be int");
            
            delete a;
            
            if(else_exp){
                delete b;
                return c;
            }else{
                return b;
            }
            /* default type */

            break;
        }
        case Exp::kExp_While:
        {
            m_logger.D("type check with kExp_While");
            Exp* test_exp;
            Exp* body_exp;
            
            ExpBaseTy* a;
            ExpBaseTy* b;
            
            test_exp = dynamic_cast<WhileExp*>(exp)->GetTest();
            body_exp = dynamic_cast<WhileExp*>(exp)->GetTest();
            
            TIGER_ASSERT(test_exp!=0,"while exp is null");
            TIGER_ASSERT(body_exp!=0,"while body is null");
            
            a = TransExp(venv,tenv,test_exp);
            b = TransExp(venv,tenv,body_exp);
            
            TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"while exp should be int");
            
            delete a;
            delete b;
            
            /* default type */
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            break;
        }
        case Exp::kExp_Break:
        {
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            break;
        }
        case Exp::kExp_For:
        {
            m_logger.D("type check with kExp_For");
            Symbol* var;
            Exp* lo_exp;
            Exp* hi_exp;
            Exp* body_exp;
            
            //ExpBaseTy* a;
            ExpBaseTy* b;
            ExpBaseTy* c;
            ExpBaseTy* d;
            
            var = dynamic_cast<ForExp*>(exp)->GetVar();
            lo_exp = dynamic_cast<ForExp*>(exp)->GetLo();
            hi_exp = dynamic_cast<ForExp*>(exp)->GetHi();
            body_exp = dynamic_cast<ForExp*>(exp)->GetExp();
            
            TIGER_ASSERT(var!=0,"for var is null");
            TIGER_ASSERT(lo_exp!=0,"for lo is null");
            TIGER_ASSERT(hi_exp!=0,"for hi is null");
            
            //a = TransVar(venv,tenv,var);
            b = TransExp(venv,tenv,lo_exp);
            c = TransExp(venv,tenv,hi_exp);
            
            
            
            //TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"for var should be int");
            TIGER_ASSERT(b->Type()->Kind()==TypeBase::kType_Int,"for lo should be int");
            TIGER_ASSERT(c->Type()->Kind()==TypeBase::kType_Int,"for hi should be int");
            
            venv->BeginScope();
            tenv->BeginScope();
            
            venv->Enter(venv->MakeSymbol(var),new EnvEntryVar(b->Type(),EnvEntryVar::kEnvEntryVar_For_Value));
            
            d = TransExp(venv,tenv,body_exp);
            
            tenv->EndScope();
            venv->EndScope();
            
            //delete a;
            delete b;
            delete c;
            delete d;
            
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            break;
        }
        case Exp::kExp_Let:
        {
            m_logger.D("type check with kExp_Let");
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
            
            if(body)
                delete ret;
            
            tenv->EndScope();
            venv->EndScope();
            
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            break;
        }
        case Exp::kExp_Array:
        {
            m_logger.D("type check with kExp_Array");
            //dynamic_cast<ArrayExp*>(exp)->Name();//
            Exp* size_exp;
            Exp* init_exp;
            
            ExpBaseTy* size_ty;
            ExpBaseTy* init_ty;
            EnvEntryVar* p;
            m_logger.D("%s,%d",__FILE__,__LINE__);
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayExp*>(exp)->Name())));
            m_logger.D("%s,%d",__FILE__,__LINE__);
            TIGER_ASSERT(p->Type()->Kind()==TypeBase::kType_Name,"type %s not found",dynamic_cast<ArrayExp*>(exp)->Name()->Name());
            size_ty = TransExp(venv,tenv,dynamic_cast<ArrayExp*>(exp)->GetSize());
            init_ty = TransExp(venv,tenv,dynamic_cast<ArrayExp*>(exp)->GetInit());
            
            TIGER_ASSERT(size_ty!=0,"array size type is null");
            TIGER_ASSERT(init_ty!=0,"array init type is null");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())
            TIGER_ASSERT(size_ty->Type()->Kind()==TypeBase::kType_Int,"array size type error");
            TIGER_ASSERT(init_ty->Type()==dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),"array init type mismatch");
            
            return new ExpBaseTy(p->Type(),0);
            break;
        }
        // to be continue
        default:
        {
            m_logger.W("should not reach here %s,%d",__FILE__,__LINE__);
            break;
        }
    }
    std::cout<<"should not reach here "<<exp->Kind()<<std::endl;
    return 0;
}
TypeFieldList* Translator::MakeFormalsList(SymTab* venv,SymTab* tenv,FieldList* params)
{
    FieldNode* head;
    
    TypeFieldNode* tyhead=0;
    TypeFieldNode* tynext=0;
    TypeFieldNode* tynew=0;
    
    /* function foo() */
    if(params==0){
        m_logger.D("function formals is empty");
        return new TypeFieldList(0);
    }
    
    head = params->GetHead();
    
    while(head)
    {
        tynew = new TypeFieldNode;
        m_logger.D("%s,%d",__FILE__,__LINE__);
        tynew->m_field = (new TypeField(venv->MakeSymbol(head->m_field->Name()),dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type()));
        m_logger.D("%s,%d",__FILE__,__LINE__);
        if(tyhead==0)
            tyhead = tynew;
        if(tynext==0)
            tynext = tynew;
        else{
            tynext->next = tynew;
            tynew->prev = tynext;
            tynext = tynew;

        }
        head = head->next;
    }
    return new TypeFieldList(tyhead);
    
}
void Translator::TransFunctionDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    FunDecNode* fundec_head;
    FieldNode* head;
    
    TypeFieldNode* tyhead=0;
    TypeFieldNode* tynext=0;
    TypeFieldNode* tynew=0;
    
    ExpBaseTy *a;
    ExpBaseTy *b;
    
    m_logger.D("type check with kDec_Function");
    
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();
    
    /* process all function header decs */
    while(fundec_head){
    
        //fundec_head->m_fundec->Name() function name
        //fundec_head->m_fundec->Type() return type name
        //fundec_head->m_fundec->GetList()->GetHead() formals list
        //fundec_head->m_fundec->GetExp() function body
        // TypeFieldList* MakeFormalsList(FieldList *params);
        // new EnvEntryFun(list,type)

        if(fundec_head->m_fundec->Type()==0){
            m_logger.D("empty function return type ");
            venv->Enter(venv->MakeSymbol(fundec_head->m_fundec->Name()),new EnvEntryFun( MakeFormalsList(venv,tenv,fundec_head->m_fundec->GetList()), 0 ));

        }else{
            m_logger.D("%s,%d",__FILE__,__LINE__);
            venv->Enter(venv->MakeSymbol(fundec_head->m_fundec->Name()),new EnvEntryFun( MakeFormalsList(venv,tenv,fundec_head->m_fundec->GetList()), dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(fundec_head->m_fundec->Type())))->Type() ));
            m_logger.D("%s,%d",__FILE__,__LINE__);
        }
        fundec_head = fundec_head->next;
    }
    
    /* process all function body decs */
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();
    while(fundec_head){
        
        /* function foo() */
        if(fundec_head->m_fundec->GetList()!=0)
            head = fundec_head->m_fundec->GetList()->GetHead();
        
        venv->BeginScope();
        if(fundec_head->m_fundec->GetList()!=0){
            while(head){
                m_logger.D("%s,%d",__FILE__,__LINE__);
                venv->Enter(venv->MakeSymbol(head->m_field->Name()),new EnvEntryVar( dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type(), EnvEntryVar::kEnvEntryVar_For_Value) );
                m_logger.D("%s,%d",__FILE__,__LINE__);
                head = head->next;
            }
        }
        a = TransExp(venv,tenv,fundec_head->m_fundec->GetExp());
        m_logger.D("%s,%d",__FILE__,__LINE__);
        TIGER_ASSERT(a->Type() == dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(fundec_head->m_fundec->Type())))->Type(), "return type mismatch");
        m_logger.D("%s,%d",__FILE__,__LINE__);
        delete a;
        
        venv->EndScope();
        
        fundec_head = fundec_head->next;
    }
    
    
}
void Translator::TransTypeDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    NameTyPairNode* head;
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    /* process headers of decs */
    while(head){
        /*
         *
         * */
        m_logger.D("%s,%d",__FILE__,__LINE__);
        EnvEntryVar* p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())));
        m_logger.D("%s,%d",__FILE__,__LINE__);
        if(p){
            m_logger.W("Type %s redefined",head->m_nametypair->Name()->Name());
            //TIGER_ASSERT(0,"Type %s redefined",head->m_nametypair->Name()->Name());
        }
        //m_logger.D("New type with %s",head->m_nametypair->Name()->Name());
        tenv->Enter(tenv->MakeSymbol(head->m_nametypair->Name()),new EnvEntryVar(new TypeName(tenv->MakeSymbol(head->m_nametypair->Name()),0),EnvEntryVar::kEnvEntryVar_For_Type));
        head = head->next;
    }
    /* process bodys of decs*/
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    while(head){
        TypeBase* t = TransTy(tenv,head->m_nametypair->Type());
        if(t->Kind()!=TypeBase::kType_Name){
            m_logger.D("%s,%d",__FILE__,__LINE__);
            dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())))->Update(t);
            m_logger.D("%s,%d",__FILE__,__LINE__);
        }
        else{
            m_logger.D("TypeName update");
            EnvEntryVar* p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())));
            p->Update(dynamic_cast<TypeName*>(t));
            if(dynamic_cast<TypeName*>(t)->Type()==dynamic_cast<TypeName*>(p->Type())){
                TIGER_ASSERT(0,"cycle dependency occur");                        
            }

        }
        head = head->next;
    }
}
void Translator::TransDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            ExpBaseTy* t=TransExp(venv,tenv,dynamic_cast<VarDec*>(dec)->GetExp());
            if(dynamic_cast<VarDec*>(dec)->GetType()){
                EnvEntryBase* p;
                m_logger.D("%s,%d",__FILE__,__LINE__);
                p = tenv->Lookup(tenv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetType()));
                m_logger.D("%s,%d",__FILE__,__LINE__);
                /* t->Type() & p check */
                if(!t->Type()->Equal(dynamic_cast<EnvEntryVar*>(p)->Type()))
                {
                    std::cout<<"type not match"<<std::endl;
                }else{
                    venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type(),EnvEntryVar::kEnvEntryVar_For_Value));
                }
            }else{
                venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type(),EnvEntryVar::kEnvEntryVar_For_Value));
            }
            delete t;
            break;
        }
        case Dec::kDec_Function:
        {
            TransFunctionDec(venv,tenv,dec);
            break;
        }
        case Dec::kDec_Type:{
            m_logger.D("type check with kDec_Type");
            TransTypeDec(venv,tenv,dec);

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
            TIGER_ASSERT(t!=0,"type %s not found",dynamic_cast<NameTy*>(ty)->Name()->Name());
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
                m_logger.D("%s,%d",__FILE__,__LINE__);
                p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())));
                m_logger.D("%s,%d",__FILE__,__LINE__);
                TIGER_ASSERT(p!=0,"type %s not found",head->m_field->Type()->Name());

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
            m_logger.D("%s,%d",__FILE__,__LINE__);
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayTy*>(ty)->Name())));
            m_logger.D("%s,%d",__FILE__,__LINE__);
            return new TypeArray(p->Type());
            break;
        }
        default:
            break;
    }
    return 0;
}
    
    
}//namespace tiger
