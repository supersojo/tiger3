#include <iostream>
#include "semant.h"
#include "tiger_assert.h"
#include "frame.h"

namespace tiger{
    
Translator::Translator(){
    m_level_manager = new LevelManager;
    
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("semant");
    
    m_outer_most_level = 0;
    
    m_lit_string_list = new LitStringList;
    
    //frame pointer
    m_fp = 0;
}
Translator::~Translator(){
    delete m_level_manager;
    
    delete m_lit_string_list;
    
    //free all temp label memory
    TempLabel::Exit();
}

Level*      Translator::OuterMostLevel()
{
    if(m_outer_most_level==0)
    {
        m_outer_most_level = new Level;
        m_level_manager->NewLevel(m_outer_most_level);
    }
    return m_outer_most_level;
}
ExpBaseTy*  Translator::TransVar(SymTab* venv,SymTab* tenv,Level* level,Var* var){
    m_logger.D("TransVar with kind %d",var->Kind());
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol())));
            TIGER_ASSERT(t!=0,"var %s not found",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            
            TreeBaseEx* ex;
            AccessFrame* af;
            AccessReg*   ar;
            if(t->Access()->GetAccess()->Kind()==AccessBase::kAccess_Frame){
                af = dynamic_cast<AccessFrame*>(t->Access()->GetAccess());
                ex = new TreeBaseEx( new ExpBaseMem( new ExpBaseBinop(BinaryOp::kBinaryOp_Add,new ExpBaseTemp( FP() ),new ExpBaseConst(af->Offset()))) );
            }
            if(t->Access()->GetAccess()->Kind()==AccessBase::kAccess_Reg){
                ar = dynamic_cast<AccessReg*>(t->Access()->GetAccess());
                ex = new TreeBaseEx( new ExpBaseTemp( ar->GetTemp() ) );
            }

            return new ExpBaseTy(t->Type(),ex);//
        }
        case Var::kVar_Field:
        {
            ExpBaseTy* p;
            TypeFieldNode* head;
            p = TransVar(venv,tenv,level,dynamic_cast<FieldVar*>(var)->GetVar());
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
            p = TransVar(venv,tenv,level,dynamic_cast<SubscriptVar*>(var)->GetVar());
            if(p->Type()->Kind()!=TypeBase::kType_Name){
                m_logger.W("name type needed");
            }
            TIGER_ASSERT(p!=0,"name type needed");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type()
            
            t = TransExp(venv,tenv,level,dynamic_cast<SubscriptVar*>(var)->GetExp());
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

ExpBaseTy*  Translator::TransExp(SymTab* venv,SymTab* tenv,Level* level,Exp* exp){
    switch(exp->Kind())
    {
        case Exp::kExp_Var:
        {
            return TransVar(venv,tenv,level,dynamic_cast<VarExp*>(exp)->GetVar());
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
            ExpBaseTy* result=0;
            m_logger.D("type check with kExp_Int");
            Symbol t("int");
            result = new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseEx( new ExpBaseConst(dynamic_cast<IntExp*>(exp)->GetInt()) ));
            return result;
        }
        case Exp::kExp_String:
        {
            Symbol t("string");
            Label* l;
            l = m_lit_string_list->FindByString(dynamic_cast<StringExp*>(exp)->GetString());
            if(l==0){
                l = TempLabel::NewLabel();
                m_lit_string_list->Insert(l,dynamic_cast<StringExp*>(exp)->GetString());
            }
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseEx( new ExpBaseName( l ) ));
        }
        case Exp::kExp_Call:
        {
            m_logger.D("type check with kExp_Call");
            m_logger.D("function call with %s",dynamic_cast<CallExp*>(exp)->Name()->Name());
            ExpNode* head;
            TypeFieldNode* p;
            ExpBaseTy* t;
            EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<CallExp*>(exp)->Name())));
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
                t = TransExp(venv,tenv,level,head->m_exp);
                if(p->m_field->Type()!=t->Type()){
                    TIGER_ASSERT(0,"type mismatch");
                }
                delete t;
                head = head->next;
                p = p->next;
            }
            return new ExpBaseTy(f->Type(),0);// new TreeBaseEx( new ExpBaseCall( new ExpBaseName(f->GetLabel()), new ExpBaseList()) )
            break;
        }
        case Exp::kExp_Op:
        {
            m_logger.D("type check with kExp_Op");
            Oper* op = dynamic_cast<OpExp*>(exp)->GetOper();
            ExpBaseTy* left,*right;
            TreeBase* l, *r;
            left = TransExp(venv,tenv,level,dynamic_cast<OpExp*>(exp)->GetLeft());
            right = TransExp(venv,tenv,level,dynamic_cast<OpExp*>(exp)->GetRight());
            l = left->Tree();
            r = right->Tree();
            if(op->Kind()==Oper::kOper_Add||
               op->Kind()==Oper::kOper_Sub||
               op->Kind()==Oper::kOper_Mul||
               op->Kind()==Oper::kOper_Div){
                if(left->Type()->Kind()!=TypeBase::kType_Int)
                    std::cout<<"type error"<<std::endl;
                if(right->Type()->Kind()!=TypeBase::kType_Int)
                    std::cout<<"type error"<<std::endl;
                
                delete left;
                delete right;
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseEx( new ExpBaseBinop(BinaryOp::ToBinaryOp(op->Kind()),TreeBase::UnEx(l),TreeBase::UnEx(r)) ));
            }
            delete left;
            delete right;
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseEx( new ExpBaseBinop(BinaryOp::ToBinaryOp(op->Kind()),TreeBase::UnEx(l),TreeBase::UnEx(r)) ));
            break;
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
                return new ExpBaseTy(p->Type(),0);
            }
            
            n = dynamic_cast<TypeRecord*>(dynamic_cast<TypeName*>(p->Type())->Type())->GetList()->GetHead();
            
            while(head){
                ExpBaseTy* a;
                TIGER_ASSERT(n->m_field->Name()==tenv->MakeSymbol(head->m_efield->Name()),"member mismatch");
                
                a = TransExp(venv,tenv,level,head->m_efield->GetExp());
                
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
            TypeBase* ty=0;
            TreeBase* result=0;
            StatementBase* statement=0;
            ExpNode* p = dynamic_cast<SeqExp*>(exp)->GetList()->GetHead();
            if(p==0){
                m_logger.D("empty seq exp");
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            }
            // return value ignore for now
            while(p){
                tmp = TransExp(venv,tenv,level,p->m_exp);
                p = p->next;
                if(p){
                    if(statement==0)
                        statement = TreeBase::UnNx( tmp->Tree() );
                    else
                        statement = new StatementSeq(statement, TreeBase::UnNx( tmp->Tree() ));
                    delete tmp;
                }
            }
            
            if( statement )// at least 2 exps 
            {
                if(statement->Kind()==StatementBase::kStatement_Seq){
                    result = new TreeBaseEx( new ExpBaseEseq( statement, TreeBase::UnEx( tmp->Tree() ) ) );
                }else{
                    result = new TreeBaseEx( TreeBase::UnEx( tmp->Tree() ) );
                }
            }
            else// 1 exp
            {
                result = new TreeBaseEx( TreeBase::UnEx( tmp->Tree() ) );
            }
            
            if(tmp){
                ty = tmp->Type();
                delete tmp;
            }
            //Symbol t("int");
            // need optimize for type 
            //return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),result);
            return new ExpBaseTy(ty,result);
        }
        case Exp::kExp_Assign:
        {
            ExpBaseTy* a;
            ExpBaseTy* b;
            ExpBaseTy* result;
            
            m_logger.D("type check with kExp_Assign");
            
            a = TransVar(venv,tenv,level,dynamic_cast<AssignExp*>(exp)->GetVar());
            b = TransExp(venv,tenv,level,dynamic_cast<AssignExp*>(exp)->GetExp());
            
            TIGER_ASSERT(a!=0,"var type is null");
            TIGER_ASSERT(b!=0,"exp type is null");
            
            //m_logger.D("var type:%s",a->Type()->TypeString());
            //m_logger.D("var type:%s",b->Type()->TypeString());
            
            TIGER_ASSERT(a->Type()==b->Type(),"type mismatch");
            
            //new TreeBaseNx( new StatementMove( TreeBase::UnEx( a->Tree() ), TreeBase::UnEx( b->Tree() ) ) );
            
            result = new ExpBaseTy( b->Type(), new TreeBaseNx( new StatementMove( TreeBase::UnEx( a->Tree() ), TreeBase::UnEx( b->Tree() ) ) ) );
            
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
            
            ExpBaseTy* a;
            ExpBaseTy* b;
            ExpBaseTy* c;
            
            if_exp = dynamic_cast<IfExp*>(exp)->GetTest();
            then_exp = dynamic_cast<IfExp*>(exp)->GetThen();
            else_exp = dynamic_cast<IfExp*>(exp)->GetElsee();
            
            TIGER_ASSERT(if_exp!=0,"if exp is null");
            TIGER_ASSERT(then_exp!=0,"then exp is null");
            
            a = TransExp(venv,tenv,level,if_exp);
            b = TransExp(venv,tenv,level,then_exp);
            if(else_exp)
                c = TransExp(venv,tenv,level,else_exp);
            
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
            
            a = TransExp(venv,tenv,level,test_exp);
            
            venv->BeginScope(ScopeMaker::kScope_While);
            b = TransExp(venv,tenv,level,body_exp);
            venv->EndScope();
            
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
            if((venv->Scope()!=ScopeMaker::kScope_For) &&
               (venv->Scope()!=ScopeMaker::kScope_While)){
                m_logger.D("break should be in for/while scope.");
            }
            TIGER_ASSERT( ((venv->Scope()==ScopeMaker::kScope_For)||(venv->Scope()==ScopeMaker::kScope_While)),"expected in for or while scope");
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
            b = TransExp(venv,tenv,level,lo_exp);
            c = TransExp(venv,tenv,level,hi_exp);
            
            
            
            //TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"for var should be int");
            TIGER_ASSERT(b->Type()->Kind()==TypeBase::kType_Int,"for lo should be int");
            TIGER_ASSERT(c->Type()->Kind()==TypeBase::kType_Int,"for hi should be int");
            
            venv->BeginScope(ScopeMaker::kScope_For);
            //tenv->BeginScope();
            
            venv->Enter(venv->MakeSymbol(var),new EnvEntryVar(b->Type(), EnvEntryVar::kEnvEntryVar_For_Value, 0));
            
            d = TransExp(venv,tenv,level,body_exp);
            
            //tenv->EndScope();
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
            ExpBaseTy* result=0;
            
            Level* alevel;
            
            TreeBase* tree;
            StatementBase* statement = 0;
            
            DecList* declist;
            Exp* body;
            declist = dynamic_cast<LetExp*>(exp)->GetDecList();
            body = dynamic_cast<LetExp*>(exp)->GetBody();
            
            venv->BeginScope(ScopeMaker::kScope_Let);
            tenv->BeginScope(ScopeMaker::kScope_Invalid);// type should not use scope
            
            alevel = new Level(level,MakeNewFrame( 0 ));
            m_level_manager->NewLevel(alevel);
        
            // dec list
            DecNode* p;
            if(declist){
                p = declist->GetHead();
                while(p){
                    m_logger.D("TransDec var a:=1");
                    tree = TransDec(venv,tenv,alevel,p->m_dec);
                    p = p->next;
                    /*
                    a=1
                    b=2
                    c=3
                           seq
                         /   \
                       seq  c=3
                     /   \
                    a=1  b=2
                    */
                    if(statement==0)
                        statement = TreeBase::UnNx(tree);
                    else
                        statement = new StatementSeq(statement, TreeBase::UnNx(tree));
                    
                    delete tree;// the tree is not used already
                }
            }
            
            if(body){
                ret = TransExp(venv,tenv,level,body);
                if(statement)
                    statement = new StatementSeq( statement, TreeBase::UnNx(ret->Tree()) );
                else
                    statement = TreeBase::UnNx( ret->Tree() );
            }
            
            
            tenv->EndScope();
            venv->EndScope();
            if(body){
                result = new ExpBaseTy( ret->Type(), new TreeBaseNx(statement) );
                delete ret;
            }else{
                Symbol t("int");
                result = new ExpBaseTy( tenv->Type( tenv->MakeSymbol(&t) ), new TreeBaseNx(statement) );
            }
            return result; 
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
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayExp*>(exp)->Name())));
            TIGER_ASSERT(p->Type()->Kind()==TypeBase::kType_Name,"type %s not found",dynamic_cast<ArrayExp*>(exp)->Name()->Name());
            size_ty = TransExp(venv,tenv,level,dynamic_cast<ArrayExp*>(exp)->GetSize());
            init_ty = TransExp(venv,tenv,level,dynamic_cast<ArrayExp*>(exp)->GetInit());
            
            TIGER_ASSERT(size_ty!=0,"array size type is null");
            TIGER_ASSERT(init_ty!=0,"array init type is null");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())
            TIGER_ASSERT(size_ty->Type()->Kind()==TypeBase::kType_Int,"array size type error");
            TIGER_ASSERT(init_ty->Type()==dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),"array init type mismatch");
            
            delete size_ty;
            delete init_ty;
            
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
TypeFieldList* Translator::MakeFormalsList(SymTab* venv,SymTab* tenv,Level* level,FieldList* params)
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

FrameBase* Translator::MakeNewFrame(FunDec* fundec)
{
    FieldNode* head = 0;
    FrameBase* f;
    
    AccessBase* access;
    AccessList* al;
    BoolList* bl;
    f = new FrameBase(FrameBase::kFrame_X86);
    al = f->GetFormals();
    bl = f->GetEscapes();
    
    m_logger.D("New Frame Begin~~~");
    /**for static linklist **/
    access = f->AllocLocal(1/*true*/);
    access->Retain();//inc refcnt
    al->Insert(access,AccessList::kAccessList_Rear);
    bl->Insert(BoolNode::kBool_True,BoolList::kBoolList_Rear);
    
    
    if(fundec==0)
        return f;
    
    if(fundec->GetList()==0){
        //empty formals
        head = 0;
    }
    else{
        head = fundec->GetList()->GetHead();
    }
    while(head){

        access = f->AllocLocal(0/*false*/);
        access->Retain();//inc refcnt
        al->Insert(access,AccessList::kAccessList_Rear);
        
        bl->Insert(BoolNode::kBool_False,BoolList::kBoolList_Rear);
        
        head = head->next;
    }
    m_logger.D("formals size:%d",al->Size());
    m_logger.D("escapes size:%d",bl->Size());
    m_logger.D("New Frame End~~~");
    
    return f;
}

void Translator::TransFunctionDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec)
{
    FunDecNode* fundec_head;
    FieldNode* head;
    
    TypeFieldNode* tyhead=0;
    TypeFieldNode* tynext=0;
    TypeFieldNode* tynew=0;
    
    ExpBaseTy *a;
    ExpBaseTy *b;

    Level* alevel;
    
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

        /* level and label */
        alevel = new Level(level,MakeNewFrame(fundec_head->m_fundec));
        m_level_manager->NewLevel(alevel);
        if(fundec_head->m_fundec->Type()==0){
            m_logger.D("empty function return type ");
            venv->Enter(venv->MakeSymbol(fundec_head->m_fundec->Name()),new EnvEntryFun( MakeFormalsList(venv,tenv,level,fundec_head->m_fundec->GetList()), 0, alevel, TempLabel::NewNamedLabel(fundec_head->m_fundec->Name()->Name()) ));

        }else{
            venv->Enter(venv->MakeSymbol(fundec_head->m_fundec->Name()),new EnvEntryFun( MakeFormalsList(venv,tenv,level,fundec_head->m_fundec->GetList()), dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(fundec_head->m_fundec->Type())))->Type(), alevel, TempLabel::NewNamedLabel(fundec_head->m_fundec->Name()->Name()) ));
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
                venv->Enter(venv->MakeSymbol(head->m_field->Name()),new EnvEntryVar( dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type(), EnvEntryVar::kEnvEntryVar_For_Value, 0) );
                head = head->next;
            }
        }
        /* each function needs its own level information */
        alevel = dynamic_cast<EnvEntryFun*>( venv->Lookup( venv->MakeSymbol(fundec_head->m_fundec->Name()) ))->GetLevel();
        TIGER_ASSERT(alevel!=0,"function level is empty!");
        a = TransExp(venv,tenv,alevel,fundec_head->m_fundec->GetExp());
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
void Translator::TransTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec)
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
        if(p){
            m_logger.W("Type %s redefined",head->m_nametypair->Name()->Name());
            //TIGER_ASSERT(0,"Type %s redefined",head->m_nametypair->Name()->Name());
        }
        //m_logger.D("New type with %s",head->m_nametypair->Name()->Name());
        tenv->Enter( tenv->MakeSymbol(head->m_nametypair->Name()),new EnvEntryVar(new TypeName(tenv->MakeSymbol(head->m_nametypair->Name()),0),EnvEntryVar::kEnvEntryVar_For_Type, 0) );
        head = head->next;
    }
    /* process bodys of decs*/
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    while(head){
        TypeBase* t = TransTy(tenv,level,head->m_nametypair->Type());
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
TreeBase* Translator::TransDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            ExpBaseTy* t=TransExp(venv,tenv,level,dynamic_cast<VarDec*>(dec)->GetExp());
            TreeBase* tree;
            AccessFrame* af;
            AccessReg*   ar;
            VarAccess* access;
            
            tree = t->Tree();
            
            if(dynamic_cast<VarDec*>(dec)->GetSymbol()->GetEscape()==1){
                af = dynamic_cast<AccessFrame*>( level->Frame()->AllocLocal(1) );
                access = new VarAccess(level,af);
            }else{
                ar = dynamic_cast<AccessReg*>( level->Frame()->AllocLocal(0) );
                access = new VarAccess(level,ar);
            }
            if(dynamic_cast<VarDec*>(dec)->GetType()){
                EnvEntryBase* p;
                p = tenv->Lookup(tenv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetType()));
                /* t->Type() & p check */
                if(!t->Type()->Equal(dynamic_cast<EnvEntryVar*>(p)->Type()))
                {
                    std::cout<<"type not match"<<std::endl;
                }else{
                    
                    venv->Enter( venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()), new EnvEntryVar(t->Type(), EnvEntryVar::kEnvEntryVar_For_Value, access) );
                }
            }else{
                venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->Type(), EnvEntryVar::kEnvEntryVar_For_Value, access));
            }
            
            
            
            /* tree */
            if(dynamic_cast<VarDec*>(dec)->GetSymbol()->GetEscape()==1){
                tree = new TreeBaseNx( new StatementMove( 
                                          new ExpBaseMem( 
                                             new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( FP() ), new ExpBaseConst( af->Offset() ) 
                                             ) 
                                          ),
                                          TreeBase::UnEx( tree )                                          
                                       )
                                     );
            }
            else
            {
                tree = new TreeBaseNx( new StatementMove ( 
                                          new ExpBaseTemp( ar->GetTemp() 
                                          ),
                                          TreeBase::UnEx( tree )                                          
                                       )
                                     );
            }
            delete t;
            return tree;
        }
        case Dec::kDec_Function:
        {
            TransFunctionDec(venv,tenv,level,dec);
            break;
        }
        case Dec::kDec_Type:{
            m_logger.D("type check with kDec_Type");
            TransTypeDec(venv,tenv,level,dec);

            break;
        }
        default:
            break;
    }
}
TypeBase* Translator::TransTy(SymTab* tenv,Level* level,Ty* ty)
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

ExpBase*       TreeBase::UnEx(TreeBase* tree){
    switch(tree->Kind()){
        case kTreeBase_Ex:
        {
            return dynamic_cast<TreeBaseEx*>(tree)->GetExp();
        }
        case kTreeBase_Nx:
        {
            return new ExpBaseEseq(dynamic_cast<TreeBaseNx*>(tree)->GetStatement(),new ExpBaseConst(0));
        }
        case kTreeBase_Cx:
        {
            Temp* r;
            Label* t,*f;
            
            r = TempLabel::NewTemp();
            t = TempLabel::NewLabel();
            f = TempLabel::NewLabel();
            
            dynamic_cast<TreeBaseCx*>(tree)->GetTrues()->DoPatch(t);
            dynamic_cast<TreeBaseCx*>(tree)->GetFalses()->DoPatch(f);
            
            return new ExpBaseEseq(new StatementMove(new ExpBaseTemp(r),new ExpBaseConst(1)),
                                      new ExpBaseEseq( dynamic_cast<TreeBaseCx*>(tree)->GetStatement(),
                                         new ExpBaseEseq( new StatementLabel(f),
                                            new ExpBaseEseq(new StatementMove(new ExpBaseTemp(r),new ExpBaseConst(0)),
                                               new ExpBaseEseq(new StatementLabel(t),new ExpBaseTemp(r)
                                               )
                                            )
                                         )
                                      )
                                  );
        }
        default:
            ;/* should not reach here */
    }
}
StatementBase* TreeBase::UnNx(TreeBase* tree){
    switch(tree->Kind()){
        case kTreeBase_Ex:
        {
            return new StatementExp(dynamic_cast<TreeBaseEx*>(tree)->GetExp());
        }
        case kTreeBase_Nx:
        {
            return dynamic_cast<TreeBaseNx*>(tree)->GetStatement();
        }
        case kTreeBase_Cx:
        {
            return dynamic_cast<TreeBaseCx*>(tree)->GetStatement();
        }
        default:
            ;/* should not reach here */
    }
}
TreeBaseCx* TreeBase::UnCx(TreeBase* tree){
    switch(tree->Kind()){
        case kTreeBase_Ex:
        {
            StatementBase* tmp;
            PatchList * trues = new PatchList;
            PatchList* falses = new PatchList;
            
            tmp = new StatementCjump(RelationOp::kRelationOp_Ne,dynamic_cast<TreeBaseEx*>(tree)->GetExp(),new ExpBaseConst(0),0/*true*/,0/*false*/);
            
            trues->Insert(dynamic_cast<StatementCjump*>(tmp)->GetATrueLabel(),PatchList::kPatchList_Rear);
            falses->Insert(dynamic_cast<StatementCjump*>(tmp)->GetAFalseLabel(),PatchList::kPatchList_Rear);
            
            return new TreeBaseCx(tmp,trues,falses);
        }
        case kTreeBase_Nx:
        {
            ;/*should not reach here */
        }
        case kTreeBase_Cx:
        {
            return dynamic_cast<TreeBaseCx*>(tree);
        }
        default:
            ;/* should not reach here */
    }
}
LitStringList::LitStringList(){
    m_tab = new LitStringNode*[kLitStringList_Size];
    for(s32 i=0;i<kLitStringList_Size;i++){
        m_tab[i] = 0;
    }
    m_size = 0;
    
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("LitStringList");
}
void LitStringList::Insert(Label* l,char* str){
    s32 index = hash(l);
    LitStringNode* p;
    p = m_tab[index];
    while(p){
        if(l==p->m_label){
            // exist already
            m_logger.D("%s already exist",l->Name());
            return;
        }
        p = p->next;
    }
    LitStringNode* n;
    n = new LitStringNode;
    n->m_label = l;
    n->m_string = strdup(str);//memory leak
    n->next = m_tab[index];
    if(m_tab[index])
        m_tab[index]->prev = n;
    m_tab[index] = n;
    m_size++;
    m_logger.D("%s with \"%s\" insert ok",l->Name(),str);
}
char* LitStringList::Find(Label* l){
    s32 index = hash(l);
    LitStringNode* p;
    p = m_tab[index];
    while(p){
        if(l==p->m_label){
            // exist already
            return p->m_string;
        }
        p = p->next;
    }
    return 0;//not found
}
char* LitStringList::FindByLabel(Label* l){
    return Find(l);
}
Label* LitStringList::FindByString(char* str){// low performance 
    LitStringNode* p;
    for(s32 i=0;i<kLitStringList_Size;i++){
        p = m_tab[i];
        while(p){
            if(strcmp(p->m_string,str)==0){
                m_logger.D("FindByString with \"%s\" ok",str);
                return p->m_label;
            }
            p = p->next;
        }
    }
    return 0;
}
LitStringList::~LitStringList(){
    for(s32 i=0;i<kLitStringList_Size;i++){
        Clean(m_tab[i]);
    }
    
    delete[] m_tab;
}

void Translator::Traverse(TreeBase* tree)
{
    if(tree->Kind()==TreeBase::kTreeBase_Ex)
        TraverseEx(dynamic_cast<TreeBaseEx*>(tree)->GetExp());
    if(tree->Kind()==TreeBase::kTreeBase_Nx)
        TraverseNx(dynamic_cast<TreeBaseNx*>(tree)->GetStatement());
    if(tree->Kind()==TreeBase::kTreeBase_Cx)
        TraverseCx(dynamic_cast<TreeBaseCx*>(tree)->GetStatement());
}
void Translator::TraverseEx(ExpBase* exp){
    if(exp==0)
        return;
    switch(exp->Kind()){
        case ExpBase::kExpBase_Binop:
            m_logger.D("ADD(");
            TraverseEx( dynamic_cast<ExpBaseBinop*>(exp)->Left() );
            m_logger.D(",");
            TraverseEx( dynamic_cast<ExpBaseBinop*>(exp)->Right() );
            m_logger.D(")");
            break;
        case ExpBase::kExpBase_Mem:
            m_logger.D("MEM(");
            TraverseEx( dynamic_cast<ExpBaseMem*>(exp)->GetExp() );
            m_logger.D(")");
            break;
        case ExpBase::kExpBase_Temp:
            m_logger.D("TEMP(%s)",dynamic_cast<ExpBaseTemp*>(exp)->GetTemp()->Name());
            break;
        case ExpBase::kExpBase_Eseq:
            break;
        case ExpBase::kExpBase_Name:
            m_logger.D("NAME(%s)",dynamic_cast<ExpBaseName*>(exp)->GetLabel()->Name());
            break;
        case ExpBase::kExpBase_Const:
            m_logger.D("CONST(%d)",dynamic_cast<ExpBaseConst*>(exp)->GetValue());
            break;
        case ExpBase::kExpBase_Call:
            break;
    }
}
void Translator::TraverseNx(StatementBase* statement){
    if(statement==0)
        return;
    switch(statement->Kind())
    {
        case StatementBase::kStatement_Seq:
            TraverseNx( dynamic_cast<StatementSeq*>(statement)->Left() );
            TraverseNx( dynamic_cast<StatementSeq*>(statement)->Right() );
            break;
        case StatementBase::kStatement_Label:
            m_logger.D("LABEL(%s)",dynamic_cast<StatementLabel*>(statement)->GetLabel()->Name());
            break;
        case StatementBase::kStatement_Jump:
            break;
        case StatementBase::kStatement_Cjump:
            break;
        case StatementBase::kStatement_Move:
            m_logger.D("MOVE(");
            TraverseEx( dynamic_cast<StatementMove*>(statement)->Left() );
            m_logger.D(",");
            TraverseEx( dynamic_cast<StatementMove*>(statement)->Right() );
            m_logger.D(")");
            break;
        case StatementBase::kStatement_Exp:
            break;
    }
}
void Translator::TraverseCx(StatementBase* statement){
    
}
    
}//namespace tiger
