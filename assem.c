#include "tiger_assert.h"
#include "assem.h"
#include "regalloc.h"

namespace tiger{

void InstrOper::Output(TempMapList* map,char* o){
    if(m_dst->Size()==0 && m_src->Size()==0){
        sprintf(o,"%s",m_str);
    }
    if(m_dst->Size()==0 && m_src->Size()!=0){
        if(m_src->Size()==1)
            sprintf(o,m_str, map->Look( m_src->Get(0) ) );
        if(m_src->Size()==2)
                sprintf(o,m_str,map->Look( m_src->Get(0) ),map->Look( m_src->Get(1) ));
    }
    if(m_dst->Size()!=0 && m_src->Size()==0){
        sprintf(o,m_str, map->Look( m_dst->Get(0) ));
    }
    if(m_dst->Size()!=0 && m_src->Size()!=0){
        sprintf(o,m_str,map->Look( m_src->Get(0) ),map->Look( m_dst->Get(0) ));
    }
}
void InstrMove::Output(TempMapList* map,char* o){

    if(m_dst->Size()==0 && m_src->Size()==0){
        sprintf(o,"%s",m_str);
    }
    if(m_dst->Size()==0 && m_src->Size()!=0){
        sprintf(o,m_str, map->Look( m_src->Get(0) ) );
    }
    if(m_dst->Size()!=0 && m_src->Size()==0){
        sprintf(o,m_str, map->Look( m_dst->Get(0) ));
    }
    if(m_dst->Size()!=0 && m_src->Size()!=0){
        // non meaning move instruction
        if(strcmp(map->Look( m_src->Get(0) ),map->Look( m_dst->Get(0) ))!=0)
            sprintf(o,(const char*)m_str,map->Look( m_src->Get(0) ),map->Look( m_dst->Get(0) ));
    }
}
void InstrList::Output(TempMapList* map,char* o)
{
    InstrNode* p = m_head;
    s32 i_offset = 0;
    char s[1024]={0};
    while(p){
        memset(s,0,1024);
        if(p->m_instr)
            p->m_instr->Output(map,s);
        if(*s)
            i_offset += sprintf(i_offset+o,"%s\n",s);
        p = p->next;
    }
}
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
TempList* CodeGenerator::_MunchArgs(ExpBaseList* el)
{
    TempList* tl = new TempList;
    InstrList il;
    for(s32 i=0;i<el->Size();i++){
        if(el->Get(i)->Kind()!=ExpBase::kExpBase_Const)
            tl->Insert( _MunchExpBase(&il,el->Get(i)), TempList::kTempList_Rear );
    }
    return tl;
}
void CodeGenerator::_SaveRegs(InstrList* il)
{
    char buf[1024]={0};
    
    TempList* dst = new TempList;
    TempList* src = new TempList;
    
    sprintf(buf,"push %RAX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"push %RBX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"push %RCX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"push %RDX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"push %RDI");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"push %RSI");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
}
void CodeGenerator::_RestoreRegs(InstrList* il)
{
    char buf[1024]={0};
    
    TempList* dst = new TempList;
    TempList* src = new TempList;
    
    
    sprintf(buf,"pop %RSI");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"pop %RDI");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"pop %RDX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"pop %RCX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"pop %RBX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    dst = new TempList;
    src = new TempList;
    sprintf(buf,"pop %RAX");
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
}
Temp* CodeGenerator::_MunchExpBaseCall(InstrList* il,ExpBaseCall *e){
    Temp* r = TempLabel::NewTemp();
    ExpBaseList* el = e->GetList();
    s32 i = 0;
    char buf[1024]={0};
    
    TempList* dst = new TempList;
    TempList* src = _MunchArgs(el);
    
    _SaveRegs(il);
    
    sprintf(buf,"call %s",dynamic_cast<ExpBaseName*>(e->GetExp())->GetLabel()->Name());

    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    _RestoreRegs(il);
    
    // return value's temp
    return TempLabel::NewNamedTemp("RV");
}
Temp* CodeGenerator::_MunchExpBaseMem(InstrList* il, ExpBaseMem *e){
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Temp){
        //[t]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"%s","movq [%s],%s");
        TempList* dst = new TempList;
        TempList* src = new TempList;
        dst->Insert(r,TempList::kTempList_Rear);

        sprintf(buf,"%s","movq [%s],%s");
        src->Insert(_MunchExpBase(il,e->GetExp()),TempList::kTempList_Rear);
        
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
        return r;
    }
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Const){
        //mov 'd0,[i]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"movq [$%d],%s",dynamic_cast<ExpBaseConst*>(e->GetExp())->GetValue(),"%s");
        TempList* dst = new TempList;
        TempList* src = new TempList;
        dst->Insert(r,TempList::kTempList_Rear);        
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
        return r;
    }
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Binop){
        //[a+b]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"%s","movq (%s),%s");
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
    TempList* dst = new TempList;
    TempList* src = new TempList;
    
    char buf[1024]={0};
    if(e->Left()->Kind()==ExpBase::kExpBase_Const){
        sprintf(buf,"movq $%d,%s",dynamic_cast<ExpBaseConst*>(e->Left())->GetValue(),"%s");
        dst->Insert(r,TempList::kTempList_Rear); 
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
    }else{
        sprintf(buf,"%s","movq %s,%s");
        src->Insert(_MunchExpBase(il,e->Left()),TempList::kTempList_Rear);  
        dst->Insert(r,TempList::kTempList_Rear);  
        il->Insert(new InstrMove(buf,dst,src), InstrList::kInstrList_Rear);
    }
    
    dst = new TempList;
    src = new TempList;
    
    if(e->Op()==BinaryOp::kBinaryOp_Add){
        if(e->Right()->Kind()==ExpBase::kExpBase_Const){
            sprintf(buf,"addq $%d,%s",dynamic_cast<ExpBaseConst*>(e->Right())->GetValue(),"%s");
        }else{
            sprintf(buf,"%s","addq %s,%s");
            src->Insert(_MunchExpBase(il,e->Right()),TempList::kTempList_Rear); 
        }
    }else if(e->Op()==BinaryOp::kBinaryOp_Sub){
        if(e->Right()->Kind()==ExpBase::kExpBase_Const){
            sprintf(buf,"subq $%d,%s",dynamic_cast<ExpBaseConst*>(e->Right())->GetValue(),"%s");
        }else{
            sprintf(buf,"%s","sub %s,%s");
            src->Insert(_MunchExpBase(il,e->Right()),TempList::kTempList_Rear); 
        }
    }else{
        ;
    }
    
    dst->Insert(r,TempList::kTempList_Rear);   
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    return r;
}
Temp* CodeGenerator::_MunchExpBaseConst(InstrList* il, ExpBaseConst *e){
    Temp* r = TempLabel::NewTemp();
    char buf[1024]={0};
    sprintf(buf,"movq $%d,%s",e->GetValue(),"%s");
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
StatementLabel (*)
StatementExp
StatementJump
StatementCJump

*/
void CodeGenerator::_MunchStatement(InstrList* il,StatementBase *s){
    if( s->Kind()==StatementBase::kStatement_Move )
    {
        // most important for assem code gen
        _MunchStatementMove(il,dynamic_cast<StatementMove*>(s));
    }
    if( s->Kind()==StatementBase::kStatement_Jump )
    {
        // simple 
        _MunchStatementJump(il,dynamic_cast<StatementJump*>(s));
    }
    if( s->Kind()==StatementBase::kStatement_Cjump )
    {
        //simple 
        _MunchStatementCjump(il,dynamic_cast<StatementCjump*>(s));
    }
    if( s->Kind()==StatementBase::kStatement_Exp )
    {
        TempList* dst = new TempList;
        TempList* src = new TempList;
        //only for call ??? 
        /*
        dynamic_cast<StatementExp*>(s)->GetExp() should be call exp
        */
        if(dynamic_cast<StatementExp*>(s)->GetExp()->Kind()!=ExpBase::kExpBase_Call)
        {
            m_logger.W("exp statement should be call");
            //TIGER_ASSERT(0,"exp statement should be call");
        }
        //call exp
        _MunchStatementExp(il,dynamic_cast<StatementExp*>(s));
        /*
        char buf[1024]={0};
        sprintf(buf,"%s","movq %s,%s");
        src->Insert(_MunchExpBase(il,dynamic_cast<StatementExp*>(s)->GetExp()),TempList::kTempList_Rear);
        dst->Insert(TempLabel::NewNamedTemp("RV"),TempList::kTempList_Rear);
        il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
        */
    }
    if( s->Kind()==StatementBase::kStatement_Label )
    {// simple branch
        char buf[1024]={0};
        sprintf(buf,"%s:",dynamic_cast<StatementLabel*>(s)->GetLabel()->Name());
        
        il->Insert( new InstrLabel(buf,dynamic_cast<StatementLabel*>(s)->GetLabel()), InstrList::kInstrList_Rear);
    }
}
void CodeGenerator::_MunchStatementExp(InstrList* il,StatementExp *s){
    //discard return value
    _MunchExpBase(il,s->GetExp());
}
void CodeGenerator::_MunchStatementMove(InstrList* il,StatementMove *s){
    /*
    For move statement
    it's left can only be Mem or temp
    */
    TempList* dst = new TempList;
    TempList* src = new TempList;
    if( s->Left()->Kind()==ExpBase::kExpBase_Mem )
    {
        char buf[1024]={0};
        ExpBase* e1,*e2;
        e1 = s->Left();
        e2 = s->Right();

        /*
        mem(a+2)
        mem(2+a)
        */
        if(dynamic_cast<ExpBaseMem*>(e1)->GetExp()->Kind()==ExpBase::kExpBase_Binop)
        {
            dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left();
            dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right();
            if((dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left()->Kind()==ExpBase::kExpBase_Const)||
               (dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right()->Kind()!=ExpBase::kExpBase_Const))
            {
                if(e2->Kind()==ExpBase::kExpBase_Const){
                    sprintf(buf,"movq $%d,%d(%s)",dynamic_cast<ExpBaseConst*>(e2)->GetValue(),dynamic_cast<ExpBaseConst*>(dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left())->GetValue(),"%s");
                    dst->Insert(_MunchExpBase(il,dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right()),TempList::kTempList_Rear);
                    il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
                    return;
                }else{
                    sprintf(buf,"movq %s,%d(%s)","%",dynamic_cast<ExpBaseConst*>(dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left())->GetValue(),"%s");
                    dst->Insert(_MunchExpBase(il,dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right()),TempList::kTempList_Rear);
                    src->Insert(_MunchExpBase(il,e2),TempList::kTempList_Rear);
                    il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
                    return;
                } 
            }
            if((dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left()->Kind()!=ExpBase::kExpBase_Const)||
               (dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right()->Kind()==ExpBase::kExpBase_Const))
            {
                if(e2->Kind()==ExpBase::kExpBase_Const){
                    sprintf(buf,"movq $%d,%d(%s)",dynamic_cast<ExpBaseConst*>(e2)->GetValue(),dynamic_cast<ExpBaseConst*>(dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right())->GetValue(),"%s");
                    dst->Insert(_MunchExpBase(il,dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left()),TempList::kTempList_Rear);
                    il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
                    return;
                }else{
                    sprintf(buf,"movq %s,%d(%s)","%s",dynamic_cast<ExpBaseConst*>(dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Right())->GetValue(),"%s");
                    dst->Insert(_MunchExpBase(il,dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(e1)->GetExp())->Left()),TempList::kTempList_Rear);
                    src->Insert(_MunchExpBase(il,e2),TempList::kTempList_Rear);
                    il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
                    return;
                }
                
            }
            
        }
        
        /*
        mem(1+2)
        mem(a+b)
        */
        
        sprintf(buf,"%s","movq %s,(%s)");

        dst->Insert(_MunchExpBase(il,e1),TempList::kTempList_Rear);
        src->Insert(_MunchExpBase(il,e2),TempList::kTempList_Rear);
        il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);

    }
    if( s->Left()->Kind()==ExpBase::kExpBase_Temp )
    {
        char buf[1024]={0};
        ExpBase* e1,*e2;
        
        //TempList* dst = new TempList;
        //TempList* src = new TempList;
        
        e1 = s->Left();
        e2 = s->Right();
        //dynamic_cast<ExpBaseConst*>(dynamic_cast<ExpBaseMem*>(s->Left())->GetExp())->GetValue();
        if(e2->Kind()==ExpBase::kExpBase_Const){
            sprintf(buf,"movq $%d,%s",dynamic_cast<ExpBaseConst*>(e2)->GetValue(),"%s");
            dst->Insert(_MunchExpBase(il,e1),TempList::kTempList_Rear);
            il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
            return;
        }
        else {
            sprintf(buf,"%s","movq %s,%s");
            dst->Insert(_MunchExpBase(il,e1),TempList::kTempList_Rear);
            src->Insert(_MunchExpBase(il,e2),TempList::kTempList_Rear);
            il->Insert( new InstrMove( buf, dst, src), InstrList::kInstrList_Rear);
            return;
        }
    }
    TIGER_ASSERT( (s->Left()->Kind()==ExpBase::kExpBase_Mem)||(s->Left()->Kind()==ExpBase::kExpBase_Temp),"move's dst must be mem or temp exp");
}
void CodeGenerator::_MunchStatementJump(InstrList* il,StatementJump *s){
    char buf[1024]={0};
    TempList* dst = new TempList;
    TempList* src = new TempList;
    sprintf(buf,"jmp %s",s->GetList()->Get(0)->Name());
    
    il->Insert( new InstrOper( buf, dst, src, s->GetList()->Clone() ), InstrList::kInstrList_Rear);
}
void CodeGenerator::_MunchStatementCjump(InstrList* il,StatementCjump *s){
    char buf[1024]={0};
    
    //sprintf(buf,"%s","cmp %s,%s");
    TempList* dst = new TempList;
    TempList* src = new TempList;
    
    if(s->Left()->Kind()==ExpBase::kExpBase_Const && s->Right()->Kind()!=ExpBase::kExpBase_Const)
    {
        sprintf(buf,"cmp $%d,%s",dynamic_cast<ExpBaseConst*>(s->Left())->GetValue(),"%s");
        src->Insert(_MunchExpBase(il,s->Right()),TempList::kTempList_Rear);
        il->Insert( new InstrOper( buf, dst, src, 0), InstrList::kInstrList_Rear);
    }
    else if(s->Left()->Kind()!=ExpBase::kExpBase_Const && s->Right()->Kind()==ExpBase::kExpBase_Const)
    {
        sprintf(buf,"cmp $%d,%s",dynamic_cast<ExpBaseConst*>(s->Right())->GetValue(),"%s");
        src->Insert(_MunchExpBase(il,s->Left()),TempList::kTempList_Rear);
        il->Insert( new InstrOper( buf, dst, src, 0), InstrList::kInstrList_Rear);
    }else if(s->Left()->Kind()!=ExpBase::kExpBase_Const && s->Right()->Kind()!=ExpBase::kExpBase_Const){
        memset(buf,0,1024);
        sprintf(buf,"%s","cmp %s,%s");
        src->Insert(_MunchExpBase(il,s->Right()),TempList::kTempList_Rear);
        src->Insert(_MunchExpBase(il,s->Left()),TempList::kTempList_Rear);
        il->Insert( new InstrOper( buf, dst, src, 0), InstrList::kInstrList_Rear);
    }else{
        sprintf(buf,"cmp $%d,$%d",dynamic_cast<ExpBaseConst*>(s->Left())->GetValue(),dynamic_cast<ExpBaseConst*>(s->Right())->GetValue());
        il->Insert( new InstrOper( buf, dst, src, 0), InstrList::kInstrList_Rear);
    }
    if(s->Op()==RelationOp::kRelationOp_Le)
        sprintf(buf,"jle %s",s->GetTrueLabel()->Name());
    if(s->Op()==RelationOp::kRelationOp_Eq)
        sprintf(buf,"je %s",s->GetTrueLabel()->Name());
    if(s->Op()==RelationOp::kRelationOp_Gt)
        sprintf(buf,"jg %s",s->GetTrueLabel()->Name());
    dst = new TempList; 
    src = new TempList;
    LabelList* ll = new LabelList;
    ll->Insert(s->GetTrueLabel(),LabelList::kLabelList_Rear);
    il->Insert( new InstrOper( buf, dst, src, ll), InstrList::kInstrList_Rear);
    
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
