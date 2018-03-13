#include "tree_gen.h"
#include "tiger_assert.h"

namespace tiger{

TreeGenerator::TreeGenerator()
{
    m_logger.SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger.SetModule("tree gen");
    
    m_level_manager = new LevelManager;
    
    //init literal string list
    m_lit_string_list = new LitStringList;
    
    m_frag_list = new FragList;
    
    m_outer_most_level = 0;
    
    m_fp = TempLabel::NewTemp();
    m_sp  = TempLabel::NewTemp();
    m_rv  = TempLabel::NewTemp();
    
    m_temp_map_list = new TempMapList;
    m_temp_map_list->Enter(m_fp,"%RBP");
    m_temp_map_list->Enter(m_sp,"%RSP");
    m_temp_map_list->Enter(TempLabel::NewNamedTemp("RDI"),"%RDI");
}
TreeGenerator::~TreeGenerator()
{
    delete m_lit_string_list;
    delete m_level_manager;
    delete m_frag_list;
    delete m_temp_map_list;
    //free all temp label memory
    TempLabel::Exit();
}
ExpBase*       TreeGenerator::UnEx(TreeGenResult* tr)
{
    TIGER_ASSERT(tr!=0,"tr is null");
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Ex)
    {
        return tr->m_exp;
    }
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Nx)
    {
        return new ExpBaseEseq( tr->m_statement,new ExpBaseConst( 0 ) );
    }
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Cx)
    {
        /* 
           y := a>10
           when translate a>10, generate cjump statement, true and false labels have not filled. Fill them here.
        */
        Temp* r;
        Label* t,*f;
        
        r = TempLabel::NewTemp();
        t = TempLabel::NewLabel();
        f = TempLabel::NewLabel();
        
        tr->m_trues->DoPatch(t);
        tr->m_falses->DoPatch(f);
        
        return new ExpBaseEseq(new StatementMove(new ExpBaseTemp(r),new ExpBaseConst(1)),
                                  new ExpBaseEseq( tr->m_statement,
                                     new ExpBaseEseq( new StatementLabel(f),
                                        new ExpBaseEseq(new StatementMove(new ExpBaseTemp(r),new ExpBaseConst(0)),
                                           new ExpBaseEseq(new StatementLabel(t),new ExpBaseTemp(r)
                                           )
                                        )
                                     )
                                  )
                              );
    }
}
StatementBase* TreeGenerator::UnNx(TreeGenResult* tr)
{
    TIGER_ASSERT(tr!=0,"tr is null");
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Ex)
    {
        /*
        (a;2)
        */
        return new StatementExp( tr->m_exp );
    }
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Nx)
    {
        /*
        a:=1
        */
        return tr->m_statement;
    }
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Cx)
    {
        /*
        (a>b,0)
        */
        Label* t,*f;
        t = TempLabel::NewLabel();
        f = TempLabel::NewLabel();
        
        tr->m_trues->DoPatch(t);
        tr->m_falses->DoPatch(f);
        
        return new StatementSeq( tr->m_statement,
                   new StatementSeq( new StatementLabel(t), new StatementLabel(f)) );
    }
}
TreeGenResult* TreeGenerator::UnCx(TreeGenResult* tr)
{
    TIGER_ASSERT(tr!=0,"tr is null");
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Ex)
    {
        /*
        if a then x1 else x2
        */
        StatementBase* tmp;
        PatchList * trues = new PatchList;
        PatchList* falses = new PatchList;
        
        tmp = new StatementCjump(RelationOp::kRelationOp_Ne,tr->m_exp,new ExpBaseConst(0),0/*true*/,0/*false*/);
        
        trues->Insert(dynamic_cast<StatementCjump*>(tmp)->GetATrueLabel(),PatchList::kPatchList_Rear);
        falses->Insert(dynamic_cast<StatementCjump*>(tmp)->GetAFalseLabel(),PatchList::kPatchList_Rear);
        
        return new TreeGenResult(tr->m_type,tmp,trues,falses);
    }
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Nx)
    {
        TIGER_ASSERT(0,"Should not reach here");
    }
    if(tr->Kind()==TreeGenResult::kTreeGenResult_Cx)
    {
        return tr;
    }
}
TreeGenResult*  TreeGenerator::TreeGenVar(SymTab* venv,SymTab* tenv,Level* level,Var* var,Label* done_label)
{
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol())));
            TIGER_ASSERT(t!=0,"var %s not found",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            
            Level* alevel=0;
            ExpBase* tmp=0;/* used to calc static link */
            AccessFrame* af;
            AccessReg*   ar;
            //frame
            if(t->Access()->GetAccess()->Kind()==AccessBase::kAccess_Frame){
                af = dynamic_cast<AccessFrame*>(t->Access()->GetAccess());
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
                if(tmp==0){// in the same level
                    tmp = new ExpBaseTemp( FP() );
                }
                return new TreeGenResult(t->Type(),
                    new ExpBaseMem(
                        new ExpBaseBinop( BinaryOp::kBinaryOp_Add, tmp, new ExpBaseConst( af->Offset() )
                        )
                    )
                );
            }
            //reg
            if(t->Access()->GetAccess()->Kind()==AccessBase::kAccess_Reg){
                ar = dynamic_cast<AccessReg*>(t->Access()->GetAccess());
                return new TreeGenResult( t->Type(), new ExpBaseTemp( ar->GetTemp() ) );
                
            }
        }
        case Var::kVar_Field:
        {
            TreeGenResult* p;
            TypeFieldNode* head;
            ExpBase* tmp;
            p = TreeGenVar(venv,tenv,level,dynamic_cast<FieldVar*>(var)->GetVar(),done_label);

            head = dynamic_cast<TypeRecord*>(dynamic_cast<TypeName*>(p->Type())->Type())->GetList()->GetHead();
            s32 i_offset=0;// field offset in Record
            while(head){
                if(head->m_field->Name()==tenv->MakeSymbol(dynamic_cast<FieldVar*>(var)->GetSym())){
                    /* ok */
                    tmp = UnEx(p);
                    delete p;
                    return new TreeGenResult( head->m_field->Type(), new ExpBaseMem( new ExpBaseBinop( BinaryOp::kBinaryOp_Add, tmp, new ExpBaseConst(i_offset) ) ) );
                }
                i_offset += head->m_field->Type()->Size();
                head = head->next;
            }
            TIGER_ASSERT(0,"%s not found in record type",dynamic_cast<FieldVar*>(var)->GetSym()->Name());
            
            break;

        }
        case Var::kVar_Subscript:
        {
            TreeGenResult* p;
            TreeGenResult* t;
            TreeGenResult* ret;
            ExpBase* tree;
            p = TreeGenVar(venv,tenv,level,dynamic_cast<SubscriptVar*>(var)->GetVar(),done_label);
            if( (p->Type())->Kind()!=TypeBase::kType_Name){
                m_logger.W("name type needed");
            }
            TIGER_ASSERT(p!=0,"name type needed");
            
            //dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type()
            
            t = TreeGenExp(venv,tenv,level,dynamic_cast<SubscriptVar*>(var)->GetExp(),done_label);
            
            
            tree =  new ExpBaseMem( new ExpBaseBinop( BinaryOp::kBinaryOp_Add, UnEx(p), UnEx(t) ) );
            
            ret = new TreeGenResult(dynamic_cast<TypeArray*>(dynamic_cast<TypeName*>(p->Type())->Type())->Type(),tree); 
            
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
void TreeGenerator::ProceeExternalFunctions(SymTab* venv,SymTab* tenv)
{
    TypeFieldNode* head;
    Level* alevel;
    //for all external function, create level for them
    EnvEntryFun* p = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbolFromString("printint")));
    head = p->GetList()->GetHead();
    
    FrameBase* f;
    
    AccessBase* access;
    AccessList* al;
    BoolList* bl;
    f = new FrameBase(FrameBase::kFrame_X86);
    al = f->GetFormals();
    bl = f->GetEscapes();
    
    while(head){
        /*
        for external function, tiger put args in frame
        */
        access = f->AllocLocal(1/*true*/);
        access->Retain();//inc refcnt
        al->Insert(access,AccessList::kAccessList_Rear);
        
        bl->Insert(BoolNode::kBool_True,BoolList::kBoolList_Rear);
        
        head = head->next;
    }
    alevel = new Level(OuterMostLevel(),f);
    m_level_manager->NewLevel(alevel);
    p->SetLevel( alevel );
}
Level*      TreeGenerator::OuterMostLevel()
{
    if(m_outer_most_level==0)
    {
        m_outer_most_level = new Level(0, new FrameBase(FrameBase::kFrame_X86));
        
        /* the out most level don't need static link and formal args */
        m_level_manager->NewLevel(m_outer_most_level);
    }
    return m_outer_most_level;
}
TreeGenResult* TreeGenerator::TreeGenExpCall(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    ExpNode* head;
    TypeFieldNode* p;
    TreeGenResult* t;

    ExpBaseList* explist;
    explist = new ExpBaseList;

    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Call,"call exp expected");
    
    EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<CallExp*>(exp)->Name())));
    TIGER_ASSERT(f!=0,"function name not found",dynamic_cast<CallExp*>(exp)->Name());
    
    // tree code
    StatementBase* st=0;
    AccessList* al=0;
    s32 j=0;//0 for args start
    TIGER_ASSERT(f->GetLevel()!=0,"null frame");

    //interal function
    al = f->GetLevel()->Frame()->GetFormals();
    
    // allocate frame
    st = new StatementMove( new ExpBaseTemp( SP() ),
                                 new ExpBaseBinop( BinaryOp::kBinaryOp_Sub, new ExpBaseTemp( FP() ), 
                                                     new ExpBaseConst( f->GetLevel()->Frame()->Size() )
                                                 )
                             );
    //  args
    head = dynamic_cast<CallExp*>(exp)->GetList()->GetHead();
    while(head){
        t = TreeGenExp(venv,tenv,level,head->m_exp,done_label);

        
        // formal args
        if(al->Get(j)->Kind()==AccessBase::kAccess_Frame){//always here
            //dynamic_cast<AccessFrame*>(al->Get(j))->Offset()
            st = new StatementSeq(st,
                 new StatementMove( new ExpBaseMem(new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( SP() ), new ExpBaseConst(dynamic_cast<AccessFrame*>(al->Get(j))->Offset())) ), UnEx(t) )
            );
            // for x86_64
            // rdi rsi rdx rcx r8 r9
            st = new StatementSeq(
                    st,
                    new StatementMove(
                        new ExpBaseTemp(TempLabel::NewNamedTemp("RDI")),
                        UnEx(t)->Clone()
                    )
            );
        }else{
            //dynamic_cast<AccessReg*>(al->Get(j))->GetTemp()
            st = new StatementSeq(st,
                 new StatementMove( new ExpBaseTemp( dynamic_cast<AccessReg*>(al->Get(j))->GetTemp() ), UnEx(t) )
            );
        }
        
        explist->Insert( UnEx(t)->Clone(), ExpBaseList::kExpBaseList_Rear);
        
        delete t;
        
        head = head->next;
        j++;
    }
    //static link
    if(f->Kind()==EnvEntryFun::kFunction_Internal)
    {
        st = new StatementSeq(st,
                     new StatementMove( 
                         new ExpBaseMem(
                             new ExpBaseBinop( BinaryOp::kBinaryOp_Add, new ExpBaseTemp( SP() ), new ExpBaseConst(dynamic_cast<AccessFrame*>(al->Get(j))->Offset())) 
                         ), new ExpBaseTemp( FP() ) 
                     )
             );
    }
    // update fp = sp
    st = new StatementSeq( st,
                           new StatementMove( new ExpBaseTemp( FP()), new ExpBaseTemp( SP() ) )
                         );
    // call
    st = new StatementSeq(st,
            new StatementExp(new ExpBaseCall( new ExpBaseName(f->GetLabel()), explist )));
    
    
    //free frame
    st = new StatementSeq(st,
            new StatementMove( 
                    new ExpBaseTemp( FP() ),
                     new ExpBaseBinop( 
                            BinaryOp::kBinaryOp_Add, 
                            new ExpBaseTemp( FP() ), 
                            new ExpBaseConst( f->GetLevel()->Frame()->Size() )
                     )
            )
    );
                             
    return new TreeGenResult( f->Type(), st);
}
TreeGenResult* TreeGenerator::TreeGenExpOp(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    Oper* op = dynamic_cast<OpExp*>(exp)->GetOper();
    TreeGenResult* left,*right;
    ExpBase* ret;
    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Op,"op exp expected");
    
    left = TreeGenExp(venv,tenv,level,dynamic_cast<OpExp*>(exp)->GetLeft(),done_label);
    right = TreeGenExp(venv,tenv,level,dynamic_cast<OpExp*>(exp)->GetRight(),done_label);

    /* compare */
    if(op->Kind()==Oper::kOper_Lt||
       op->Kind()==Oper::kOper_Le||
       op->Kind()==Oper::kOper_Gt||
       op->Kind()==Oper::kOper_Ge||
       op->Kind()==Oper::kOper_Eq||
       op->Kind()==Oper::kOper_Neq){
        StatementBase* statement;
        PatchList * ts = new PatchList;
        PatchList * fs = new PatchList;
        statement = new StatementCjump( RelationOp::ToRelationOp(op->Kind()), UnEx(left), UnEx(right), 0/* in place */, 0/* in place */);
        ts->Insert( dynamic_cast<StatementCjump*>(statement)->GetATrueLabel(), PatchList::kPatchList_Rear );
        fs->Insert( dynamic_cast<StatementCjump*>(statement)->GetAFalseLabel(), PatchList::kPatchList_Rear );
        
        delete left;
        delete right;
        
        Symbol t("int");
        return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)),statement, ts, fs );
    }
    if(op->Kind()==Oper::kOper_Add||
       op->Kind()==Oper::kOper_Sub||
       op->Kind()==Oper::kOper_Mul||
       op->Kind()==Oper::kOper_Div){

        ret = new ExpBaseBinop(BinaryOp::ToBinaryOp(op->Kind()),UnEx(left),UnEx(right));
        
        delete left;
        delete right;
        
        Symbol t("int");
        return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)),ret );
    }
     
    //others
    ret = new ExpBaseBinop(BinaryOp::ToBinaryOp(op->Kind()),UnEx(left),UnEx(right));
    
    delete left;
    delete right;
    
    Symbol t("int");
    return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)),ret );
}
s32 TreeGenerator::GetMemberOffset(TypeBase* ty,Symbol* member)
{
    TypeFieldNode* head;
    TIGER_ASSERT(ty->Kind()==TypeBase::kType_Record,"record type needed");
    head = dynamic_cast<TypeRecord*>(ty)->GetList()->GetHead();
    if(head==0){
        m_logger.D("empty record type");
        return 0;
    }
    s32 i = 0;
    while(head){
        if(strcmp(head->m_field->Name()->Name(),member->Name())==0)
            return i;
        i+=head->m_field->Type()->Size();
        head = head->next;
    }
    return i;
}
TreeGenResult* TreeGenerator::TreeGenExpRecord(SymTab* venv,SymTab* tenv,Level* level,Exp* exp,Label* done_label)
{
    EnvEntryVar* p;
    EFieldNode* head;
    TypeFieldNode* n;
    TreeGenResult* a;
    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Record,"record exp expected");
    
    p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<RecordExp*>(exp)->Name())));
    
    TIGER_ASSERT(dynamic_cast<TypeName*>(p->Type())->Type()->Kind()==TypeBase::kType_Record,"record type needed");
    
    /* id{} */
    head = dynamic_cast<RecordExp*>(exp)->GetList()->GetHead();
    if(head==0){
        m_logger.D("record exp is {}");
        Symbol t("int");
        return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)),new ExpBaseConst(0));
    }
    /*
    allocate record memory in heap
    type a={x:int,b:string}
    var a=a{x=0,b="hello"}
    */
    EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbolFromString("my_malloc")));
    TIGER_ASSERT(f!=0,"function mymalloc not found");
    ExpBaseList* explist;
    explist = new ExpBaseList;
    ExpBase* ret;
    StatementBase* statement=0;
    
    ret = new ExpBaseTemp(TempLabel::NewTemp());
    
    m_logger.D( "record type size:%d", dynamic_cast<TypeName*>(p->Type())->Size() );
    
    explist->Insert( new ExpBaseConst( dynamic_cast<TypeName*>(p->Type())->Size() ), ExpBaseList::kExpBaseList_Rear);
    
    //allocate memory
    statement = new StatementMove( ret,
                       new ExpBaseCall( new ExpBaseName(f->GetLabel()), explist ) );
    
    //init members
    s32 offset=0;
    while(head){

        a = TreeGenExp(venv,tenv,level,head->m_efield->GetExp(),done_label);
        offset = GetMemberOffset(dynamic_cast<TypeName*>(p->Type())->Type(),head->m_efield->Name());//
        statement = new StatementSeq(
                        statement,
                        new StatementMove(
                            new ExpBaseMem( new ExpBaseBinop(BinaryOp::kBinaryOp_Add,ret->Clone(),new ExpBaseConst(offset)) ),
                            UnEx(a) )
                    );
       
        delete a;
        
        head = head->next;
    }
    //return address of allocated memory
    return new TreeGenResult( p->Type(), new ExpBaseEseq( statement,ret->Clone() ) );
}
TreeGenResult* TreeGenerator::TreeGenExpArray(SymTab* venv,SymTab* tenv,Level* level,Exp* exp,Label* done_label)
{
    Exp* size_exp;
    Exp* init_exp;
    
    TreeGenResult* size_ty;
    TreeGenResult* init_ty;
    
    EnvEntryVar* p;
    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Array,"array exp expected");
    
    /*
    type a=array of int
    var x=a[10] of 1
    */
    p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayExp*>(exp)->Name())));
    TIGER_ASSERT(p->Type()->Kind()==TypeBase::kType_Name,"type %s not found",dynamic_cast<ArrayExp*>(exp)->Name()->Name());
    
    m_logger.D( "array type size:%d", dynamic_cast<TypeName*>(p->Type())->Size() );
    
    size_ty = TreeGenExp(venv,tenv,level,dynamic_cast<ArrayExp*>(exp)->GetSize(),done_label);
    init_ty = TreeGenExp(venv,tenv,level,dynamic_cast<ArrayExp*>(exp)->GetInit(),done_label);
    
    TIGER_ASSERT(size_ty!=0,"array size type is null");
    TIGER_ASSERT(init_ty!=0,"array init type is null");
    
    
    
    // allocate array space
    EnvEntryFun* f = dynamic_cast<EnvEntryFun*>(venv->Lookup(venv->MakeSymbolFromString("my_malloc")));
    TIGER_ASSERT(f!=0,"function mymalloc not found");
    ExpBaseList* explist;
    explist = new ExpBaseList;
    ExpBase* ret;
    ExpBase* v;
    Label* loop_start,*loop_end;
    StatementBase* statement=0;
    
    ret = new ExpBaseTemp(TempLabel::NewTemp());
    v = new ExpBaseTemp(TempLabel::NewTemp());
    
    loop_start = TempLabel::NewLabel();
    loop_end = TempLabel::NewLabel();
    
    explist->Insert( 
          new ExpBaseBinop( BinaryOp::kBinaryOp_Mul,
              new ExpBaseConst(dynamic_cast<TypeName*>(p->Type())->Size()),
              UnEx(size_ty)),
          ExpBaseList::kExpBaseList_Rear);
    
    //allocate memory
    statement = new StatementMove( ret,
                       new ExpBaseCall( new ExpBaseName(f->GetLabel()), explist ) );
    
    /*
    mov v=0
    loop:
    mov arr[v],init
    add v,v,1
    cjump <,v,size,loop,loop_end
    loop_end:
    */
    
    //init members
    statement = new StatementSeq(statement,
                    new StatementMove( v,new ExpBaseConst(0))
                    );
    statement = new StatementSeq(statement,
                    new StatementLabel( loop_start )
                    );    
    statement = new StatementSeq(statement,
                        new StatementMove( new ExpBaseMem( 
                                               new ExpBaseBinop( BinaryOp::kBinaryOp_Add,ret->Clone(),v->Clone() 
                                               ) 
                                           ),
                                           UnEx(init_ty)
                        )
                    );
    statement = new StatementSeq(statement,
                    new StatementMove( v->Clone(), new ExpBaseBinop(BinaryOp::kBinaryOp_Add,v->Clone(),new ExpBaseConst(1)) )
                    ); 
    statement = new StatementSeq(statement,
                    new StatementCjump(RelationOp::kRelationOp_Lt,v->Clone(),UnEx(size_ty),loop_start,loop_end)
                    );
    statement = new StatementSeq(statement,
                    new StatementLabel( loop_end )
                    );                     
    
    
    delete size_ty;
    delete init_ty;

    return new TreeGenResult( p->Type(), ret->Clone() );
}
TreeGenResult* TreeGenerator::TreeGenExpSeq(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    TreeGenResult* tmp=0;
    TreeGenResult* result=0;
    StatementBase* statement=0;
    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Seq,"seq exp expected");
    
    ExpNode* p = dynamic_cast<SeqExp*>(exp)->GetList()->GetHead();
    if(p==0){
        m_logger.D("empty seq exp");
        Symbol t("int");
        return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)),new ExpBaseConst(0) );
    }
    // return value ignore for now
    while(p){
        tmp = TreeGenExp(venv,tenv,level,p->m_exp,done_label);
        p = p->next;
        if(p){
            if(statement==0)
                statement = UnNx( tmp );
            else
                statement = new StatementSeq(statement, UnNx( tmp ));
            delete tmp;
        }
    }
    
    if( statement )// at least 2 exps
    {
        result = new TreeGenResult( tmp->m_type, new ExpBaseEseq( statement, UnEx( tmp ) ) );
    }
    else// 1 exp
    {
        result = new TreeGenResult( tmp->m_type, UnEx( tmp ) );
    }
    
    if(tmp)
        delete tmp;

    return result;
}
TreeGenResult* TreeGenerator::TreeGenExpAssign(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    TreeGenResult* a;
    TreeGenResult* b;
    TreeGenResult* result;
    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Assign,"assign exp expected");
    a = TreeGenVar(venv,tenv,level,dynamic_cast<AssignExp*>(exp)->GetVar(),done_label);
    b = TreeGenExp(venv,tenv,level,dynamic_cast<AssignExp*>(exp)->GetExp(),done_label);
    
    TIGER_ASSERT(a!=0,"var type is null");
    TIGER_ASSERT(b!=0,"exp type is null");
    
    Symbol tmp("int");
    result = new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)),
                  new StatementMove( UnEx(a), UnEx(b) ) );
    
    delete a;
    delete b;

    return result;
}
TreeGenResult* TreeGenerator::TreeGenExpIf(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    Exp* if_exp;
    Exp* then_exp;
    Exp* else_exp;
    
    TreeGenResult* a;
    TreeGenResult* b;
    TreeGenResult* c;
    TreeGenResult* tr;
    
    StatementBase* statement;
    
    TIGER_ASSERT(exp->Kind()==Exp::kExp_If,"if exp expected");
    
    if_exp = dynamic_cast<IfExp*>(exp)->GetTest();
    then_exp = dynamic_cast<IfExp*>(exp)->GetThen();
    else_exp = dynamic_cast<IfExp*>(exp)->GetElsee();
    
    TIGER_ASSERT(if_exp!=0,"if exp is null");
    TIGER_ASSERT(then_exp!=0,"then exp is null");
    
    

    Label *t = TempLabel::NewLabel();
    Label *f = TempLabel::NewLabel();
    Label *end = TempLabel::NewLabel();
    
    // if 
    a = TreeGenExp(venv,tenv,level,if_exp,done_label);
    
    if(a->Kind()==TreeGenResult::kTreeGenResult_Cx)
        m_logger.D("a is Cx");
    tr = UnCx( a );
    tr->m_trues->DoPatch(t);
    tr->m_falses->DoPatch(f);
    
    //then
    b = TreeGenExp(venv,tenv,level,then_exp,done_label);

    LabelList* llist = new LabelList;
    llist->Insert(end,LabelList::kLabelList_Rear);
    
    statement = new StatementSeq(
                    new StatementSeq(
                        new StatementSeq(
                            new StatementSeq( tr->m_statement, new StatementLabel(t)
                            ), 
                            UnNx(b)
                        ), 
                        new StatementJump(new ExpBaseName(end),llist)
                    ),
                    new StatementLabel(f)
    );
    
    //else
    if(else_exp){
        c = TreeGenExp(venv,tenv,level,else_exp,done_label);
        statement = new StatementSeq( statement, UnNx(c) );
    }
    
    statement = new StatementSeq( statement, new StatementLabel(end) );
    
    /* for Ex, we need free UnCx()'s memory */
    if(a->Kind()!=TreeGenResult::kTreeGenResult_Cx)
        delete tr;
    
    delete a;
    delete b;
    if(else_exp)
        delete c;

    Symbol tmp("int");
    return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)), statement );
}
TreeGenResult* TreeGenerator::TreeGenExpWhile(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    Exp* test_exp;
    Exp* body_exp;
    
    TreeGenResult* a;
    TreeGenResult* b;
    
    StatementBase* statement=0;
    
    Label* test_label;
    Label* body_label;
    Label* adone_label;
    
    //check
    TIGER_ASSERT(exp->Kind()==Exp::kExp_While,"while exp expected");
    
    test_label = TempLabel::NewLabel();
    body_label = TempLabel::NewLabel();
    adone_label = TempLabel::NewLabel();
    
    statement = new StatementLabel(test_label);
    
    test_exp = dynamic_cast<WhileExp*>(exp)->GetTest();
    body_exp = dynamic_cast<WhileExp*>(exp)->GetExp();
    

    TIGER_ASSERT(test_exp!=0,"while exp is null");
    TIGER_ASSERT(body_exp!=0,"while body is null");
    
    a = TreeGenExp(venv,tenv,level,test_exp,done_label);
    
    statement = new StatementSeq(statement,
                    new StatementCjump(RelationOp::kRelationOp_Eq, UnEx(a), new ExpBaseConst(1), body_label/*true label*/, adone_label/*false label*/)
                );
    
    statement = new StatementSeq(statement,new StatementLabel(body_label));
    // adone_label for "break" statement
    venv->BeginScope(ScopeMaker::kScope_While);
    b = TreeGenExp(venv,tenv,level,body_exp,adone_label);
    venv->EndScope();
    
    statement = new StatementSeq( statement, UnNx(b) );

    LabelList* llist = new LabelList;
    llist->Insert(test_label,LabelList::kLabelList_Rear);

    statement = new StatementSeq(statement,
                    new StatementJump(new ExpBaseName(test_label),llist));
    
    statement = new StatementSeq(statement,new StatementLabel(adone_label));

    
    delete a;
    delete b;
    Symbol tmp("int");
    return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)), statement );
}
LetExp* TreeGenerator::For2Let(ForExp* exp)
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
    //TIGER_ASSERT(exp1->m_exp->Kind()==Exp::kExp_Assign,"m_exp is null!!");
    exp2->m_exp = assign;
    exp1->next = exp2;
    exp2->prev = exp1;
    list = new ExpList(exp1);
    body = new SeqExp(list);
    
    while_exp = new WhileExp(test,body);
    
    let_exp = new LetExp(decs,while_exp);
    
    return let_exp;
}  
TreeGenResult* TreeGenerator::TreeGenExpFor(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    TreeGenResult* a;
    LetExp* let_exp;
    
    //check
    TIGER_ASSERT(exp->Kind()==Exp::kExp_For,"for exp expected");
    
    let_exp = For2Let( dynamic_cast<ForExp*>(exp) );
    
    a = TreeGenExp(venv,tenv,level,let_exp,done_label);
    
    delete let_exp;

    return a;
}
TreeGenResult* TreeGenerator::TreeGenExpLet(SymTab* venv,SymTab*tenv,Level* level,Exp* exp,Label* done_label)
{
    TreeGenResult* ret=0;
    TreeGenResult* result=0;
    
    
    StatementBase* statement = 0;
    
    //check
    TIGER_ASSERT(exp->Kind()==Exp::kExp_Let,"let exp expected");
    
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
            ret = TreeGenDec(venv,tenv,level,p->m_dec,done_label);
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
                statement = UnNx(ret);
            else
                statement = new StatementSeq(statement, UnNx(ret));
            
            delete ret;// the tree is not used already
        }
    }
    
    if(body){
        ret = TreeGenExp(venv,tenv,level,body,done_label);
        if(statement){
            statement = new StatementSeq( statement, UnNx(ret) );
        }else{
            statement = UnNx( ret );
        }
    }
    
    
    tenv->EndScope();
    venv->EndScope();
    
    if(body)
        delete ret;
    Symbol tmp("int");
    
    return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)), statement );
     
}
TreeGenResult* TreeGenerator::TreeGenDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            TreeGenResult* t;
            
            if(dynamic_cast<VarDec*>(dec)->GetExp()->Kind()==Exp::kExp_Array){
                //array
                t = TreeGenExpArray(venv,tenv,level,dynamic_cast<VarDec*>(dec)->GetExp(),done_label);
            }else if(dynamic_cast<VarDec*>(dec)->GetExp()->Kind()==Exp::kExp_Record){
                //record
                t = TreeGenExpRecord(venv,tenv,level,dynamic_cast<VarDec*>(dec)->GetExp(),done_label);
            }else{
                // simple var declaration
                t = TreeGenExp(venv,tenv,level,dynamic_cast<VarDec*>(dec)->GetExp(),done_label);
            }

            AccessFrame* af;
            AccessReg*   ar;
            VarAccess* access;
            StatementBase* result;

            m_logger.D("~~~~~~~~~~~~~ type size: %d",t->Type()->Size());
            //if(t->Type()->Kind()==TypeBase::kType_Array){
                //ExpBaseBinop(Binop::SUB,)
                //mul t1,w,
            //}
            if(dynamic_cast<VarDec*>(dec)->GetSymbol()->GetEscape()==1){// it's a frame var
                af = dynamic_cast<AccessFrame*>( level->Frame()->AllocLocal(1) );
                access = new VarAccess(level,af);
                // array address 
                result = new StatementMove( new ExpBaseMem( new ExpBaseBinop( BinaryOp::kBinaryOp_Add,
                                                       new ExpBaseTemp(FP()), 
                                                       new ExpBaseConst(af->Offset()) ) ), UnEx(t) );
            }else{
                // register var
                ar = dynamic_cast<AccessReg*>( level->Frame()->AllocLocal(0) );
                access = new VarAccess(level,ar);
                // array address 
                result = new StatementMove( new ExpBaseTemp(ar->GetTemp()), UnEx(t) );
            }
            if(dynamic_cast<VarDec*>(dec)->GetType()){
                EnvEntryBase* p;
                p = tenv->Lookup(tenv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetType()));
                /* t->Type() & p check */
                if(!t->m_type->Equal(dynamic_cast<EnvEntryVar*>(p)->Type()))
                {
                    m_logger.W("type not match");
                }else{
                    
                    venv->Enter( venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()), new EnvEntryVar(t->m_type, EnvEntryVar::kEnvEntryVar_For_Value, access) );
                }
            }else{
                venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryVar(t->m_type, EnvEntryVar::kEnvEntryVar_For_Value, access));
            }
            
            delete t;
            
            Symbol tmp("int");
            return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)), result );
        }
        case Dec::kDec_Function:
        {
            TreeGenFunctionDec(venv,tenv,level,dec,done_label);
            Symbol tmp("int");
            return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)), new ExpBaseConst(0) );
        }
        case Dec::kDec_Type:{
            m_logger.D("type check with kDec_Type");
            TreeGenTypeDec(venv,tenv,level,dec);
            Symbol tmp("int");
            return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&tmp)), new ExpBaseConst(0) );
        }
        default:
            break;
    }
}
TypeBase* TreeGenerator::TreeGenTy(SymTab* tenv,Level* level,Ty* ty)
{
    switch(ty->Kind())
    {
        case Ty::kTy_Name:
        {
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<NameTy*>(ty)->Name())));
            TIGER_ASSERT(t!=0,"type %s not found",dynamic_cast<NameTy*>(ty)->Name()->Name());
            return t->Type();
        }
        case Ty::kTy_Record:
        {
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
            EnvEntryVar* p;
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayTy*>(ty)->Name())));
            return new TypeArray(p->Type());
        }
        default:
            break;
    }
    return 0;
}
TypeFieldList* TreeGenerator::MakeFormalsList(SymTab* venv,SymTab* tenv,Level* level,FieldList* params)
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
FrameBase* TreeGenerator::MakeNewFrame(FunDec* fundec)
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

    if(fundec==0)
        return f;
    
    /** for formal args **/
    if(fundec->GetList()==0){
        //empty formals
        head = 0;
    }
    else{
        head = fundec->GetList()->GetHead();
    }
    

    while(head){

        access = f->AllocLocal(1/*true*/);
        access->Retain();//inc refcnt
        al->Insert(access,AccessList::kAccessList_Rear);
        
        bl->Insert(BoolNode::kBool_True,BoolList::kBoolList_Rear);
        
        /* let function formal arg are all escape type,so all store in frame space */
        head->m_field->Name()->SetEscape( 1 );
        
        head = head->next;
    }
    m_logger.D("formals size:%d",al->Size());
    m_logger.D("escapes size:%d",bl->Size());
    
    /**for static linklist **/
    access = f->AllocLocal(1/*true*/);
    access->Retain();//inc refcnt
    al->Insert(access,AccessList::kAccessList_Rear);
    bl->Insert(BoolNode::kBool_True,BoolList::kBoolList_Rear);
    
    
    
    m_logger.D("New Frame End~~~");
    
    /*
    // allocate array in heap using external call malloc or free
    // temp not need here
    // allocate a new temp store dynamical vars such as array and record
    access = f->AllocLocal(0);
    access->Retain();//inc refcnt
    al->Insert(access,AccessList::kAccessList_Rear);
        
    bl->Insert(BoolNode::kBool_False,BoolList::kBoolList_Rear);
    */
    return f;
}
void TreeGenerator::TreeGenFunctionDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,Label* done_label)
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
    
    TreeGenResult *a;

    Level* alevel;
    
    m_logger.D("type check with kDec_Function");
    
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();
    
    /* process all function header decs */
    while(fundec_head){

        /* level and label */
        alevel = new Level(level,MakeNewFrame(fundec_head->m_fundec));
        m_level_manager->NewLevel(alevel);
        if(fundec_head->m_fundec->Type()==0){
            m_logger.D("empty function return type ");
            venv->Enter( venv->MakeSymbol(fundec_head->m_fundec->Name()),
                         new EnvEntryFun( 
                                          MakeFormalsList(venv,tenv,level,fundec_head->m_fundec->GetList()),
                                          0, 
                                          alevel, 
                                          TempLabel::NewNamedLabel(fundec_head->m_fundec->Name()->Name()),
                                          0/*kind*/
                                        )
                       );

        }else{
            venv->Enter( venv->MakeSymbol(fundec_head->m_fundec->Name()),
                         new EnvEntryFun( 
                                          MakeFormalsList(venv,tenv,level,fundec_head->m_fundec->GetList()), 
                                          dynamic_cast<EnvEntryVar*>( 
                                                                      tenv->Lookup( tenv->MakeSymbol(fundec_head->m_fundec->Type()) )
                                                                    )->Type(), 
                                          alevel, 
                                          TempLabel::NewNamedLabel(fundec_head->m_fundec->Name()->Name()),
                                          0/*kind*/
                                        )
                       );
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
            s32 i=0;//last formal arg for static link
            while(head){
                VarAccess* access = 0;
                AccessFrame* af=0;
                AccessReg*   ar=0;
                if(head->m_field->Name()->GetEscape()==1){
                    af = dynamic_cast<AccessFrame*>( alevel->Frame()->GetFormals()->Get(i) );
                    access = new VarAccess(alevel,af);
                    // new ExpBaseName()
                }else{
                    ar = dynamic_cast<AccessReg*>( alevel->Frame()->GetFormals()->Get(i) );
                    m_logger.D("----------------formal arg :%s",ar->GetTemp()->Name());
                    access = new VarAccess(alevel,ar);
                }
                
                venv->Enter( venv->MakeSymbol(head->m_field->Name()),
                             new EnvEntryVar( 
                                              dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_field->Type())))->Type(), 
                                              EnvEntryVar::kEnvEntryVar_For_Value, access 
                                            ) 
                           );
                head = head->next;
                i++;
            }
        }
        // process function body
        a = TreeGenExp(venv,tenv,alevel,fundec_head->m_fundec->GetExp(),done_label);
        //update type info from body if no type in function head
        if(fundec_head->m_fundec->Type()==0){
            m_logger.D("function return type is null");
            // TBD: update type info
            dynamic_cast<EnvEntryFun*>( venv->Lookup( venv->MakeSymbol(fundec_head->m_fundec->Name()) ))->SetType(a->Type());
        }else
        {
            m_logger.D("type kind %d",a->Type()->Kind());
            if(a->Type()->Kind()!=TypeBase::kType_Nil)
            {
                TIGER_ASSERT(a->Type() == dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(fundec_head->m_fundec->Type())))->Type(), "return type mismatch");
            }
        }
        
        // after process end of the function body 
        // restore fp to it's parent 
        // fp =  mem[fp+static link's address
        // return to the parent's control path
        
        m_frag_list->Insert( dynamic_cast<EnvEntryFun*>( venv->Lookup( venv->MakeSymbol(fundec_head->m_fundec->Name()) ))->GetLabel(),
                             new Frag( UnNx(a), alevel->Frame() ) );
        delete a;
        
        venv->EndScope();
        
        fundec_head = fundec_head->next;
    }
}
void TreeGenerator::TreeGenTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec)
{
    NameTyPairNode* head;
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    /* process headers of decs */
    while(head){
        /*
        EnvEntryVar* p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())));
        if(p){
            m_logger.W("Type %s redefined",head->m_nametypair->Name()->Name());
            //TIGER_ASSERT(0,"Type %s redefined",head->m_nametypair->Name()->Name());
        }
        */
        //m_logger.D("New type with %s",head->m_nametypair->Name()->Name());
        tenv->Enter( tenv->MakeSymbol(head->m_nametypair->Name()),
                     new EnvEntryVar( 
                                      new TypeName(tenv->MakeSymbol(head->m_nametypair->Name()),0),
                                      EnvEntryVar::kEnvEntryVar_For_Type, 0
                                    ) 
                   );
        head = head->next;
    }
    /* process bodys of decs*/
    head = dynamic_cast<TypeDec*>(dec)->GetList()->GetHead();
    while(head){
        /*
        gen type infor from absyn
        */
        TypeBase* t = TreeGenTy(tenv,level,head->m_nametypair->Type());
        if(t->Kind()!=TypeBase::kType_Name){
            /*
            type a=int
            When type "a" insert tenv, it's type is dummy TypeName.Now we get real type so refill it here.
            */
            dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())))->Update(t);
        }
        else{
            /*
            type b=int
            type a=b
            */
            EnvEntryVar* p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())));
            p->Update(dynamic_cast<TypeName*>(t));
            if(dynamic_cast<TypeName*>(t)->Type()==dynamic_cast<TypeName*>(p->Type())){
                /*
                type b=a
                type a=b
                */
                TIGER_ASSERT(0,"cycle dependency occur");                        
            }

        }
        head = head->next;
    }
}
/*
 level for escape variable access
 done_label for break "for" or "while"
*/
TreeGenResult* TreeGenerator::TreeGenExp(SymTab* venv,SymTab* tenv,Level* level,Exp* exp,Label* done_label)
{
    switch(exp->Kind())
    {
        case Exp::kExp_Var:
        {
            return TreeGenVar(venv,tenv,level,dynamic_cast<VarExp*>(exp)->GetVar(),done_label);
        }
        case Exp::kExp_Nil:
        {
            // for nil, just return 0 instead
            Symbol t("int");
            return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)), new ExpBaseConst( 0 ) );
        }
        case Exp::kExp_Int:
        {
            Symbol t("int");
            return new TreeGenResult(  tenv->Type(tenv->MakeSymbol(&t)), new ExpBaseConst(dynamic_cast<IntExp*>(exp)->GetInt()) );
        }
        case Exp::kExp_String:
        {
            // for literal string, use a label instead
            Label* l;
            l = m_lit_string_list->FindByString(dynamic_cast<StringExp*>(exp)->GetString());
            if(l==0){
                l = TempLabel::NewLabel();
                m_lit_string_list->Insert(l,dynamic_cast<StringExp*>(exp)->GetString());
            }
            TIGER_ASSERT(l!=0,"label is null!");
            
            Symbol t("string");
            return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)), new ExpBaseName( l ) );
        }
        case Exp::kExp_Call:
        {
            return TreeGenExpCall(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Op:
        {
            return TreeGenExpOp(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Record:
        {
            return TreeGenExpRecord(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Seq:
        {
            return TreeGenExpSeq(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Assign:
        {
            return TreeGenExpAssign(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_If:
        {
            return TreeGenExpIf(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_While:
        {
            return TreeGenExpWhile(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Break:
        {
            if((venv->Scope()!=ScopeMaker::kScope_For) &&
               (venv->Scope()!=ScopeMaker::kScope_While)){
                m_logger.D("break should be in for/while scope.");
            }
            TIGER_ASSERT( ((venv->Scope()==ScopeMaker::kScope_For)||(venv->Scope()==ScopeMaker::kScope_While)),"expected in for or while scope");
            LabelList* llist = new LabelList;
            llist->Insert(done_label,LabelList::kLabelList_Rear);
            
            Symbol t("int");
            return new TreeGenResult( tenv->Type(tenv->MakeSymbol(&t)), new StatementJump(new ExpBaseName(done_label),llist) );
        }
        case Exp::kExp_For:
        {
            return TreeGenExpFor(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Let:
        {
            return TreeGenExpLet(venv,tenv,level,exp,done_label);
        }
        case Exp::kExp_Array:
        {
            return TreeGenExpArray(venv,tenv,level,exp,done_label);
        }
        default:
        {
            m_logger.W("should not reach here %s,%d",__FILE__,__LINE__);
            break;
        }
    }
    m_logger.W("should not reach here %s,%d",__FILE__,__LINE__);
    return 0;
}
/*
only out most code call this
*/
StatementBase* TreeGenerator::ProcessEntryExit(SymTab* venv,SymTab* tenv, Level* level,StatementBase* s)
{
    m_logger.D("frame size is:%d",level->Frame()->Size());
    m_logger.D("Alloc space for out most level");
    s = new StatementSeq(
            new StatementMove(
                new ExpBaseTemp(FP()),
                new ExpBaseBinop(
                    BinaryOp::kBinaryOp_Sub,
                    new ExpBaseTemp(FP()),
                    new ExpBaseConst(level->Frame()->Size())
                )
            ),
            s
       );
    s = new StatementSeq(
           s,
           new StatementMove(
                new ExpBaseTemp(FP()),
                new ExpBaseBinop(
                    BinaryOp::kBinaryOp_Add,
                    new ExpBaseTemp(FP()),
                    new ExpBaseConst(level->Frame()->Size())
                )
           )
           
    );
    return new StatementSeq(s,
      new StatementMove( new ExpBaseTemp(SP()), new ExpBaseTemp(FP()))
    );
}

}//namespace tiger