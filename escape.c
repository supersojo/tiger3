#include "escape.h"

namespace tiger{

void EscapeHelper::FindEscape(Exp* exp){
    SymTab venv;
    m_logger.D("FindEscape");
    TransExp(&venv,0,exp);
}

void EscapeHelper::TransExp(SymTab* venv,s32 depth,Exp* exp){
    if(exp==0)
        return;
    switch(exp->Kind()){
        case Exp::kExp_Var:
        {
            m_logger.D("TransExp with kExp_Var");
            TransVar(venv,depth,dynamic_cast<VarExp*>(exp)->GetVar());
            break;
        }
        case Exp::kExp_Let:{
            DecNode* head;
            m_logger.D("TransExp with kExp_Let");
            
            venv->BeginScope(ScopeMaker::kScope_Let);
            
            if(dynamic_cast<LetExp*>(exp)->GetDecList()==0)
            {
                m_logger.D("empty decs in let");
            }
            else
            {
                head = dynamic_cast<LetExp*>(exp)->GetDecList()->GetHead();
                while(head){
                    TransDec(venv,depth+1,head->m_dec);
                    head=head->next;
                }
            }
            
            if(dynamic_cast<LetExp*>(exp)->GetBody()==0)
                m_logger.D("empty body in let");
            
            TransExp(venv,depth+1,dynamic_cast<LetExp*>(exp)->GetBody());
            
            venv->EndScope();
            break;
        }
        case Exp::kExp_Call:
        {
            m_logger.D("TransExp with kExp_Call");
            m_logger.D("function call with %s",dynamic_cast<CallExp*>(exp)->Name()->Name());
            ExpNode* head;

            head = dynamic_cast<CallExp*>(exp)->GetList()->GetHead();

            while(head){
                TransExp(venv,depth,head->m_exp);

                head = head->next;
            }
            break;
        }
        case Exp::kExp_Op:
        {
            m_logger.D("TransExp with kExp_Op");

            TransExp(venv,depth,dynamic_cast<OpExp*>(exp)->GetLeft());
            TransExp(venv,depth,dynamic_cast<OpExp*>(exp)->GetRight());

            break;
        }
        case Exp::kExp_Record:
        {
            m_logger.D("TransExp with kExp_Record");
            EFieldNode* head;

            /* id{} */
            head = dynamic_cast<RecordExp*>(exp)->GetList()->GetHead();
            if(head==0){
                return;
            }
            
            while(head){
                
                TransExp(venv,depth,head->m_efield->GetExp());
                
                head = head->next;
            }
            break;
        }
        case Exp::kExp_Assign:
        {
            m_logger.D("TransExp with kExp_Assign");
            TransVar(venv,depth,dynamic_cast<AssignExp*>(exp)->GetVar());
            TransExp(venv,depth,dynamic_cast<AssignExp*>(exp)->GetExp());
            break;
        }
        case Exp::kExp_For:
        {
            m_logger.D("TransExp with kExp_For");
            Symbol* var;
            Exp* body_exp;
             
            var = dynamic_cast<ForExp*>(exp)->GetVar();
            body_exp = dynamic_cast<ForExp*>(exp)->GetExp();
                        
            venv->BeginScope(ScopeMaker::kScope_For);
            
            venv->Enter(venv->MakeSymbol(var),new EnvEntryEscape(depth+1,var->GetEscapeRefer()));
            var->SetEscape(0/*false*/);
            
            TransExp(venv,depth+1,body_exp);
            
            venv->EndScope();
            
            break;
        }
        case Exp::kExp_Seq:{
            m_logger.D("TransExp with kExp_Seq");
            ExpNode* p = dynamic_cast<SeqExp*>(exp)->GetList()->GetHead();
            if(p==0){
                m_logger.D("empty seq exp");
            }
            // return value ignore for now
            while(p){
                TransExp(venv,depth,p->m_exp);
                p = p->next;
            }

            break;
        }
        case Exp::kExp_If:
        {
            m_logger.D("TransExp with kExp_If");
            Exp* if_exp;
            Exp* then_exp;
            Exp* else_exp;
             
            if_exp = dynamic_cast<IfExp*>(exp)->GetTest();
            then_exp = dynamic_cast<IfExp*>(exp)->GetThen();
            else_exp = dynamic_cast<IfExp*>(exp)->GetElsee();
            
            TransExp(venv,depth,if_exp);
            TransExp(venv,depth,then_exp);
            if(else_exp)
                TransExp(venv,depth,else_exp);

            break;
        }
        case Exp::kExp_While:
        {
            m_logger.D("TransExp with kExp_While");
            Exp* test_exp;
            Exp* body_exp;

            test_exp = dynamic_cast<WhileExp*>(exp)->GetTest();
            body_exp = dynamic_cast<WhileExp*>(exp)->GetTest();

            TransExp(venv,depth,test_exp);
            
            venv->BeginScope(ScopeMaker::kScope_While);// only for "break" match
            TransExp(venv,depth,body_exp);
            venv->EndScope();
            
            break;
        }
        case Exp::kExp_Array:
        {
            m_logger.D("TransExp with kExp_Array");

            TransExp(venv,depth,dynamic_cast<ArrayExp*>(exp)->GetSize());
            TransExp(venv,depth,dynamic_cast<ArrayExp*>(exp)->GetInit());

            break;
        }
        default:
            break;
    }
}

void EscapeHelper::TransFunctionDec(SymTab* venv, s32 depth, Dec* dec)
{
    FunDecNode* fundec_head;
    FieldNode* head;
    
    m_logger.D("TransFunctionDec");
    
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();

    /* process all function body decs */
    fundec_head = dynamic_cast<FunctionDec*>(dec)->GetList()->GetHead();
    while(fundec_head){
        
        /* function foo() */
        if(fundec_head->m_fundec->GetList()!=0)
            head = fundec_head->m_fundec->GetList()->GetHead();
        
        venv->BeginScope(ScopeMaker::kScope_Fun);
        
        if(fundec_head->m_fundec->GetList()!=0){
            while(head){
                venv->Enter(venv->MakeSymbol(head->m_field->Name()),new EnvEntryEscape(depth+1,head->m_field->Name()->GetEscapeRefer()/*false*/) );
                head->m_field->Name()->SetEscape(0/*false*/);
                head = head->next;
            }
        }
        
        TransExp(venv,depth+1,fundec_head->m_fundec->GetExp());

        venv->EndScope();
        
        fundec_head = fundec_head->next;
    }    
}

void EscapeHelper::TransDec(SymTab* venv,s32 depth,Dec* dec){
    switch(dec->Kind()){
        case Dec::kDec_Var:{
            m_logger.D("TransDec with kDec_Var");
            venv->Enter(venv->MakeSymbol(dynamic_cast<VarDec*>(dec)->GetSymbol()),new EnvEntryEscape(depth,dynamic_cast<VarDec*>(dec)->GetSymbol()->GetEscapeRefer()));
            dynamic_cast<VarDec*>(dec)->GetSymbol()->SetEscape(0/*false*/);
            TransExp(venv,depth,dynamic_cast<VarDec*>(dec)->GetExp());
            break;
        }
        case Dec::kDec_Function:{
            m_logger.D("TransDec with kDec_Function");
            TransFunctionDec(venv,depth,dec);
            break;
        }
        default:
            break;
    }
}

void EscapeHelper::TransVar(SymTab* venv,s32 depth,Var* var){
    switch(var->Kind()){
        case Var::kVar_Simple:{
            m_logger.D("TransVar with kVar_Simple");
            EnvEntryEscape* t;
            //dynamic_cast<SimpleVar*>(var)->GetSymbol()
            t = dynamic_cast<EnvEntryEscape*>( venv->Lookup( venv->MakeSymbol( dynamic_cast<SimpleVar*>(var)->GetSymbol() ) ) );
            if((depth > t->Depth()) && t->GetEscape()==0){
                t->SetEscape(1/*true*/);
                m_logger.D("Get a escape var with %s",dynamic_cast<SimpleVar*>(var)->GetSymbol()->Name());
            }
            break;
        }
        case Var::kVar_Field:
        {
            m_logger.D("TransVar with kVar_Field");
            TransVar(venv,depth,dynamic_cast<FieldVar*>(var)->GetVar());
            break;

        }
        case Var::kVar_Subscript:
        {

            m_logger.D("TransVar with kVar_Subscript");
            TransVar(venv,depth,dynamic_cast<SubscriptVar*>(var)->GetVar());

            TransExp(venv,depth,dynamic_cast<SubscriptVar*>(var)->GetExp());
            
            break;
            
        }
    }
}


}// namespace tiger
