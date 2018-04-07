#include "tiger_llvm.h"

namespace tiger{

void IRGen::Init()
{
    m_context = IRGenContext::Get();
}
Level* IRGen::OutmostLevel()
{
    if(m_top_level==0){
        //create the mout most function and the entry basic block
        m_top_level = new Level(0, new FrameBase(FrameBase::kFrame_X86));
        llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*(m_context->C())),false/*var args flag*/);
        llvm::Function* f = llvm::Function::Create(ft,llvm::Function::ExternalLinkage,"main",m_context->M());
        llvm::BasicBlock* bb = llvm::BasicBlock::Create( *(m_context->C()), "entry", f);
        m_context->B()->SetInsertPoint(bb);
        m_top_level->SetFunc(f);
    }

    return m_top_level;
}
/*
 1 2 3 -> ConstantValue
 a b c -> Value
 a>b ->Value
 
 level - function - basic blocks 
 a=1
 if a>1 a=0
 a=3
 
*/
llvm::Value*  IRGen::IRGenVar(SymTab* venv,SymTab* tenv,Level* level,Var* var,llvm::BasicBlock* dest_bb)
{
    switch(var->Kind()){
        case Var::kVar_Simple:
        {
            EnvEntryVarLLVM* t;
            t = dynamic_cast<EnvEntryVarLLVM*>(venv->Lookup(venv->MakeSymbol(dynamic_cast<SimpleVar*>(var)->GetSymbol())));
            TIGER_ASSERT(t!=0,"var %s not found",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            return t->GetLLVMValue();
            
        }
        case Var::kVar_Field:
        {


        }
        case Var::kVar_Subscript:
        {

            
        }
        default:
            break;
    }
    m_logger.W("shoud not reach here %s,%d",__FILE__,__LINE__);
    return 0;
}
TypeBase* IRGen::IRGenTy(SymTab* tenv,Level* level,Ty* ty)
{
    switch(ty->Kind())
    {
        /*
        type a={x:int,y:{a:int}}
        type a = x
        NameTyPair("a",NameTy("x"))
        */
        case Ty::kTy_Name:
        {
            EnvEntryVar* t;
            t = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<NameTy*>(ty)->Name())));
            TIGER_ASSERT(t!=0,"type %s not found",dynamic_cast<NameTy*>(ty)->Name()->Name());
            return t->Type();
        }
        case Ty::kTy_Record:
        {
            /*
            type a = {a:int,b:string}
            (Field("a","int"),Field("b","string"))
            */
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
            /*
            type a=array of b
            ArrayTy("b")
            */
            EnvEntryVar* p;
            p = dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(dynamic_cast<ArrayTy*>(ty)->Name())));
            return new TypeArray(p->Type());
        }
        default:
            break;
    }
    return 0;
}
/*
int Int32Ty
string i8*
atype a={a:int,b:string}
*/
void IRGen::IRGenTypeDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec)
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
                     new EnvEntryVarLLVM( 
                                      new TypeName(tenv->MakeSymbol(head->m_nametypair->Name()),0/* TypeBase* */),
                                      EnvEntryVarLLVM::kEnvEntryVarLLVM_For_Type, 0/*llvm::Type*/,0/*llvm::Value* */
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
        TypeBase* t = IRGenTy(tenv,level,head->m_nametypair->Type());
        if(t->Kind()!=TypeBase::kType_Name){
            /*
            type a={x:int,y:int}
            When type "a" insert tenv, it's type is dummy TypeName.Now we get real type so refill it here.
            */
            dynamic_cast<EnvEntryVar*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())))->Update(t);
        }
        else{
            /* gen llvm type 
            type a={x:int,y:string} aStruct
            type b={x:int,y:a}
            StructType{
                Int32Ty,
                sStruct*
            }
            
            type a={x:int,y:a}
            structtype{
                Int32Ty,
                aStruct*
            }
            */
            /*
            type b=int
            type a=b
            */
            if(t->Kind()==TypeBase::kType_Record){//fill llvm struct type 
                /*
                type a={x:int,y:int}
                */
                
            }
            EnvEntryVarLLVM* p = dynamic_cast<EnvEntryVarLLVM*>(tenv->Lookup(tenv->MakeSymbol(head->m_nametypair->Name())));
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
void IRGen::IRGenDec(SymTab* venv,SymTab* tenv,Level* level,Dec* dec,llvm::BasicBlock* dest_bb)
{
    switch(dec->Kind())
    {
        case Dec::kDec_Var:{
            m_logger.D("type check with kDec_Var");
            llvm::Value* t;
            if(dynamic_cast<VarDec*>(dec)->GetExp()->Kind()==Exp::kExp_Array){
                //array
                // var a=intarry [12] of [1]
            }else if(dynamic_cast<VarDec*>(dec)->GetExp()->Kind()==Exp::kExp_Record){
                //record
            }else{
                // simple var declaration
                // var a:=1
                t = IRGenExp(venv,tenv,level,dynamic_cast<VarDec*>(dec)->GetExp(),dest_bb);
            }
            
        
            llvm::Value* v = m_context->B()->CreateAlloca(llvm::Type::getInt32Ty( *(m_context->C()) ));
            m_context->B()->CreateStore(t,v);
            venv->Enter( 
                    venv->MakeSymbol( dynamic_cast<VarDec*>(dec)->GetSymbol() ), 
                    new EnvEntryVarLLVM( 
                        0, 
                        EnvEntryVarLLVM::kEnvEntryVarLLVM_For_Value,
                        llvm::Type::getInt32Ty( *(m_context->C()) ),
                        v
                    ) 
            );

            return ;
        }
        case Dec::kDec_Function:
        {
            return;
        }
        case Dec::kDec_Type:{
            IRGenTypeDec(venv,tenv,level,dec);
            return;
        }
        default:
            break;
    }
}
llvm::Value* IRGen::IRGenExpLet(SymTab* venv,SymTab* tenv,Level* level,Exp* exp,llvm::BasicBlock* dest_bb)
{

    llvm::Value* ret;
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
            IRGenDec(venv,tenv,level,p->m_dec,dest_bb);
            p = p->next;
        }
    }
    
    if(body){
        m_logger.D("TransDec body");
        IRGenExp(venv,tenv,level,body,dest_bb);
    }
    
    
    tenv->EndScope();
    venv->EndScope();
    
    return 0;
     
}
llvm::Value* IRGen::IRGenExp(SymTab* venv,SymTab* tenv,Level* level,Exp* e,llvm::BasicBlock* dest_bb)
{
    switch( e->Kind() )
    {
        case Exp::kExp_Int:
        {
            m_logger.D("kExp_Int");
            return m_context->B()->getInt32( dynamic_cast<IntExp*>(e)->GetInt() );
        }
        case Exp::kExp_Var:
        {
            return IRGenVar(venv,tenv,level,dynamic_cast<VarExp*>(e)->GetVar(),dest_bb);
        }
        case Exp::kExp_Assign:
        {
            m_logger.D("kExp_Assign");
            llvm::Value* a = IRGenVar(venv,tenv,level,dynamic_cast<AssignExp*>(e)->GetVar(),dest_bb);
            llvm::Value* b = IRGenExp(venv,tenv,level,dynamic_cast<AssignExp*>(e)->GetExp(),dest_bb);
            m_context->B()->CreateStore(b,a);
            return 0;
        }
        case Exp::kExp_Seq:
        {
            TIGER_ASSERT(e->Kind()==Exp::kExp_Seq,"seq exp expected");
            
            ExpNode* p = dynamic_cast<SeqExp*>(e)->GetList()->GetHead();
            if(p==0){
                m_logger.D("empty seq exp");
                return 0;
            }
            // return value ignore for now
            while(p){
                IRGenExp(venv,tenv,level,p->m_exp,dest_bb);
                p = p->next;
            }
            return 0;
        }
        case Exp::kExp_Op:
        {
            m_logger.D("kExp_Op");
            llvm::Value* l_val,*r_val;
            Exp* l,*r;
            llvm::Value* l_vv=0,*r_vv=0;
            Oper* oper;
            l = dynamic_cast<OpExp*>(e)->GetLeft();
            r = dynamic_cast<OpExp*>(e)->GetRight();
            l_val = IRGenExp(venv,tenv,level,l,0);
            r_val = IRGenExp(venv,tenv,level,r,0);
            if(!llvm::isa<llvm::Constant>(*l_val)){
                l_vv = m_context->B()->CreateLoad(l_val);
            }
            if(!llvm::isa<llvm::Constant>(*r_val)){
                r_vv = m_context->B()->CreateLoad(r_val);
            }
            oper = dynamic_cast<OpExp*>(e)->GetOper();
            switch(oper->Kind())
            {
                case Oper::kOper_Add:
                {
                    m_logger.D("add");
                    return m_context->B()->CreateAdd(l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Sub:
                {
                    return m_context->B()->CreateSub(l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Mul:
                {
                    return m_context->B()->CreateMul(l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Div:
                {
                    return m_context->B()->CreateSDiv(l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Lt:
                {
                    return m_context->B()->CreateICmp(llvm::CmpInst::ICMP_SLT,l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Gt:
                {
                    return m_context->B()->CreateICmp(llvm::CmpInst::ICMP_SGT,l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Le:
                {
                    return m_context->B()->CreateICmp(llvm::CmpInst::ICMP_SLE,l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                case Oper::kOper_Ge:
                {
                    return m_context->B()->CreateICmp(llvm::CmpInst::ICMP_SGE,l_vv?l_vv:l_val,r_vv?r_vv:r_val);
                }
                default:
                {
                    TIGER_ASSERT(1,"Should not reach here");
                }
            }
        }
        case Exp::kExp_If:
        {
            /*
            XXXX1
            if_statement
            XXXX2
            
            if need create new bb, new bb should jump to XXXX2
            */
            Exp* test;
            Exp* then;
            Exp* elsee;
            
            llvm::Value* test_val;
            llvm::Value* then_val;
            llvm::Value* elsee_val;
            
            llvm::BasicBlock* then_bb;
            llvm::BasicBlock* elsee_bb;
            llvm::BasicBlock* end_bb;
            
            test = dynamic_cast<IfExp*>(e)->GetTest();
            then = dynamic_cast<IfExp*>(e)->GetThen();
            elsee = dynamic_cast<IfExp*>(e)->GetElsee();
            
            then_bb = llvm::BasicBlock::Create( *(m_context->C()),"then",level->GetFunc());
            elsee_bb = llvm::BasicBlock::Create( *(m_context->C()),"else",level->GetFunc());
            end_bb = llvm::BasicBlock::Create( *(m_context->C()),"end",level->GetFunc());
            
            test_val = IRGenExp(venv,tenv,level,test,end_bb);
            m_context->B()->CreateCondBr(test_val,then_bb,elsee_bb);
            
            m_context->B()->SetInsertPoint(then_bb);
            then_val = IRGenExp(venv,tenv,level,then,end_bb);
            m_context->B()->SetInsertPoint(then_bb);
            m_context->B()->CreateBr(end_bb);
            
            m_context->B()->SetInsertPoint(elsee_bb);
            elsee_val = IRGenExp(venv,tenv,level,elsee,end_bb);
            m_context->B()->SetInsertPoint(elsee_bb);
            m_context->B()->CreateBr(end_bb);
            
            m_context->B()->SetInsertPoint(end_bb);
            if(dest_bb)
                m_context->B()->CreateBr(dest_bb);
            return 0;
            
        }
        case Exp::kExp_While:
        {
            /*
            XXXX1
            while_statement
            XXXX2
            while need jump to XXXX2 // create new bb  the new bb should jump to the XXXX2
            */
            Exp* test;
            Exp* body;
            
            llvm::Value* test_val;
            llvm::Value* body_val;
            
            llvm::BasicBlock* loop_bb;
            llvm::BasicBlock* body_bb;
            llvm::BasicBlock* end_bb;
            
            test = dynamic_cast<WhileExp*>(e)->GetTest();
            body = dynamic_cast<WhileExp*>(e)->GetExp();
            
            loop_bb = llvm::BasicBlock::Create( *(m_context->C()),"loop",level->GetFunc());
            body_bb = llvm::BasicBlock::Create( *(m_context->C()),"body",level->GetFunc());
            end_bb = llvm::BasicBlock::Create( *(m_context->C()),"end",level->GetFunc());
            
            m_context->B()->CreateBr(loop_bb);
            m_context->B()->SetInsertPoint(loop_bb);
            test_val = IRGenExp(venv,tenv,level,test,end_bb);
            m_context->B()->SetInsertPoint(loop_bb);
            m_context->B()->CreateCondBr(test_val,body_bb,end_bb);
            
            m_context->B()->SetInsertPoint(body_bb);
            body_val = IRGenExp(venv,tenv,level,body,end_bb);
            m_context->B()->SetInsertPoint(body_bb);
            m_context->B()->CreateBr(loop_bb);
            
            m_context->B()->SetInsertPoint(end_bb);
            if(dest_bb)
                m_context->B()->CreateBr(dest_bb);
            return 0;
        }
        case Exp::kExp_Let:
        {
            return IRGenExpLet(venv,tenv,level,e,dest_bb);
        }
        default:
        {
            return 0;
        }
    }
}



}//namespace tiger