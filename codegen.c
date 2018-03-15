#include "tiger_assert.h"
#include "assem.h"
#include "regalloc.h"

namespace tiger{

void InstrOper::Output(ColorList* cl,char* o){
    // parse m_str
    static char* regs[]={
        "",
        "eax",
        "ebx",
        "ecx",
        "edx"
    };
    if(m_dst->Size()==0 && m_src->Size()==0){
        sprintf(o,"%s",m_str);
    }
    if(m_dst->Size()==0 && m_src->Size()!=0){
        sprintf(o,m_str, regs[ cl->GetByTemp( m_src->Get(0) )->m_color ] );
    }
    if(m_dst->Size()!=0 && m_src->Size()==0){
        sprintf(o,m_str, regs[ cl->GetByTemp( m_dst->Get(0) )->m_color ]);
    }
    if(m_dst->Size()!=0 && m_src->Size()!=0){
        sprintf(o,(const char*)m_str,regs[ cl->GetByTemp( m_src->Get(0) )->m_color ],regs[ cl->GetByTemp( m_dst->Get(0) )->m_color ]);
    }
}
void InstrMove::Output(ColorList* cl,char* o){
    // parse m_str
    static char* regs[]={
        "",
        "eax",
        "ebx",
        "ecx",
        "edx"
    };
    if(m_dst->Size()==0 && m_src->Size()==0){
        sprintf(o,"%s",m_str);
    }
    if(m_dst->Size()==0 && m_src->Size()!=0){
        sprintf(o,m_str, regs[ cl->GetByTemp( m_src->Get(0) )->m_color ] );
    }
    if(m_dst->Size()!=0 && m_src->Size()==0){
        sprintf(o,m_str, regs[ cl->GetByTemp( m_dst->Get(0) )->m_color ]);
    }
    if(m_dst->Size()!=0 && m_src->Size()!=0){
        // non meaning move instruction
        if(cl->GetByTemp( m_src->Get(0) )->m_color == cl->GetByTemp( m_dst->Get(0) )->m_color )
            return;
        sprintf(o,(const char*)m_str,regs[ cl->GetByTemp( m_src->Get(0) )->m_color ],regs[ cl->GetByTemp( m_dst->Get(0) )->m_color ]);
    }
}
void InstrList::Output(ColorList* cl,char* o)
{
    InstrNode* p = m_head;
    s32 i_offset = 0;
    char s[1024]={0};
    while(p){
        if(p->m_instr)
            p->m_instr->Output(cl,s);
        printf("=%s=\n",s);
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
Temp* CodeGenerator::_MunchExpBaseCall(InstrList* il,ExpBaseCall *e){
    Temp* r = TempLabel::NewTemp();
    ExpBaseList* el = e->GetList();
    s32 i = 0;
    char buf[1024]={0};
    TempList* dst;
    TempList* src;
    for(i=0;i<el->Size();i++){
        
        sprintf(buf,"push %%%%s");
        TempList* dst = new TempList;
        TempList* src = new TempList;

        src->Insert(_MunchExpBase(il,el->Get(i)),TempList::kTempList_Rear);
        il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
        
    }
    
    sprintf(buf,"call %s",dynamic_cast<ExpBaseName*>(e->GetExp())->GetLabel()->Name());
    dst = new TempList;
    src = new TempList;

    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    // return value's temp
    return TempLabel::NewTemp();
}
Temp* CodeGenerator::_MunchExpBaseMem(InstrList* il, ExpBaseMem *e){
    if(e->GetExp()->Kind()==ExpBase::kExpBase_Temp){
        //mov 'd0,['s0]
        Temp* r = TempLabel::NewTemp();
        char buf[1024]={0};
        sprintf(buf,"mov [%%%%%%s],%%%%%%s");
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
        sprintf(buf,"mov [$%d],%%%%%%s",dynamic_cast<ExpBaseConst*>(e->GetExp())->GetValue());
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
        sprintf(buf,"mov [%%%%%%s],%%%%%%s");
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
    sprintf(buf,"mov $0,%%%%%%s");
    TempList* dst = new TempList;
    TempList* src = new TempList;
    dst->Insert(r,TempList::kTempList_Rear);  
    //src->Insert(_MunchExpBase(il,e->Left()),TempList::kTempList_Rear);    
    //src->Insert(_MunchExpBase(il,e->Right()),TempList::kTempList_Rear);        
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    sprintf(buf,"add %%%%%%s,%%%%%%s");
    dst = new TempList;
    src = new TempList;
    dst->Insert(r,TempList::kTempList_Rear);  
    src->Insert(_MunchExpBase(il,e->Left()),TempList::kTempList_Rear);    
    //src->Insert(_MunchExpBase(il,e->Right()),TempList::kTempList_Rear);        
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    
    sprintf(buf,"add %%%%%%s,%%%%%%s");
    dst = new TempList;
    src = new TempList;
    dst->Insert(r,TempList::kTempList_Rear);  
    //src->Insert(_MunchExpBase(il,e->Left()),TempList::kTempList_Rear);    
    src->Insert(_MunchExpBase(il,e->Right()),TempList::kTempList_Rear);        
    il->Insert(new InstrOper(buf,dst,src,0), InstrList::kInstrList_Rear);
    

    return r;
}
Temp* CodeGenerator::_MunchExpBaseConst(InstrList* il, ExpBaseConst *e){
    Temp* r = TempLabel::NewTemp();
    char buf[1024]={0};
    sprintf(buf,"mov $%d,%%%%%%s",e->GetValue());
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
    if( s->Kind()==StatementBase::kStatement_Exp )
    {
        _MunchStatementExp(il,dynamic_cast<StatementExp*>(s));
    }
    if( s->Kind()==StatementBase::kStatement_Label )
    {
        char buf[1024]={0};
        sprintf(buf,"%s:",dynamic_cast<StatementLabel*>(s)->GetLabel()->Name());
        
        il->Insert( new InstrLabel(buf,dynamic_cast<StatementLabel*>(s)->GetLabel()), InstrList::kInstrList_Rear);
    }
}
void CodeGenerator::_MunchStatementExp(InstrList* il,StatementExp *s){
    //no need return value from _MunchExpBase
    _MunchExpBase(il,s->GetExp());
}
void CodeGenerator::_MunchStatementMove(InstrList* il,StatementMove *s){
    if( s->Left()->Kind()==ExpBase::kExpBase_Mem )
    {
        char buf[1024]={0};
        ExpBase* e1,*e2;
        e1 = s->Left();
        e2 = s->Right();//dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(s->Left())->GetExp())->Right()
        //dynamic_cast<ExpBaseConst*>( dynamic_cast<ExpBaseBinop*>(dynamic_cast<ExpBaseMem*>(s->Left())->GetExp())->Right() )->GetValue()
        sprintf(buf,"mov %%%%%%s,[%%%%%%s]");
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
        sprintf(buf,"mov %%%%%%s,%%%%%%s");
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
    char buf[1024]={0};
    TempList* dst = new TempList;
    TempList* src = new TempList;
    sprintf(buf,"jmp %s",s->GetList()->Get(0)->Name());
    
    il->Insert( new InstrOper( buf, dst, src, s->GetList()->Clone() ), InstrList::kInstrList_Rear);
}
void CodeGenerator::_MunchStatementCjump(InstrList* il,StatementCjump *s){
    char buf[1024]={0};
    
    sprintf(buf,"cmp s0',s1'");
    TempList* dst = new TempList;
    TempList* src = new TempList;
    
    
    src->Insert(_MunchExpBase(il,s->Left()),TempList::kTempList_Rear);
    src->Insert(_MunchExpBase(il,s->Right()),TempList::kTempList_Rear);
    
    il->Insert( new InstrOper( buf, dst, src, 0), InstrList::kInstrList_Rear);
    
    sprintf(buf,"je %s",s->GetTrueLabel()->Name());
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
