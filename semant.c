#include <iostream>
#include "semant.h"
#include "tiger_assert.h"
#include "frame.h"
#include "tree.h"

namespace tiger{
    
Translator::Translator(){
    m_level_manager = new LevelManager;
    
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("semant");
    
    m_outer_most_level = 0;
    
    m_lit_string_list = new LitStringList;
    m_frag_list = new FragList;
    //frame pointer
    m_fp = 0;
}
Translator::~Translator(){
    delete m_level_manager;
    
    delete m_lit_string_list;
    
    delete m_frag_list;
    
    //free all temp label memory
    TempLabel::Exit();
}

Level*      Translator::OuterMostLevel()
{
    if(m_outer_most_level==0)
    {
        m_outer_most_level = new Level(0, new FrameBase(FrameBase::kFrame_X86));
        m_level_manager->NewLevel(m_outer_most_level);
    }
    return m_outer_most_level;
}
LetExp* Translator::For2Let(ForExp* exp)
{
    LetExp* let_exp = 0;
    DecList* decs = 0;
    VarDec* var1 = new VarDec(exp->GetVar()->Clone(),0,exp->GetLo()->Clone());
    VarDec* var2 = new VarDec(new Symbol("limit"),0,exp->GetHi()->Clone());
    DecNode* dec1 = new DecNode;
    DecNode* dec2 = new DecNode;
    dec1->m_dec = var1;
    dec2->m_dec = var2;
    dec1->next = dec2;
    dec2->prev = dec1;
    decs = new DecList(dec1);
    
    WhileExp* while_exp = 0;
    OpExp* test = new OpExp(new Oper(Oper::kOper_Le),new VarExp(new SimpleVar(exp->GetVar()->Clone())),new VarExp(new SimpleVar(new Symbol("limit"))));
    
    AssignExp* assign = new AssignExp(new SimpleVar(exp->GetVar()->Clone()), new OpExp(new Oper(Oper::kOper_Add),new VarExp(new SimpleVar(exp->GetVar()->Clone())),new IntExp(1)) );
    
    SeqExp* body = 0;
    
    ExpList* list = 0;
    ExpNode* exp1 = new ExpNode;
    ExpNode* exp2 = new ExpNode;
    exp1->m_exp = exp->GetExp()->Clone();
    TIGER_ASSERT(exp1->m_exp->Kind()==Exp::kExp_Assign,"m_exp is null!!");
    exp2->m_exp = assign;
    exp1->next = exp2;
    exp2->prev = exp1;
    list = new ExpList(exp1);
    body = new SeqExp(list);
    
    while_exp = new WhileExp(test,body);
    
    let_exp = new LetExp(decs,while_exp);
    
    return let_exp;
}  
ExpBaseTy*  Translator::TransVar(SymTab* venv,SymTab* tenv,Level* level,Var* var,Label* done_label){
    m_logger.D("TransVar with kind %d",var->Kind());
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol())));
            TIGER_ASSERT(t!=0,"var %s not found",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            
            Level* alevel=0;
            TreeBaseEx* ex;
            ExpBase* tmp=0;/* used to calc static link */
            AccessFrame* af;
            AccessReg*   ar;
            if(t->Access()->GetAccess()->Kind()==AccessBase::kAccess_Frame){
                TIGER_ASSERT(t->Access()->GetLevel()!=level,"level must different!!");
                af = dynamic_cast<AccessFrame*>(t->Access()->GetAccess());
                //ex = new TreeBaseEx( new ExpBaseMem( new ExpBaseBinop(BinaryOp::kBinaryOp_Add,new ExpBaseTemp( FP() ),new ExpBaseConst(af->Offset()))) );
                /*
                a frame variable should access by static link
                t->Access()->GetLevel() -- the level where var declaration 
                level -- the level where var being used
                
                */
                alevel = level;
                TIGER_ASSERT(alevel!=0,"level is null!!");
                while(alevel!=t->Access()->GetLevel()){
                    if(tmp==0){
                        tmp = new ExpBaseMem(
                            new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( FP() ), new ExpBaseConst(0/* static link's offset*/)
                            )
                        );
                    }else{
                        tmp = new ExpBaseMem(
                            new ExpBaseBinop( BinaryOp::kBinaryOp_Add, tmp, new ExpBaseConst(0/* static link's offset*/)
                            )
                        );
                    }
                    alevel=alevel->Parent();
                }
                ex = new TreeBaseEx(
                    new ExpBaseMem(
                        new ExpBaseBinop( BinaryOp::kBinaryOp_Add, tmp, new ExpBaseConst( af->Offset() )
                        )
                    )
                );
            }
            if(t->Access()->GetAccess()->Kind()==AccessBase::kAccess_Reg){
                //TIGER_ASSERT(t->Access()->GetLevel()==level,"level must match!!");
                ar = dynamic_cast<AccessReg*>(t->Access()->GetAccess());
                ex = new TreeBaseEx( new ExpBaseTemp( ar->GetTemp() ) );
                
            }
            
            return new ExpBaseTy(t->Type(),ex);//
        }
        case Var::kVar_Field:
        {
            ExpBaseTy* p;
            TypeFieldNode* head;
            p = TransVar(venv,tenv,level,dynamic_cast<FieldVar*>(var)->GetVar(),done_label);
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
            p = TransVar(venv,tenv,level,dynamic_cast<SubscriptVar*>(var)->GetVar(),done_label);
            if(p->Type()->Kind()!=TypeBase::kType_Name){
                m_logger.W("name type needed");
            }
            TIGER_ASSERT(p!=0,"name type needed");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type()
            
            t = TransExp(venv,tenv,level,dynamic_cast<SubscriptVar*>(var)->GetExp(),done_label);
            if(t->Type()->Kind()!=TypeBase::kType_Int){
                m_logger.W("array index should be int");
            }
            TIGER_ASSERT(p!=0,"array index should be int");
            
            ret = new ExpBaseTy(dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),new TreeBaseEx(new ExpBaseConst(0))); 
            
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
ExpBaseTy*  Translator::TransExp(SymTab* venv,SymTab* tenv,Level* level,Exp* exp,Label* done_label/* for break in while or for*/){
    switch(exp->Kind())
    {
        case Exp::kExp_Var:
        {
            return TransVar(venv,tenv,level,dynamic_cast<VarExp*>(exp)->GetVar(),done_label);
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
        
            ExpBaseList* explist;
            explist = new ExpBaseList;
        
            EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<CallExp*>(exp)->Name())));
            TIGER_ASSERT(f!=0,"function name not found",dynamic_cast<CallExp*>(exp)->Name());
            /* function foo() */
            if(f->GetList()->GetHead()==0){
                TIGER_ASSERT(dynamic_cast<CallExp*>(exp)->GetList()->GetHead()==0,"function actuals should be empty");
                /*
                TreeBase* tree;
                tree = new TreeBaseEx(
                    new ExpBaseEseq(
                        new StatementMove( new ExpBaseMem(new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( FP() ), new ExpBaseConst(0)) ), new ExpBaseTemp( FP() ) ),
                        new ExpBaseCall( new ExpBaseName(f->GetLabel()), 0 )) );
                */
                TreeBase* tree;
                tree = new TreeBaseEx(
                    new ExpBaseEseq(
                        new StatementMove( new ExpBaseMem(new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( FP() ), new ExpBaseConst(0)) ), new ExpBaseTemp( FP() ) ),
                        new ExpBaseCall( new ExpBaseName(f->GetLabel()), 0 )) );
                return new ExpBaseTy(f->Type(),tree);
            }
            p = f->GetList()->GetHead();
            TIGER_ASSERT(p!=0,"%s formals is null",dynamic_cast<CallExp*>(exp)->Name());
            head = dynamic_cast<CallExp*>(exp)->GetList()->GetHead();
            TIGER_ASSERT(head!=0,"actuals is null");
            StatementBase* st=0;
            AccessList* al=0;
            s32 j=1;//0 for static link
            al = f->GetLevel()->Frame()->GetFormals();
            //static link
            st = new StatementMove( new ExpBaseMem(new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( FP() ), new ExpBaseConst(0)) ), new ExpBaseTemp( FP() ) );
            while(head){
                t = TransExp(venv,tenv,level,head->m_exp,done_label);
                if(p->m_field->Type()!=t->Type()){
                    TIGER_ASSERT(0,"type mismatch");
                }
                
                // formal args
                if(al->Get(j)->Kind()==AccessBase::kAccess_Frame){
                    //dynamic_cast<AccessFrame*>(al->Get(j))->Offset()
                    st = new StatementSeq(st,
                         new StatementMove( new ExpBaseMem(new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( FP() ), new ExpBaseConst(dynamic_cast<AccessFrame*>(al->Get(j))->Offset())) ), TreeBase::UnEx(t->Tree()) )
                    );
                }else{
                    //dynamic_cast<AccessReg*>(al->Get(j))->GetTemp()
                    st = new StatementSeq(st,
                         new StatementMove( new ExpBaseTemp( dynamic_cast<AccessReg*>(al->Get(j))->GetTemp() ), TreeBase::UnEx(t->Tree()) )
                    );
                }
                
                
                 
                
                explist->Insert( TreeBase::UnEx(t->Tree())->Clone(), ExpBaseList::kExpBaseList_Rear);
                delete t;
                head = head->next;
                p = p->next;
                j++;
            }
            /*
            static list 
            f->GetLevel()->Frame()
            */
            TreeBase* tree;
            tree = new TreeBaseEx(
                    new ExpBaseEseq(
                        st,
                        new ExpBaseCall( new ExpBaseName(f->GetLabel()), explist )) );
            return new ExpBaseTy(f->Type(),tree);// new TreeBaseEx( new ExpBaseCall( new ExpBaseName(f->GetLabel()), explist) )
            break;
        }
        case Exp::kExp_Op:
        {
            m_logger.D("type check with kExp_Op");
            Oper* op = dynamic_cast<OpExp*>(exp)->GetOper();
            ExpBaseTy* left,*right;
            TreeBase* l, *r;
            left = TransExp(venv,tenv,level,dynamic_cast<OpExp*>(exp)->GetLeft(),done_label);
            right = TransExp(venv,tenv,level,dynamic_cast<OpExp*>(exp)->GetRight(),done_label);
            l = left->Tree();
            r = right->Tree();
            /* compare */
            if(op->Kind()==Oper::kOper_Lt||
               op->Kind()==Oper::kOper_Le||
               op->Kind()==Oper::kOper_Gt||
               op->Kind()==Oper::kOper_Ge){
                TreeBaseCx* ex;
                StatementBase* statement;
                PatchList * ts = new PatchList;
                PatchList * fs = new PatchList;
                statement = new StatementCjump( RelationOp::ToRelationOp(op->Kind()), TreeBase::UnEx(l), TreeBase::UnEx(r), 0/* in place */, 0/* in place */);
                ts->Insert( dynamic_cast<StatementCjump*>(statement)->GetATrueLabel(), PatchList::kPatchList_Rear );
                fs->Insert( dynamic_cast<StatementCjump*>(statement)->GetAFalseLabel(), PatchList::kPatchList_Rear );
                ex = new TreeBaseCx( statement, ts, fs);
                
                delete left;
                delete right;
                
                Symbol t("int");
                return new ExpBaseTy( tenv->Type(tenv->MakeSymbol(&t)), ex );
            }
            if(op->Kind()==Oper::kOper_Add||
               op->Kind()==Oper::kOper_Sub||
               op->Kind()==Oper::kOper_Mul||
               op->Kind()==Oper::kOper_Div){
                if(left->Type()->Kind()!=TypeBase::kType_Int)
                    std::cout<<"type error"<<std::endl;
                if(right->Type()->Kind()!=TypeBase::kType_Int)
                    std::cout<<"type error"<<std::endl;
                
                
                TreeBaseEx* ex = new TreeBaseEx( new ExpBaseBinop(BinaryOp::ToBinaryOp(op->Kind()),TreeBase::UnEx(l),TreeBase::UnEx(r)) );
                
                delete left;
                delete right;
                
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),ex);
            }
             
           
            TreeBaseEx* ex = new TreeBaseEx( new ExpBaseBinop(BinaryOp::ToBinaryOp(op->Kind()),TreeBase::UnEx(l),TreeBase::UnEx(r)) );
                
            delete left;
            delete right;
            
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),ex);
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
                
                a = TransExp(venv,tenv,level,head->m_efield->GetExp(),done_label);
                
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
                tmp = TransExp(venv,tenv,level,p->m_exp,done_label);
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
                result = new TreeBaseEx( new ExpBaseEseq( statement, TreeBase::UnEx( tmp->Tree() ) ) );
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
            //TraverseEx( TreeBase::UnEx(result) );
            return new ExpBaseTy(ty,result);
        }
        case Exp::kExp_Assign:
        {
            ExpBaseTy* a;
            ExpBaseTy* b;
            ExpBaseTy* result;
            
            m_logger.D("type check with kExp_Assign");
            
            a = TransVar(venv,tenv,level,dynamic_cast<AssignExp*>(exp)->GetVar(),done_label);
            b = TransExp(venv,tenv,level,dynamic_cast<AssignExp*>(exp)->GetExp(),done_label);
            
            TIGER_ASSERT(a!=0,"var type is null");
            TIGER_ASSERT(b!=0,"exp type is null");
            
            //m_logger.D("var type:%s",a->Type()->TypeString());
            //m_logger.D("var type:%s",b->Type()->TypeString());
            
            TIGER_ASSERT(a->Type()==b->Type(),"type mismatch");
            
            //new TreeBaseNx( new StatementMove( TreeBase::UnEx( a->Tree() ), TreeBase::UnEx( b->Tree() ) ) );
            
            //TraverseEx( TreeBase::UnEx(a->Tree()) );
            //TraverseEx( TreeBase::UnEx(b->Tree()) );
        
            result = new ExpBaseTy( b->Type(), new TreeBaseNx( new StatementMove( TreeBase::UnEx( a->Tree() ), TreeBase::UnEx( b->Tree() ) ) ) );
            
            delete a;
            delete b;
            //TraverseNx( TreeBase::UnNx(result->Tree()) );
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
            
            StatementBase* statement;
        
            Label *t = TempLabel::NewLabel();
            Label *f = TempLabel::NewLabel();
            Label *z = TempLabel::NewLabel();
            Temp* tmp = TempLabel::NewTemp();
            a = TransExp(venv,tenv,level,if_exp,done_label);
            dynamic_cast<TreeBaseCx*>( a->Tree() )->GetTrues()->DoPatch(t);
            dynamic_cast<TreeBaseCx*>( a->Tree() )->GetFalses()->DoPatch(f);
            
                
            b = TransExp(venv,tenv,level,then_exp,done_label);
            //LabelList* llist = new LabelList;
            //llist->Insert(z,LabelList::kLabelList_Rear);
            statement = new StatementSeq(
                new StatementSeq(
                    new StatementSeq(TreeBase::UnCx(a->Tree())->GetStatement(),new StatementLabel(t)
                    ), 
                    new StatementExp(TreeBase::UnEx(b->Tree()))
                ), 
                new StatementLabel(f)
            );
            if(else_exp)
                c = TransExp(venv,tenv,level,else_exp,done_label);
            
            TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"if exp should be int");
            
            delete a;
            
            if(else_exp){
                delete b;
                delete c;
                //return c;
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseNx(statement));
            }else{
                //return b;
                delete b;
                Symbol t("int");
                return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseNx(statement));
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
            
            StatementBase* statement=0;
            Label* test_label;
            Label* body_label;
            Label* adone_label;
            test_label = TempLabel::NewLabel();
            body_label = TempLabel::NewLabel();
            adone_label = TempLabel::NewLabel();
            statement = new StatementLabel(test_label);
            
            test_exp = dynamic_cast<WhileExp*>(exp)->GetTest();
            body_exp = dynamic_cast<WhileExp*>(exp)->GetExp();
            
        
            TIGER_ASSERT(test_exp!=0,"while exp is null");
            TIGER_ASSERT(body_exp!=0,"while body is null");
            
            a = TransExp(venv,tenv,level,test_exp,done_label);
            
            statement = new StatementSeq(statement,
                            new StatementCjump(RelationOp::kRelationOp_Eq, TreeBase::UnEx(a->Tree()), new ExpBaseConst(1), body_label/*true label*/, adone_label/*false label*/)
                        );
                
            venv->BeginScope(ScopeMaker::kScope_While);
            b = TransExp(venv,tenv,level,body_exp,adone_label);
            venv->EndScope();
            
            statement = new StatementSeq(statement,
                            TreeBase::UnNx(b->Tree()));
        
            LabelList* llist = new LabelList;
            llist->Insert(test_label,LabelList::kLabelList_Rear);
        
            statement = new StatementSeq(statement,
                            new StatementJump(new ExpBaseName(test_label),llist));
            
            statement = new StatementSeq(statement,new StatementLabel(adone_label));
        
            TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"while exp should be int");
            
            delete a;
            delete b;
            
            /* default type */
            Symbol t("int");
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseNx(statement));
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
            LabelList* llist = new LabelList;
            llist->Insert(done_label,LabelList::kLabelList_Rear);
            
            return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),new TreeBaseNx(new StatementJump(new ExpBaseName(done_label),llist)));
            break;
        }
        case Exp::kExp_For:
        {
            m_logger.D("type check with kExp_For");
        
        
            Symbol* var;
            Exp* lo_exp;
            Exp* hi_exp;
            Exp* body_exp;
            
            ExpBaseTy* a;
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
            b = TransExp(venv,tenv,level,lo_exp,done_label);
            c = TransExp(venv,tenv,level,hi_exp,done_label);
            
            
            
            //TIGER_ASSERT(a->Type()->Kind()==TypeBase::kType_Int,"for var should be int");
            TIGER_ASSERT(b->Type()->Kind()==TypeBase::kType_Int,"for lo should be int");
            TIGER_ASSERT(c->Type()->Kind()==TypeBase::kType_Int,"for hi should be int");
            
            venv->BeginScope(ScopeMaker::kScope_For);
            //tenv->BeginScope();
            
            venv->Enter(venv->MakeSymbol(var),new EnvEntryVar(b->Type(), EnvEntryVar::kEnvEntryVar_For_Value, 0));
            
            d = TransExp(venv,tenv,level,body_exp,done_label);
            
            //tenv->EndScope();
            venv->EndScope();
            
            // release what we don't use
            ReleaseTree( b->Tree() );
            ReleaseTree( c->Tree() );
            ReleaseTree( d->Tree() );
        
            delete b;
            delete c;
            delete d;
            // type check ok
            LetExp* let_exp;
            let_exp = For2Let( dynamic_cast<ForExp*>(exp) );
            
            a = TransExp(venv,tenv,level,let_exp,done_label);
            delete let_exp;
        
            return a;
        
            //delete a;
            //Symbol t("int");
            //return new ExpBaseTy(tenv->Type(tenv->MakeSymbol(&t)),0);
            //break;
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
            
            // for each let expression we create a new level
            // wrong here, we should use frame
            //alevel = new Level(level,MakeNewFrame( 0 ));
            //m_level_manager->NewLevel(alevel);
        
            // dec list
            DecNode* p;
            if(declist){
                p = declist->GetHead();
                while(p){
                    m_logger.D("TransDec var a:=1");
                    tree = TransDec(venv,tenv,level,p->m_dec,done_label);
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
                ret = TransExp(venv,tenv,level,body,done_label);
                if(statement){
                    //m_logger.D("new statement seq");
                    //TIGER_ASSERT(TreeBase::UnNx(ret->Tree())!=0,"right is null!!");
                    //TraverseNx( TreeBase::UnNx(ret->Tree()) );
                    statement = new StatementSeq( statement, TreeBase::UnNx(ret->Tree()) );
                }else{
                    statement = TreeBase::UnNx( ret->Tree() );
                }
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
            size_ty = TransExp(venv,tenv,level,dynamic_cast<ArrayExp*>(exp)->GetSize(),done_label);
            init_ty = TransExp(venv,tenv,level,dynamic_cast<ArrayExp*>(exp)->GetInit(),done_label);
            
            TIGER_ASSERT(size_ty!=0,"array size type is null");
            TIGER_ASSERT(init_ty!=0,"array init type is null");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())
            TIGER_ASSERT(size_ty->Type()->Kind()==TypeBase::kType_Int,"array size type error");
            TIGER_ASSERT(init_ty->Type()==dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),"array init type mismatch");
            
            delete size_ty;
            delete init_ty;
            
            return new ExpBaseTy(p->Type(),new TreeBaseEx(new ExpBaseConst(0)));
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

void Translator::TransFunctionDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label)
{
    /*
    stack frame:
    - static link
    - formal args
    - local args
    */
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
        
        /* each function needs its own level information */
        alevel = dynamic_cast<EnvEntryFun*>( venv->Lookup( venv->MakeSymbol(fundec_head->m_fundec->Name()) ))->GetLevel();
        TIGER_ASSERT(alevel!=0,"function level is empty!");
        
        /* function foo() */
        if(fundec_head->m_fundec->GetList()!=0)
            head = fundec_head->m_fundec->GetList()->GetHead();
        
        venv->BeginScope(ScopeMaker::kScope_Fun);
        if(fundec_head->m_fundec->GetList()!=0){
            s32 i=1;//0 for static link
            while(head){
                VarAccess* access = 0;
                AccessFrame* af=0;
                AccessReg*   ar=0;
                if(head->m_field->Name()->GetEscape()==1){
                    af = dynamic_cast<AccessFrame*>( alevel->Frame()->GetFormals()->Get(i) );
                    access = new VarAccess(level,af);
                    // new ExpBaseName()
                }else{
                    ar = dynamic_cast<AccessReg*>( alevel->Frame()->GetFormals()->Get(i) );
                    m_logger.D("----------------formal arg :%s",ar->GetTemp()->Name());
                    access = new VarAccess(level,ar);
                }
                
                venv->Enter(venv->MakeSymbol(head->m_field->Name()),new EnvEntryVar( dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type(), EnvEntryVar::kEnvEntryVar_For_Value, access ) );
                head = head->next;
                i++;
            }
        }
        
        a = TransExp(venv,tenv,alevel,fundec_head->m_fundec->GetExp(),done_label);
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
        
        // stack frame alevel->Frame()->Size()
        
        m_frag_list->Insert( dynamic_cast<EnvEntryFun*>( venv->Lookup( venv->MakeSymbol(fundec_head->m_fundec->Name()) ))->GetLabel(),
                             new Frag( TreeBase::UnNx(a->Tree()), alevel->Frame() ) );
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
TreeBase* Translator::TransDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            ExpBaseTy* t=TransExp(venv,tenv,level,dynamic_cast<VarDec*>(dec)->GetExp(),done_label);
            TreeBase* tree;
            AccessFrame* af;
            AccessReg*   ar;
            VarAccess* access;
            
            tree = t->Tree();
            m_logger.D("~~~~~~~~~~~~~ type size: %d",t->Type()->Size());
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
            TransFunctionDec(venv,tenv,level,dec,done_label);
            return new TreeBaseEx( new ExpBaseConst(0) );
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
    if(tree==0)
        return 0;
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
FragList::FragList()
{
    m_tab = new FragNode*[kFragList_Size];
    for(s32 i=0;i<kFragList_Size;i++)
        m_tab[i]=0;
    m_size = 0;
}
FragList::~FragList()
{
    for(s32 i=0;i<kFragList_Size;i++)
        Clear(m_tab[i]);
    delete[] m_tab;
}
void FragList::Clear(FragNode* head)
{
    FragNode* p;
    p = head;
    while(p){
        head = head->next;
        delete p;
        p = head;
    }
}
void FragList::Insert(Label* l,Frag* frag)
{
    s32 index = hash(l);
    FragNode* p = m_tab[index];
    FragNode* n;
    while(p){
        if(p->m_label==l)
            return;//already exist
        p = p->next;
    }
    n = new FragNode;
    n->m_label = l;
    n->m_frag = frag;
    n->next = m_tab[index];
    if(m_tab[index])
        m_tab[index]->prev = n;
    m_tab[index] = n;
    m_size++;
}
Frag* FragList::Find(Label* l)
{
    s32 index = hash(l);
    FragNode* p = m_tab[index];
    while(p){
        if(p->m_label==l)
            return p->m_frag;
        p = p->next;
    }
    //not found
    return 0;
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
            TraverseNx( dynamic_cast<ExpBaseEseq*>(exp)->GetStatement() );
            TraverseEx( dynamic_cast<ExpBaseEseq*>(exp)->GetExp() );
            break;
        case ExpBase::kExpBase_Name:
            m_logger.D("NAME(%s)",dynamic_cast<ExpBaseName*>(exp)->GetLabel()->Name());
            break;
        case ExpBase::kExpBase_Const:
            m_logger.D("CONST(%d)",dynamic_cast<ExpBaseConst*>(exp)->GetValue());
            break;
        case ExpBase::kExpBase_Call:
            m_logger.D("CALL(%s)",dynamic_cast<ExpBaseName*>(dynamic_cast<ExpBaseCall*>(exp)->GetExp())->GetLabel()->Name());
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
            m_logger.D("JMP (%s)",dynamic_cast<StatementJump*>(statement)->GetList()->GetHeadLabel()->Name());
            break;
        case StatementBase::kStatement_Cjump:
            m_logger.D("CJUMP(");
            m_logger.D("op,");
            TraverseEx(dynamic_cast<StatementCjump*>(statement)->Left());
            m_logger.D(",");
            TraverseEx(dynamic_cast<StatementCjump*>(statement)->Right());
            m_logger.D(",");
            m_logger.D("%s,%s)",dynamic_cast<StatementCjump*>(statement)->GetTrueLabel()->Name(),dynamic_cast<StatementCjump*>(statement)->GetFalseLabel()->Name());
            break;
        case StatementBase::kStatement_Move:
            m_logger.D("MOVE(");
            TraverseEx( dynamic_cast<StatementMove*>(statement)->Left() );
            m_logger.D(",");
            TraverseEx( dynamic_cast<StatementMove*>(statement)->Right() );
            m_logger.D(")");
            break;
        case StatementBase::kStatement_Exp:
            TraverseEx( dynamic_cast<StatementExp*>(statement)->GetExp() );
            break;
    }
}
void Translator::TraverseCx(StatementBase* statement){
    TraverseNx(statement);
}
void Translator::TraverseFragList()
{
    m_frag_list->Walk(this);
}

}//namespace tiger
