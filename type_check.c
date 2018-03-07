#include "type_check.h"

namespace tiger{

TypeCheckResult* TypeChecker::TypeCheckExp(SymTab* venv,SymTab* tenv,Exp* exp){
    switch(exp->Kind())
    {
        case Exp::kExp_Var:
        {
            return TypeCheckVar(venv,tenv,dynamic_cast<VarExp*>(exp)->GetVar());
        }
        case Exp::kExp_Nil:
        {
            m_logger.D("type check with kExp_Nil");
            Symbol t("nil");
            return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
        }
        case Exp::kExp_Int:
        {
            TypeCheckResult* result=0;
            m_logger.D("type check with kExp_Int");
            Symbol t("int");
            result = new TypeCheckResult( tenv->Type( tenv->MakeSymbol(&t) ) );
            return result;
        }
        case Exp::kExp_String:
        {
            Symbol t("string");
            return new TypeCheckResult( tenv->Type( tenv->MakeSymbol(&t) ) );
        }
        case Exp::kExp_Call:
        {
            m_logger.D("type check with kExp_Call");
            m_logger.D("function call with %s",dynamic_cast<CallExp*>(exp)->Name()->Name());
            ExpNode* head;
            TypeFieldNode* p;
            TypeCheckResult* t;

        
            
            EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<CallExp*>(exp)->Name())));
            TIGER_ASSERT(f!=0,"function name not found",dynamic_cast<CallExp*>(exp)->Name());
            
            /* function foo() */
            if(f->GetList()->GetHead()==0){
                TIGER_ASSERT(dynamic_cast<CallExp*>(exp)->GetList()->GetHead()==0,"function actuals should be empty");
                return new TypeCheckResult( f->Type() );
            }
            p = f->GetList()->GetHead();
            TIGER_ASSERT(p!=0,"%s formals is null",dynamic_cast<CallExp*>(exp)->Name());
            head = dynamic_cast<CallExp*>(exp)->GetList()->GetHead();
            TIGER_ASSERT(head!=0,"actuals is null");

            while(head){
                t = TypeCheckExp(venv,tenv,head->m_exp);
                if(p->m_field->Type()!=t->Type()){
                    TIGER_ASSERT(0,"type mismatch %s",p->m_field->Name()->Name());
                }

                delete t;
                
                head = head->next;
                p = p->next;
            }

            return new TypeCheckResult( f->Type() );
        }
        case Exp::kExp_Op:
        {
            m_logger.D("type check with kExp_Op");
            Oper* op = dynamic_cast<OpExp*>(exp)->GetOper();
            TypeCheckResult* left,*right;
            left = TypeCheckExp(venv,tenv,dynamic_cast<OpExp*>(exp)->GetLeft());
            right = TypeCheckExp(venv,tenv,dynamic_cast<OpExp*>(exp)->GetRight());

            /* compare */
            if(op->Kind()==Oper::kOper_Lt||
               op->Kind()==Oper::kOper_Le||
               op->Kind()==Oper::kOper_Gt||
               op->Kind()==Oper::kOper_Ge||
               op->Kind()==Oper::kOper_Eq||
               op->Kind()==Oper::kOper_Neq){
                
                delete left;
                delete right;
                
                Symbol t("int");
                return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
            }
            if(op->Kind()==Oper::kOper_Add||
               op->Kind()==Oper::kOper_Sub||
               op->Kind()==Oper::kOper_Mul||
               op->Kind()==Oper::kOper_Div){
                /* type match? */
                if(left->Type()->Kind()!=TypeBase::kType_Int)
                    m_logger.W("type error");
                if(right->Type()->Kind()!=TypeBase::kType_Int)
                    m_logger.W("type error");

                delete left;
                delete right;
                
                Symbol t("int");
                return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
            }
   
            delete left;
            delete right;
            
            Symbol t("int");
            return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
        }
        case Exp::kExp_Record:
        {
            m_logger.D("type check with kExp_Record");
            EnvEntryVar* p;
            EFieldNode* head;
            TypeFieldNode* n;
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<RecordExp*>(exp)->Name())));
            TIGER_ASSERT(p!=0,"type %s not found",dynamic_cast<RecordExp*>(exp)->Name()->Name());
            TIGER_ASSERT(p->Type()->Kind()==TypeBase::kType_Name,"type name is needed",dynamic_cast<RecordExp*>(exp)->Name()->Name());
            
            TIGER_ASSERT(dynamic_cast<TypeName*>(p->Type())->Type()->Kind()==TypeBase::kType_Record,"record type needed");
            
            /* id{} */
            head = dynamic_cast<RecordExp*>(exp)->GetList()->GetHead();
            if(head==0){
                m_logger.D("record exp is {}");
                return new TypeCheckResult( p->Type() );
            }
            
            n = dynamic_cast<TypeRecord*>(dynamic_cast<TypeName*>(p->Type())->Type())->GetList()->GetHead();
            
            while(head){
                TypeCheckResult* a;
                TIGER_ASSERT(n->m_field->Name()==tenv->MakeSymbol(head->m_efield->Name()),"member mismatch");
                
                a = TypeCheckExp(venv,tenv,head->m_efield->GetExp());
                
                if(a->Type()->Kind()!=TypeBase::kType_Nil){
                    m_logger.D("expected type:%s",n->m_field->Type()->TypeString());
                    m_logger.D("provided type %s",a->Type()->TypeString());
                    TIGER_ASSERT(n->m_field->Type()==a->Type(),"type mismatch");
                }
                
                delete a;
                
                head = head->next;
                n = n->next;
            }
            return new TypeCheckResult( p->Type() );
        }
        case Exp::kExp_Seq:
        {
            m_logger.D("type check with kExp_Seq");
            TypeCheckResult* tmp=0;
            TypeBase* ty=0;
            ExpNode* p = dynamic_cast<SeqExp*>(exp)->GetList()->GetHead();
            if(p==0){
                m_logger.D("empty seq exp");
                Symbol t("int");
                return new TypeCheckResult(tenv->Type(tenv->MakeSymbol(&t)));
            }
            // return value ignore for now
            while(p){
                tmp = TypeCheckExp(venv,tenv,p->m_exp);
                p = p->next;
                
                ty = tmp->m_type;
                
                delete tmp;
            }
            return new TypeCheckResult(ty);
        }
        case Exp::kExp_Assign:
        {
            TypeCheckResult* a;
            TypeCheckResult* b;
            TypeCheckResult* result;
            
            m_logger.D("type check with kExp_Assign");
            
            a = TypeCheckVar(venv,tenv,dynamic_cast<AssignExp*>(exp)->GetVar());
            b = TypeCheckExp(venv,tenv,dynamic_cast<AssignExp*>(exp)->GetExp());
            
            TIGER_ASSERT(a!=0,"var type is null");
            TIGER_ASSERT(b!=0,"exp type is null");
            
            TIGER_ASSERT(a->Type()==b->Type(),"type mismatch");

            result = new TypeCheckResult( b->Type() );
            
            delete a;
            delete b;
            
            return result;
        }
        case Exp::kExp_If:
        {
            m_logger.D("type check with kExp_If");
            Exp* if_exp;
            Exp* then_exp;
            Exp* else_exp;
            
            TypeCheckResult* a;
            TypeCheckResult* b;
            TypeCheckResult* c;
            
            if_exp = dynamic_cast<IfExp*>(exp)->GetTest();
            then_exp = dynamic_cast<IfExp*>(exp)->GetThen();
            else_exp = dynamic_cast<IfExp*>(exp)->GetElsee();
            
            TIGER_ASSERT(if_exp!=0,"if exp is null");
            TIGER_ASSERT(then_exp!=0,"then exp is null");
            
            StatementBase* statement;
        

            a = TypeCheckExp(venv,tenv,if_exp);

                
            b = TypeCheckExp(venv,tenv,then_exp);

            if(else_exp)
                c = TypeCheckExp(venv,tenv,else_exp);
            
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
            
            TypeCheckResult* a;
            TypeCheckResult* b;

            
            test_exp = dynamic_cast<WhileExp*>(exp)->GetTest();
            body_exp = dynamic_cast<WhileExp*>(exp)->GetExp();
            
        
            TIGER_ASSERT(test_exp!=0,"while exp is null");
            TIGER_ASSERT(body_exp!=0,"while body is null");
            
            a = TypeCheckExp(venv,tenv,test_exp);
            

                
            venv->BeginScope(ScopeMaker::kScope_While);
            b = TypeCheckExp(venv,tenv,body_exp);
            venv->EndScope();
            
            // TBD: any else type?
            TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"while exp should be int");
            
            delete a;
            delete b;
            
            /* default type */
            Symbol t("int");
            return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
        }
        case Exp::kExp_Break:
        {
            if((venv->Scope()!=ScopeMaker::kScope_For) &&
               (venv->Scope()!=ScopeMaker::kScope_While)){
                m_logger.D("break should be in for/while scope.");
            }
            TIGER_ASSERT( ((venv->Scope()==ScopeMaker::kScope_For)||(venv->Scope()==ScopeMaker::kScope_While)),"expected in for or while scope");
            Symbol t("int");
            
            return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
        }
        case Exp::kExp_For:
        {
            m_logger.D("type check with kExp_For");
        
        
            Symbol* var;
            Exp* lo_exp;
            Exp* hi_exp;
            Exp* body_exp;
            
            TypeCheckResult* b;
            TypeCheckResult* c;
            TypeCheckResult* d;

            var = dynamic_cast<ForExp*>(exp)->GetVar();
            lo_exp = dynamic_cast<ForExp*>(exp)->GetLo();
            hi_exp = dynamic_cast<ForExp*>(exp)->GetHi();
            body_exp = dynamic_cast<ForExp*>(exp)->GetExp();
            
            TIGER_ASSERT(var!=0,"for var is null");
            TIGER_ASSERT(lo_exp!=0,"for lo is null");
            TIGER_ASSERT(hi_exp!=0,"for hi is null");
            
            //a = TransVar(venv,tenv,var);
            b = TypeCheckExp(venv,tenv,lo_exp);
            c = TypeCheckExp(venv,tenv,hi_exp);
            
            
            
            TIGER_ASSERT(b->Type()->Kind()==TypeBase::kType_Int,"for lo should be int");
            TIGER_ASSERT(c->Type()->Kind()==TypeBase::kType_Int,"for hi should be int");
            
            venv->BeginScope(ScopeMaker::kScope_For);
            //tenv->BeginScope();
            m_logger.D("in for scope begin");
            venv->Enter(venv->MakeSymbol(var),new EnvEntryVar(b->Type(), EnvEntryVar::kEnvEntryVar_For_Value, 0));
            
            d = TypeCheckExp(venv,tenv,body_exp);

            venv->EndScope();
            
            m_logger.D("in for scope end");
    
            delete b;
            delete c;
            delete d;

            Symbol t("int");
            
            return new TypeCheckResult( tenv->Type(tenv->MakeSymbol(&t)) );
        }
        case Exp::kExp_Let:
        {
            m_logger.D("type check with kExp_Let");
            TypeCheckResult* ret=0;
            TypeCheckResult* result=0;
            
            
            DecList* declist;
            Exp* body;
            declist = dynamic_cast<LetExp*>(exp)->GetDecList();
            body = dynamic_cast<LetExp*>(exp)->GetBody();
            
            venv->BeginScope(ScopeMaker::kScope_Let);
            tenv->BeginScope(ScopeMaker::kScope_Invalid);// type should not use scope
            

            // dec list
            DecNode* p;
            if(declist){
                p = declist->GetHead();
                while(p){
                    m_logger.D("TransDec var");
                    TypeCheckDec(venv,tenv,p->m_dec);
                    p = p->next;
                }
            }
            
            if(body)
                ret = TypeCheckExp(venv,tenv,body);

            tenv->EndScope();
            venv->EndScope();
            if(body){
                result = ret;
            }else{
                Symbol t("int");
                result = new TypeCheckResult( tenv->Type( tenv->MakeSymbol(&t) ) );
            }
            return result; 
        }
        case Exp::kExp_Array:
        {
            m_logger.D("type check with kExp_Array");
            //dynamic_cast<ArrayExp*>(exp)->Name();//
            Exp* size_exp;
            Exp* init_exp;
            
            TypeCheckResult* size_ty;
            TypeCheckResult* init_ty;
            EnvEntryVar* p;
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayExp*>(exp)->Name())));
            TIGER_ASSERT(p->Type()->Kind()==TypeBase::kType_Name,"type %s not found",dynamic_cast<ArrayExp*>(exp)->Name()->Name());
            size_ty = TypeCheckExp(venv,tenv,dynamic_cast<ArrayExp*>(exp)->GetSize());
            init_ty = TypeCheckExp(venv,tenv,dynamic_cast<ArrayExp*>(exp)->GetInit());
            
            TIGER_ASSERT(size_ty!=0,"array size type is null");
            TIGER_ASSERT(init_ty!=0,"array init type is null");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())
            TIGER_ASSERT(size_ty->Type()->Kind()==TypeBase::kType_Int,"array size type error");
            TIGER_ASSERT(init_ty->Type()==dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),"array init type mismatch");
            
            delete size_ty;
            delete init_ty;
            
            return new TypeCheckResult( p->Type() );
        }
        default:
        {
            m_logger.W("should not reach here %s,%d",__FILE__,__LINE__);
            break;
        }
    }
    m_logger.W("should not reach here (%s,%d)",__FILE__,__LINE__);
    return 0;
}
TypeCheckResult* TypeChecker::TypeCheckVar(SymTab* venv,SymTab* tenv,Var* var)
{
    m_logger.D("TransVar with kind %d",var->Kind());
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol())));
            TIGER_ASSERT(t!=0,"var %s not found",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            //m_logger.D("name=%s",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());

            return new TypeCheckResult( t->Type() );
        }
        case Var::kVar_Field:
        {
            TypeCheckResult* p;
            TypeFieldNode* head;
            p = TypeCheckVar(venv,tenv,dynamic_cast<FieldVar*>(var)->GetVar());
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
                    return new TypeCheckResult( head->m_field->Type() );
                }
                head = head->next;
            }
            TIGER_ASSERT(0,"%s not found in record type",dynamic_cast<FieldVar*>(var)->GetSym()->Name());
            
            break;

        }
        case Var::kVar_Subscript:
        {
            TypeCheckResult* p;
            TypeCheckResult* t;
            TypeCheckResult* ret;
            p = TypeCheckVar(venv,tenv,dynamic_cast<SubscriptVar*>(var)->GetVar());
            if(p->Type()->Kind()!=TypeBase::kType_Name){
                m_logger.W("name type needed");
            }
            TIGER_ASSERT(p!=0,"name type needed");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type()
            
            t = TypeCheckExp(venv,tenv,dynamic_cast<SubscriptVar*>(var)->GetExp());
            if(t->Type()->Kind()!=TypeBase::kType_Int){
                m_logger.W("array index should be int");
            }
            TIGER_ASSERT(p!=0,"array index should be int");
            
            ret = new TypeCheckResult(dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type()); 
            
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
void  TypeChecker::TypeCheckDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            TypeCheckResult* t;

            t = TypeCheckExp(venv,tenv,dynamic_cast<VarDec*>(dec)->GetExp());

            
            if(dynamic_cast<VarDec*>(dec)->GetType()){
                EnvEntryBase* p;
                p = tenv->Lookup(tenv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetType()));
                /* t->Type() & p check */
                if(!t->Type()->Equal(dynamic_cast<EnvEntryVar*>(p)->Type()))
                {
                    std::cout<<"type not match"<<std::endl;
                }else{
                    venv->Enter( venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()), new EnvEntryVar(t->Type(), EnvEntryVar::kEnvEntryVar_For_Value, 0) );
                }
            }else{
                venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type(), EnvEntryVar::kEnvEntryVar_For_Value, 0));
            }

            delete t;
            break;
        }
        case Dec::kDec_Function:
        {
            m_logger.D("type check with kDec_Function");
            TypeCheckFunctionDec(venv,tenv,dec);
            break;
        }
        case Dec::kDec_Type:{
            m_logger.D("type check with kDec_Type");
            TypeCheckTypeDec(venv,tenv,dec);
            break;
        }
        default:
            break;
    }
}
TypeFieldList* TypeChecker::MakeFormalsList(SymTab* venv,SymTab* tenv,FieldList* params)
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
        tynew->m_field = (new TypeField(venv->MakeSymbol(head->m_field->Name()),dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type()));
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
void TypeChecker::TypeCheckFunctionDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    FunDecNode* fundec_head;
    FieldNode* head;

    
    TypeCheckResult *a;
    TypeCheckResult *b;

    
    m_logger.D("type check with kDec_Function");
    
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();
    
    /* process all function header decs */
    while(fundec_head){
    

        if(fundec_head->m_fundec->Type()==0){
            m_logger.D("empty function return type ");
            venv->Enter( venv->MakeSymbol(fundec_head->m_fundec->Name()),
                         new EnvEntryFun( 
                                          MakeFormalsList(venv,tenv,fundec_head->m_fundec->GetList()),
                                          0, 
                                          0, 
                                          TempLabel::NewNamedLabel(fundec_head->m_fundec->Name()->Name()),
                                          0
                                        )
                       );

        }else{
            venv->Enter( venv->MakeSymbol(fundec_head->m_fundec->Name()),
                         new EnvEntryFun( 
                                          MakeFormalsList(venv,tenv,fundec_head->m_fundec->GetList()), 
                                          dynamic_cast<EnvEntryVar*>( 
                                                                      tenv->Lookup( tenv->MakeSymbol(fundec_head->m_fundec->Type()) )
                                                                    )->Type(), 
                                          0, 
                                          TempLabel::NewNamedLabel(fundec_head->m_fundec->Name()->Name()),
                                          0
                                        )
                       );
        }
        fundec_head = fundec_head->next;
    }
    
    /* process all function body decs */
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();
    while(fundec_head){
        
        
        /* function foo() */
        if(fundec_head->m_fundec->GetList()!=0)
            head = fundec_head->m_fundec->GetList()->GetHead();
        
        venv->BeginScope(ScopeMaker::kScope_Fun);
        if(fundec_head->m_fundec->GetList()!=0){
            while(head){
                venv->Enter( venv->MakeSymbol(head->m_field->Name()),
                             new EnvEntryVar( 
                                              dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type(), 
                                              EnvEntryVar::kEnvEntryVar_For_Value, 0 
                                            ) 
                           );
                head = head->next;
            }
        }
        // process function body
        a = TypeCheckExp(venv,tenv,fundec_head->m_fundec->GetExp());
        if(fundec_head->m_fundec->Type()==0){
            m_logger.D("function return type is null");
        }else
        {
            m_logger.D("type kind %d",a->Type()->Kind());
            if(a->Type()->Kind()!=TypeBase::kType_Nil)
            {
                TIGER_ASSERT(a->Type() == dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(fundec_head->m_fundec->Type())))->Type(), "return type mismatch");
            }
        }

        delete a;
        
        venv->EndScope();
        
        fundec_head = fundec_head->next;
    }
}
void TypeChecker::TypeCheckTypeDec(SymTab* venv,SymTab* tenv,Dec* dec)
{
    NameTyPairNode* head;
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    /* process headers of decs */
    while(head){
        /*
         *
         * */
        m_logger.D("type dec %s,%d",__FILE__,__LINE__);
        EnvEntryVar* p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())));
        if(p){
            m_logger.W("Type %s redefined",head->m_nametypair->Name()->Name());
            //TIGER_ASSERT(0,"Type %s redefined",head->m_nametypair->Name()->Name());
        }
        //m_logger.D("New type with %s",head->m_nametypair->Name()->Name());
        tenv->Enter( tenv->MakeSymbol(head->m_nametypair->Name()),
                     new EnvEntryVar( 
                                      new TypeName(tenv->MakeSymbol(head->m_nametypair->Name()),0),
                                      EnvEntryVar::kEnvEntryVar_For_Type, 0
                                    ) 
                   );
        head = head->next;
    }
    /* process bodys of decs */
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    while(head){
        TypeBase* t = TypeCheckTy(tenv,head->m_nametypair->Type());
        if(t->Kind()!=TypeBase::kType_Name){
            dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())))->Update(t);
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
TypeBase* TypeChecker::TypeCheckTy(SymTab* tenv,Ty* ty)
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
        }
        case Ty::kTy_Array:
        {
            m_logger.D("type check with kTy_Array");
            EnvEntryVar* p;
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayTy*>(ty)->Name())));
            return new TypeArray(p->Type());
        }
        default:
            break;
    }
    return 0;
}


}//namespace tiger