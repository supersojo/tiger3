#include "tiger_assert.h"
#include "assem.h"


namespace tiger{

Temp* CodeGenerator::_MunchExpBase(InstrList* il,ExpBase *e){
    if(e->Kind()==ExpBase::kExpBase_Mem)
        return _MunchExpBaseMem( il, dynamic_cast<ExpBaseMem*>(e));
    if(e->Kind()==ExpBase::kExpBase_Binop)
        return _MunchExpBaseBinop( il, dynamic_cast<ExpBaseBinop*>(e));
    if(e->Kind()==ExpBase::kExpBase_Const)
        return _MunchExpBaseConst( il, dynamic_cast<ExpBaseConst*>(e));
    if(e->Kind()==ExpBase::kExpBase_Temp)
        return _MunchExpBaseTemp( il, dynamic_cast<ExpBaseTemp*>(e));
    if(e->Kind()==ExpBase::kExpBase_Call)
        return _MunchExpBaseCall( il, dynamic_cast<ExpBaseCall*>(e));
    return 0;
}
Temp* CodeGenerator::_MunchExpBaseCall(InstrList* il,ExpBaseCall *e){
    Temp* r = TempLabel::NewTemp();
    ExpBaseList* el = e->GetList();
    s32 i = 0;
    char buf[1024]={0};
    TempList* dst;
    TempList* src;
    for(i=0;i<el->Size();i++){
        
        sprintf(buf,"push 's0");
        TempList* dst = new TempList;
        TempList* src = new TempList;

        src->Insert(_MunchExpBase(il,el->Get(i)),TempList::kTempList_Rear);
        il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
        
    }
    
    sprintf(buf,"call %s",dynamic_cast<ExpBaseName*>(e->GetExp())->GetLabel()->Name());
    dst = new TempList;
    src = new TempList;

    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    // mov 'd0,'s0
    return TempLabel::NewTemp();
}
Temp* CodeGenerator::_MunchExpBaseMem(InstrList* il, ExpBaseMem *e){
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Temp){
        //mov 'd0,['s0]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"mov 'd0,['s0]");
        TempList* dst = new TempList;
        TempList* src = new TempList;
        dst->Insert(r,TempList::kTempList_Rear);
        src->Insert(_MunchExpBase(il,e->GetExp()),TempList::kTempList_Rear);
        
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
        return r;
    }
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Const){
        //mov 'd0,[i]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"mov 'd0,[%d]",dynamic_cast<ExpBaseConst*>(e->GetExp())->GetValue());
        TempList* dst = new TempList;
        TempList* src = new TempList;
        dst->Insert(r,TempList::kTempList_Rear);        
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
        return r;
    }
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Binop){
        //mov 'd0,[i]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"mov 'd0,'[s0]",dynamic_cast<ExpBaseConst*>(e->GetExp())->GetValue());
        TempList* dst = new TempList;
        TempList* src = new TempList;
        dst->Insert(r,TempList::kTempList_Rear); 
        src->Insert(_MunchExpBase(il,e->GetExp()),TempList::kTempList_Rear);        
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
        return r;
    }
    return 0;
}
Temp* CodeGenerator::_MunchExpBaseBinop(InstrList* il, ExpBaseBinop *e){
    // add d0,s0,s1
    Temp* r = TempLabel::NewTemp();
    
    char buf[1024]={0};
    sprintf(buf,"add 'd0,'s0,'s1");
    TempList* dst = new TempList;
    TempList* src = new TempList;
    dst->Insert(r,TempList::kTempList_Rear);  
    src->Insert(_MunchExpBase(il,e->Left()),TempList::kTempList_Rear);    
    src->Insert(_MunchExpBase(il,e->Right()),TempList::kTempList_Rear);        
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);

    return r;
}
Temp* CodeGenerator::_MunchExpBaseConst(InstrList* il, ExpBaseConst *e){
    Temp* r = TempLabel::NewTemp();
    char buf[1024]={0};
    sprintf(buf,"mov 'd0,'%d",e->GetValue());
    TempList* dst = new TempList;
    TempList* src = new TempList;
    dst->Insert(r,TempList::kTempList_Rear);
    il->Insert( new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
    return r;
}
Temp* CodeGenerator::_MunchExpBaseTemp(InstrList* il, ExpBaseTemp *e){
    return e->GetTemp();
}
/*
StatementSeq  (X)
StatementMove
StatementLabel
StatementExp
StatementJump
StatementCJump

*/
void CodeGenerator::_MunchStatement(InstrList* il,StatementBase *s){
    if( s->Kind()==StatementBase::kStatement_Move )
    {
        _MunchStatementMove(il,dynamic_cast<StatementMove*>(s));
    }
    if( s->Kind()==StatementBase::kStatement_Jump )
    {
        _MunchStatementJump(il,dynamic_cast<StatementJump*>(s));
    }
    if( s->Kind()==StatementBase::kStatement_Cjump )
    {
        _MunchStatementCjump(il,dynamic_cast<StatementCjump*>(s));
    }
}
void CodeGenerator::_MunchStatementMove(InstrList* il,StatementMove *s){
    if( s->Left()->Kind()==ExpBase::kExpBase_Mem )
    {
        char buf[1024]={0};
        ExpBase* e1,*e2;
        e1 = s->Left();
        e2 = s->Right();//dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(s->Left())->GetExp())->Right()
        //dynamic_cast<ExpBaseConst*>( dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(s->Left())->GetExp())->Right() )->GetValue()
        sprintf(buf,"mov ['d0],'s0");
        TempList* dst = new TempList;
        TempList* src = new TempList;
        
        dst->Insert(_MunchExpBase(il,e1),TempList::kTempList_Rear);
        src->Insert(_MunchExpBase(il,e2),TempList::kTempList_Rear);
        il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);

    }
    if( s->Left()->Kind()==ExpBase::kExpBase_Temp )
    {
        char buf[1024]={0};
        ExpBase* e1,*e2;
        
        //dynamic_cast<ExpBaseConst*>(dynamic_cast<ExpBaseMem*>(s->Left())->GetExp())->GetValue();
        sprintf(buf,"mov 'd0,'s0");
        e1 = s->Left();
        e2 = s->Right();
        TempList* dst = new TempList;
        TempList* src = new TempList;
        
        dst->Insert(_MunchExpBase(il,e1),TempList::kTempList_Rear);
        src->Insert(_MunchExpBase(il,e2),TempList::kTempList_Rear);
        il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
    }
}
void CodeGenerator::_MunchStatementJump(InstrList* il,StatementJump *s){
}
void CodeGenerator::_MunchStatementCjump(InstrList* il,StatementCjump *s){
}
void CodeGenerator::Munch(InstrList* il,FrameBase* f,StatementBaseList* l){
    if(l->Size()==0)
        return;
    s32 i = 0;
    StatementBase* s;
    for(i=0;i<l->Size();i++){
        s = l->Get(i);
        _MunchStatement(il,s);
    }
}
InstrList* CodeGenerator::CodeGen(FrameBase* f,StatementBaseList* l){
    InstrList* il = new InstrList;
    Munch(il,f,l);
    return il;
}

}//namespace tiger
