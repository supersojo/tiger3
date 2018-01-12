#include "parser.h"
#include "tiger_assert.h"

namespace tiger{

namespace parser{

Parser::Parser(scanner::SourceCodeStreamBase* stream){
    m_scanner = new scanner::Scanner(stream);
    m_logger = new LoggerStdio;
    m_logger->SetLevel(LoggerBase::kLogger_Level_Error);
    m_logger->SetModule("parser");
}

bool Parser::Parse(Exp** prog)
{
    s32 v;
    Token t;
    Exp* exp;
    
    m_logger->D("Begin Parse()");
    exp = _ParseExp();
    m_logger->D("End Parse()");
    v = m_scanner->Next(&t);
    m_logger->D("%s",token_string((TokenType)v));
    TIGER_ASSERT(v==kToken_EOT,"Expected kToken_EOT here, but %s provided",token_string((TokenType)v));
    m_logger->D("parse ok");
    //print something about absyn
    m_logger->D("Top exp: %s",Exp::KindString(exp->Kind()));
    if(prog)
        *prog = exp;
    return true;
}
/*
Exp* Parser::ParseExpSeq()
{
    ExpNode* head;
    m_logger->D("Begin ParseExpSeq()");
	head=_ParseExpSeq(0);
    m_logger->D("End ParseExpSeq()");
    if(head)
        return new SeqExp(new ExpList(head));
    else
        return 0;
}
*/
ExpNode* Parser::_ParseExpSeq(ExpNode* head)
{
    s32 v;
    Token t;
    Exp* exp;
    m_logger->D("Begin _ParseExpSeq()");
    ExpNode* anode,*p,*q;
    exp = _ParseExp();
    /* () */
    if(exp==0){
        m_logger->D("empty exp list");
        v = m_scanner->Next(&t);
        anode = head;
        goto retrieve;
    }
    anode = new ExpNode;
    anode->m_exp = exp;
    m_logger->D("Get a new ExpNode");
    if(head){
        head->next=anode;
        anode->prev=head;
    }
    v = m_scanner->Next(&t);
    if(v==kToken_SEMICOLON){
        /* expseq -> exp; expseq */
        m_logger->D("End _ParseExpSeq()");
        return _ParseExpSeq(anode);
    }else{
retrieve:
        m_logger->D(token_string((TokenType)v));
        m_logger->D("retrieve the head node of ExpNode list");
        m_scanner->Back(&t);
        p = anode;
        q = anode;
        while(p){
            q = p;
            p = p->prev;
        }
        m_logger->D("End _ParseExpSeq()");
        return q;
    }
}
Var* Parser::_ParseLvalueTerm()
{
    s32 v;
    Token t;
    
    v = m_scanner->Next(&t);
    if(v==kToken_ID){
        m_logger->D("Get a new SimpleVar with %s",t.u.name);
        return new SimpleVar(new Symbol(t.u.name));
    }
    TIGER_ASSERT(0,"Wrong token %s here",token_string((TokenType)v));
    return 0;
}
Var* Parser::_ParseLvalueRest(Var* var)
{
    s32 v,v1;
    Token t,t1;
    /* lvalue' -> . lvalue' */
    v = m_scanner->Next(&t);
    if(v==kToken_DOT){
        v1 = m_scanner->Next(&t1);
        TIGER_ASSERT(v1==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v1));
        m_logger->D("Get a new field var with %s",t1.u.name);
        return _ParseLvalueRest(new FieldVar(var,new Symbol(t1.u.name)));
    }
	
    /* lvalue' -> [ exp ] lvalue' */
    if(v==kToken_LSQB){
        Exp* exp;
        exp = _ParseExp();
        v1 = m_scanner->Next(&t1);
        TIGER_ASSERT(v1==kToken_RSQB,"Expected kToken_RSQB here, but %s provided",token_string((TokenType)v1));
        m_logger->D("Get a new subscript var");
        return _ParseLvalueRest(new SubscriptVar(var,exp));
    }
    
    /* lvalue'-> empty */
    if(v!=kToken_EOT)
        m_scanner->Back(&t);
   return var; 
}
Var* Parser::_ParseLvalue(Var* head)
{
    Var * var;
    if(head==0){
        m_logger->D("_ParseLvalueTerm()");
        var = _ParseLvalueTerm();
        return _ParseLvalueRest(var);
    }else{
        m_logger->D("_ParseLvalueRest()");
        return _ParseLvalueRest(head);
    }
}

ExpNode* Parser::_ParseParms(ExpNode* head)
{
    s32 v,v1;
    Token t,t1;
    Exp* exp;
    ExpNode* anode,*p,*q;
    v = m_scanner->Next(&t);
    /* parms -> empty */
    if(v==kToken_RPAR){
        m_scanner->Back(&t);
        return 0;
    }
    /* parms -> exp */
    /* parms -> exp , parms */
    if(v!=kToken_EOT){
        m_scanner->Back(&t);
        exp = _ParseExp();
        anode = new ExpNode;
        anode->m_exp = exp;
        if(head){
            head->next = anode;
            anode->prev = head;
        }
        m_logger->D("Get a new ExpNode");
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_COMMA){
            return _ParseParms(anode);
        }
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
        m_logger->D("retrieve the head node of the ExpNode list");
        p = anode;
        q = anode;
        while(p){
            q = p;
            p = p->prev;
        }
        return q;
    }
    return 0;
}
EFieldList* Parser::_ParseIdList()
{
    IdList idList;
    return idList.Parse(this);
    
}
DecList* Parser::_ParseDecs()
{
    s32 v;
    Token t;
    Dec* adec;
    DecNode* anode,*head,*tail;
    DecList* list=0;
    head = 0;
    tail = 0;
    m_logger->D("Begin _ParseDecs");
    do{
        v = m_scanner->Next(&t);
        m_logger->D(token_string((TokenType)v));
        if(v==kToken_IN){
            m_scanner->Back(&t);
            if(head){
                m_logger->D("Get a new DecList");
                return new DecList(head);
            }else{
                m_logger->D("Get a empty DecList");
                return 0;
            }
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        Declaration dec;
        adec = dec.Parse(this);
        if(adec){
            anode = new DecNode;
            anode->m_dec = adec;
            m_logger->D("Get a new DecNode");
            if(!head)
                head = anode;
            if(!tail)
                tail = anode;
            else{
                tail->next = anode;
                anode->prev = tail;
                tail = anode;
            }
        }
    }while(1);
    
    return 0;
}
Exp* Parser::_ParseTerm()
{
    s32 v,v1,v2;
    Token t,t1,t2;
    
    v = m_scanner->Next(&t);
    /* exp -> num */
    if(v==kToken_NUM){
        m_logger->D("Get a new IntExp with %d",t.u.ival);
        return new IntExp(t.u.ival);
    }
    /* exp -> string */
    if(v==kToken_STR){
        m_logger->D("Get a new StringExp with %s",t.u.sval);
        return new StringExp(t.u.sval);
    }
    /* exp -> nil */
    if(v==kToken_NIL){
        m_logger->D("Get a new NilExp");
        return new NilExp();
    }
    /* exp -> if exp then exp else exp or exp -> if exp then exp */
    if(v==kToken_IF){
        m_logger->D("Get a new IfExp");
        Exp* if_exp,*then_exp,*else_exp;
        if_exp=_ParseExp();
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_THEN,"Expected kToken_THEN here, but %s provided",token_string((TokenType)v));
        then_exp=_ParseExp();
        v = m_scanner->Next(&t);
        if(v==kToken_ELSE){
            else_exp=_ParseExp();
            return new IfExp(if_exp,then_exp,else_exp);
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        return new IfExp(if_exp,then_exp,0) ;
    }
    
    /* exp -> while exp do exp */
    if(v==kToken_WHILE){
        m_logger->D("Get a new WhileExp");
        Exp* test_exp;
        Exp* body_exp;
        test_exp=_ParseExp();
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_DO,"Expected kToken_DO here, but %s provided",token_string((TokenType)v));
        body_exp=_ParseExp();
        return new WhileExp(test_exp,body_exp);
    }
    /* exp -> for id := exp to exp do exp */
    if(v==kToken_FOR){
        m_logger->D("Get a new ForExp");
        Symbol* id;
        Exp* lo_exp;
        Exp* hi_exp;
        Exp* body_exp;
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        id = new Symbol(t.u.name);
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_ASSIGN,"Expected kToken_ASSIGN here, but %s provided",token_string((TokenType)v));
        lo_exp=_ParseExp();
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_TO,"Expected kToken_TO here, but %s provided",token_string((TokenType)v));
        hi_exp=_ParseExp();
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_DO,"Expected kToken_DO here, but %s provided",token_string((TokenType)v));
        body_exp=_ParseExp();
        return new ForExp(id,lo_exp,hi_exp,body_exp);
    }
    /* exp -> let decs in end or let decs in explist end */
    if(v==kToken_LET){
        m_logger->D("Get a new LetExp");
        DecList* declist;
        ExpNode* head;
        declist=_ParseDecs();
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_IN,"Expected kToken_IN here, but %s provided",token_string((TokenType)v));
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_END){
            
            return new LetExp(declist,0);	
        }
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
        head = _ParseExpSeq(0);
        v = m_scanner->Next(&t);
        TIGER_ASSERT(v==kToken_END,"Expected  kToken_END here, but %s provided",token_string((TokenType)v));
        if(v==kToken_END){
            m_logger->D("return a new LetExp");
            return new LetExp(declist,new SeqExp(new ExpList(head)));	
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        m_logger->D(token_string((TokenType)v));
        TIGER_ASSERT(0,"Wrong token %s here",token_string((TokenType)v));
        return 0;
    } 
    /* exp -> break */
    if(v==kToken_BREAK){
        m_logger->D("Get a new BreakExp");
        return new BreakExp();
    }
    if(v==kToken_LPAR){
        ExpNode* head;
        v1 = m_scanner->Next(&t1);
        /* exp -> () */
        if(v1==kToken_RPAR){
            m_logger->D("Get a empty Exp");
            return 0;
        }
        /* exp -> ( expseq ) */
        m_scanner->Back(&t1);
        head=_ParseExpSeq(0);
        v1 = m_scanner->Next(&t1);
        TIGER_ASSERT(v1==kToken_RPAR,"Expected kToken_RPAR here, but %s provided",token_string((TokenType)v1));
        m_logger->D("Get a new SeqExp");
        return new SeqExp(new ExpList(head));
    }
    /* exp -> - exp */
    if(v==kToken_SUB){
        Exp* exp;
        exp = _ParseExp();
        return new OpExp(new Oper(Oper::kOper_Sub),0,exp);
    }
    
    if(v==kToken_ID){
        ExpNode* head;
        /*  exp -> id () or exp -> id (parms)*/
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_LPAR){
            head=_ParseParms(0);
            v2 = m_scanner->Next(&t2);
            TIGER_ASSERT(v2==kToken_RPAR,"Expected kToken_RPAR here, but %s provided",token_string((TokenType)v2));
            m_logger->D("Get a new CallExp");
            return new CallExp(new Symbol(t.u.name),new ExpList(head));
        }
        /*  exp -> id {} or exp -> id{id=exp{,id=exp}}*/
        if(v1==kToken_LBRA){
            EFieldList* efields;
            efields = _ParseIdList();
            v2 = m_scanner->Next(&t2);
            TIGER_ASSERT(v2==kToken_RBRA,"Expected kToken_RBRA here, but %s provided",token_string((TokenType)v2));
            m_logger->D("Get a new RecordExp");
            return new RecordExp(new Symbol(t.u.name),efields);
        }
        
        Var* var1=0;
        /* exp -> id [exp] of exp */
        if(v1==kToken_LSQB){
            Symbol* id;
            Exp* exp1,*exp2;
            
            id = new Symbol(t.u.name);
            exp1 = _ParseExp();
            v1 = m_scanner->Next(&t1);
            TIGER_ASSERT(v1==kToken_RSQB,"Expected kToken_RSQB here, but %s provided",token_string((TokenType)v1));
            v1 = m_scanner->Next(&t1);
            if(v1==kToken_OF){
                exp2 = _ParseExp();
                m_logger->D("Get a new ArrayExp");
                return new ArrayExp(id,exp1,exp2);
            }
            m_logger->D("Get a new subscript var with %s",id->Name());
            var1 = new SubscriptVar(new SimpleVar(id),exp1);
            //delete id;
        }
        
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
        
        /* exp -> lvalue or exp -> lvalue := exp */
        Var* var;
        if(var1==0)
            m_scanner->Back(&t);
        var = _ParseLvalue(var1);
        v = m_scanner->Next(&t);
        
        /* exp -> lvalue := exp */
        if(v==kToken_ASSIGN){
            Exp* exp;
            exp = _ParseExp();
            m_logger->D("Get a new AssignExp");
            return new AssignExp(var,exp);
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        
        m_logger->D("Get a new VarExp");
        /* exp -> lvalue */
        return new VarExp(var);
    }
    /* should not reach here */
    m_logger->D("Unexpected token %s here",token_string((TokenType)v));
    return 0;
    
}

Exp* Parser::_ParseExp()
{
    ExpOr exp;
    return exp.Parse(this);

}
Parser::~Parser(){
    delete m_scanner;
    delete m_logger;
}

Exp* Term::Parse(Parser* parser)
{
    return parser->_ParseTerm();
}

Exp* ExpMulDiv::Parse(Parser* parser){
    Exp* left;
    Exp* rest;
    
    left = Term::Parse(parser);
    rest = _ParseExpRest(left,parser);
    if(rest)
        return rest;
    else
        return left;
}
Exp* ExpMulDiv::_ParseExpRest(Exp* left,Parser* parser){
    s32 v;
    Token t;
    Exp *right,*rest;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_MUL){
        right = Term::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Mul");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Mul),left,right),parser);
        return rest;
    }
    if(v==kToken_DIV){
        right = Term::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Div");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Div),left,right),parser);
        return rest;
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    /* empty exp */
    return left;
}
Exp* ExpAddSub::Parse(Parser* parser){
    Exp* left;
    Exp* rest;
    left=ExpMulDiv::Parse(parser);
    rest=_ParseExpRest(left,parser);
    if(rest)
        return rest;
    else
        return left;
}
Exp* ExpAddSub::_ParseExpRest(Exp* left,Parser* parser){
    s32 v;
    Token t;
    Exp* right,*rest;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_ADD){
        right=ExpMulDiv::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Add");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Add),left,right),parser);
        return rest;
    }
    if(v==kToken_SUB){
        right=ExpMulDiv::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Sub");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Sub),left,right),parser);
        return rest;
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    /* empty exp */
    return left;
}
Exp* ExpCompare::Parse(Parser* parser){
    Exp* left;
    Exp* rest;
    left = ExpAddSub::Parse(parser);
    rest = _ParseExpRest(left,parser);
    if(rest)
        return rest;
    else
        return left;
}
Exp* ExpCompare::_ParseExpRest(Exp* left,Parser* parser){
    s32 v;
    Token t;
    Exp* right,*rest;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_LT){
        right = ExpAddSub::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Lt");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Lt),left,right),parser);
        return rest;
    }
    if(v==kToken_GT){
        right = ExpAddSub::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Gt");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Gt),left,right),parser);
        return rest;
    }
    if(v==kToken_LE){
        right = ExpAddSub::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Le");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Le),left,right),parser);
        return rest;
    }
    if(v==kToken_GE){
        right = ExpAddSub::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Ge");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Ge),left,right),parser);
        return rest;
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    /* empty exp */
    return left;
}

Exp* ExpEqualOrNot::Parse(Parser* parser){
    Exp* left,*rest;
    left=ExpCompare::Parse(parser);
    rest=_ParseExpRest(left,parser);
    if(rest)
        return rest;
    else
        return left;
}
Exp* ExpEqualOrNot::_ParseExpRest(Exp* left,Parser* parser){
    s32 v;
    Token t;
    Exp* right;
    Exp* rest;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_EQUAL){
        right=ExpCompare::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Eq");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Eq),left,right),parser);
        return rest;
    }
    if(v==kToken_NOTEQUAL){
        right=ExpCompare::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Neq");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Neq),left,right),parser);
        return rest;
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    /* empty exp */
    return left;
}
Exp* ExpAnd::Parse(Parser* parser){
    Exp* left,*rest;
    left=ExpEqualOrNot::Parse(parser);
    rest=_ParseExpRest(left,parser);
    if(rest)
        return rest;
    else
        return left;
}
Exp* ExpAnd::_ParseExpRest(Exp* left,Parser* parser){
    s32 v;
    Token t;
    Exp* right,*rest;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_AND){
        
        right=ExpEqualOrNot::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_And");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_And),left,right),parser);
        return rest;
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    /* empty exp */
    return left;
}
Exp* ExpOr::Parse(Parser* parser){
    Exp* left,*rest;
    left=ExpAnd::Parse(parser);
    rest=_ParseExpRest(left,parser);
    if(rest)
        return rest;
    else
        return left;
}
Exp* ExpOr::_ParseExpRest(Exp* left,Parser* parser){
    s32 v;
    Token t;
    Exp* right,*rest;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_OR){
        
        right=ExpAnd::Parse(parser);
        parser->GetLogger()->D("Get a new OpExp with kOper_Or");
        rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Or),left,right),parser);
        return rest;
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    /* empty exp */
    return left;
}
EFieldList* IdList::Parse(Parser* parser){
    EFieldNode* head,*rest;
    head = _ParseIdList(parser);
    rest = _ParseIdListRest(head,parser);
    if(rest)
        return new EFieldList(rest);
    else
        return new EFieldList(head);
}
EFieldNode* IdList::_ParseIdList(Parser* parser){
    s32 v,v1,v2;
    Token t,t1,t2;
    EFieldNode* n;
    v = parser->GetScanner()->Next(&t);
    /* idlist->empty */
    if(v==kToken_RBRA){
        parser->GetScanner()->Back(&t);
        /* empty EFieldNode */
        return 0;
    }
    TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
    if(v==kToken_ID){
        Exp* exp;
        v1 = parser->GetScanner()->Next(&t1);
        TIGER_ASSERT(v1==kToken_EQUAL,"Expected kToken_EQUAL here, but %s provided",token_string((TokenType)v1));
        if(v1==kToken_EQUAL){
            exp = parser->ParseExp();
        }
        n = new EFieldNode;
        n->m_efield=new EField(new Symbol(t.u.name),exp);
        parser->GetLogger()->D("Get a new EFieldNode");
        return n;
    }
    /* empty EFieldNode */
    return 0;
}
EFieldNode* IdList::_ParseIdListRest(EFieldNode* head,Parser* parser){
    s32 v;
    Token t;
    EFieldNode* anode,*p,*q;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_COMMA){
        anode=_ParseIdList(parser);
        if(head){
            head->next=anode;
            anode->prev=head;
            
        }
        return _ParseIdListRest(anode,parser);
    }
    if(v!=kToken_EOT)/* v should be kToken_RBRA*/
        parser->GetScanner()->Back(&t);
    p = head;
    q = head;
    while(p){
        q = p;
        p = p->prev;
    }
    return q;
}
Ty* TyDeclaration::Parse(Parser* parser){
    s32 v;
    Token t;
    FieldList* fields;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_ID){
        parser->GetLogger()->D("Get a new NameTy");
        return new NameTy(new Symbol(t.u.name));
    }
    if(v==kToken_LBRA){
        fields = _ParseTyFields(0,parser);
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_RBRA,"Expected kToken_RBRA here, but %s provided",token_string((TokenType)v));
        parser->GetLogger()->D("Get a new RecordTy");
        return new RecordTy(fields);
    }
    if(v==kToken_ARRAY){
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_OF,"Expected kToken_OF here, but %s provided",token_string((TokenType)v));
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        parser->GetLogger()->D("Get a new ArrayTy");
        return new ArrayTy(new Symbol(t.u.name));
    }
    if(v!=kToken_EOT)
        parser->GetScanner()->Back(&t);
    
    return 0;
}
FieldList* TyDeclaration::_ParseTyFields(FieldNode* head,Parser* parser){
    s32 v;
    Token t;
    Symbol* id,*type_id;
    FieldNode* afield;
    FieldNode* p,*q;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_RBRA){
        parser->GetScanner()->Back(&t);
        return 0;
    }
    if(v==kToken_ID){
        id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_COLON,"Expected kToken_COLON here, but %s provided",token_string((TokenType)v));
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        type_id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        
        afield = new FieldNode;
        afield->m_field = new Field(id,type_id);
        
        parser->GetLogger()->D("Get a new FieldNode");
        
        /* link it */
        if(head){
            head->next=afield;
            afield->prev=head;
        }
        
        if(v==kToken_COMMA){
            return _ParseTyFields(afield,parser);
        }
        if(v!=kToken_EOT)// v should be kToken_RBRA
            parser->GetScanner()->Back(&t);
            
        /* get field head */
        p = afield;
        q = afield;
        while(p){
            q = p;
            p = p->prev;
        }
        parser->GetLogger()->D("Get a new FieldList");
        return new FieldList(q);
    }
    return 0;
}
Dec* TyDec::Parse(Parser* parser){
    s32 v;
    Token t;
    Symbol* id;
    Ty* id_ty;
    NameTyPairNode* head;
    parser->GetLogger()->D("Begin TyDec::Parse()");
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_TYPE){
        parser->GetScanner()->Back(&t);
        head = _ParseTyDec(0,parser);
        parser->GetLogger()->D("End TyDec::Parse()");
        return new TypeDec(new NameTyPairList(head));
    }
    return 0;
}
NameTyPairNode* TyDec::_ParseTyDec(NameTyPairNode* head,Parser* parser){
    s32 v;
    Token t;
    Symbol* id;
    Ty* id_ty;
    NameTyPairNode* n,*p,*q;
    parser->GetLogger()->D("Begin TyDec::_ParseTyDec()");
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_TYPE){
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_EQUAL,"Expected kToken_EQUAL here, but %s provided",token_string((TokenType)v));
        TyDeclaration ty;
        id_ty=ty.Parse(parser);
        
        n = new NameTyPairNode;
        n->m_nametypair = new NameTyPair(id,id_ty);
        parser->GetLogger()->D("Get a new NameTyPair");
        if(head){
            head->next=n;
            n->prev=head;
        }
        
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_TYPE){
            parser->GetScanner()->Back(&t);
            parser->GetLogger()->D("End TyDec::_ParseTyDec()");
            return _ParseTyDec(n,parser);
        }else{
            parser->GetLogger()->D("retrieve the head node of NameTyPairNode list");
            if(v!=kToken_EOT)
                parser->GetScanner()->Back(&t);
            p = n;
            q = n;
            while(p){
                q = p;
                p = p->prev;
            }
            return q;
        }
        return 0;
    }
    return 0;
}
Dec* VarDeclaration::Parse(Parser* parser){
    s32 v;
    Token t;
    Symbol* id,*type_id;
    Exp* init_exp;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_VAR){
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_ASSIGN){
            TIGER_ASSERT(v==kToken_ASSIGN,"Expected kToken_ASSIGN here, but %s provided",token_string((TokenType)v));
            init_exp=parser->ParseExp();
            parser->GetLogger()->D("Get a new VarDec");
            return new VarDec(id,0,init_exp);
        }
        if(v==kToken_COLON){
            v = parser->GetScanner()->Next(&t);
            TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
            type_id = new Symbol(t.u.name);
            v = parser->GetScanner()->Next(&t);
            TIGER_ASSERT(v==kToken_ASSIGN,"Expected kToken_ASSIGN here, but %s provided",token_string((TokenType)v));
            init_exp=parser->ParseExp();
            parser->GetLogger()->D("Get a new VarDec");
            return new VarDec(id,type_id,init_exp);
        }
        /* error */
        TIGER_ASSERT(0,"Wrong token %s here",token_string((TokenType)v));
        return 0;
    }
    /* error */
    TIGER_ASSERT(0,"Wrong token %s here",token_string((TokenType)v));
    return 0;
}
Dec* FunDeclaration::Parse(Parser* parser){
    s32 v;
    Token t;
    Symbol* id;
    Symbol* type_id;
    FieldList* fields;
    Exp* body_exp;
    FunDecNode* head;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_FUNCTION){
        parser->GetScanner()->Back(&t);
        head=_ParseFuncDec(0,parser);
        return new FunctionDec(new FunDecList(head));
    }
    return 0;
}

FunDecNode* FunDeclaration::_ParseFuncDec(FunDecNode* head,Parser* parser){
    s32 v;
    Token t;
    FunDecNode* n,*p,*q;
    Symbol* id;
    Symbol* type_id;
    FieldList* fields;
    Exp* body_exp;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_FUNCTION){
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_ID,"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_LPAR,"Expected kToken_LPAR here, but %s provided",token_string((TokenType)v));
        fields = _ParseTyFields(0,parser);
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT(v==kToken_RPAR,"Expected kToken_RPAR here, but %s provided",token_string((TokenType)v));
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_EQUAL){
            body_exp = parser->ParseExp();
            
            n = new FunDecNode;
            n->m_fundec = new FunDec(id,fields,0,body_exp);
            parser->GetLogger()->D("Get a new FunDecNode");
            if(head){
                head->next=n;
                n->prev=head;
            }
            
            v = parser->GetScanner()->Next(&t);
            if(v==kToken_FUNCTION){/* continue function declarations */
                parser->GetScanner()->Back(&t);
                return _ParseFuncDec(n,parser);
            }else{
                if(v!=kToken_EOT)
                    parser->GetScanner()->Back(&t);
                parser->GetLogger()->D("Retrieve the head node of the FunDecNode list");
                p = n;
                q = n;
                while(p){
                    q = p;
                    p = p->prev;
                }
                return q;
            }
        }
        if(v==kToken_COLON){
            v = parser->GetScanner()->Next(&t);
            TIGER_ASSERT((v==kToken_ID),"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
            type_id = new Symbol(t.u.name);
            v = parser->GetScanner()->Next(&t);
            TIGER_ASSERT((v==kToken_EQUAL),"Expected kToken_EQUAL here, but %s provided",token_string((TokenType)v));
            body_exp=parser->ParseExp();
            
            
            n = new FunDecNode;
            n->m_fundec = new FunDec(id,fields,type_id,body_exp);
            parser->GetLogger()->D("Get a new FunDecNode");
            if(head){
                head->next=n;
                n->prev=head;
            }
            v = parser->GetScanner()->Next(&t);
            if(v==kToken_FUNCTION){/* continue function declarations */
                parser->GetScanner()->Back(&t);
                return _ParseFuncDec(n,parser);
            }else{
                if(v!=kToken_EOT)
                    parser->GetScanner()->Back(&t);
                parser->GetLogger()->D("Retrieve the head node of the FunDecNode list");
                p = n;
                q = n;
                while(p){
                    q = p;
                    p = p->prev;
                }
                return q;
            }
        }
    }
    return 0;
}
FieldList* FunDeclaration::_ParseTyFields(FieldNode* head,Parser* parser){
    s32 v;
    Token t;
    Symbol* id,*type_id;
    FieldNode* afield;
    FieldNode* p,*q;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_RPAR){
        parser->GetScanner()->Back(&t);
        return 0;
    }
    if(v==kToken_ID){
        id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT((v==kToken_COLON),"Expected kToken_COLON here, but %s provided",token_string((TokenType)v));
        v = parser->GetScanner()->Next(&t);
        TIGER_ASSERT((v==kToken_ID),"Expected kToken_ID here, but %s provided",token_string((TokenType)v));
        type_id = new Symbol(t.u.name);
        v = parser->GetScanner()->Next(&t);
        
        afield = new FieldNode;
        afield->m_field = new Field(id,type_id);
        parser->GetLogger()->D("Get a new FieldNode");
        /* link it */
        if(head){
            head->next=afield;
            afield->prev=head;
        }
        
        if(v==kToken_COMMA){
            return _ParseTyFields(afield,parser);
        }
        if(v!=kToken_EOT)// v should be kToken_RBRA
            parser->GetScanner()->Back(&t);
            
        parser->GetLogger()->D("Retrieve the head node of the FieldNode list");
        /* get field head */
        p = afield;
        q = afield;
        while(p){
            q = p;
            p = p->prev;
        }
        return new FieldList(q);
    }
    TIGER_ASSERT(0,"Wrong token %s here",token_string((TokenType)v));
    return 0;
}

Dec* Declaration::Parse(Parser* parser){
    s32 v;
    Token t;
    v = parser->GetScanner()->Next(&t);
    if(v==kToken_TYPE){
        parser->GetLogger()->D("Begin  type declaration parsing");
        TyDec dec;
        parser->GetScanner()->Back(&t);
        return dec.Parse(parser);
    }
    if(v==kToken_VAR){
        parser->GetLogger()->D("Begin  var declaration parsing");
        VarDeclaration dec;
        parser->GetScanner()->Back(&t);
        return dec.Parse(parser);
    }
    if(v==kToken_FUNCTION){
        parser->GetLogger()->D("Begin  function declaration parsing");
        FunDeclaration dec;
        parser->GetScanner()->Back(&t);
        return dec.Parse(parser);
    }
    //std::cout<<token_string((TokenType)v)<<std::endl;
    if(v!=kToken_EOT)//it should be kToken_IN
        parser->GetScanner()->Back(&t);
        
    return 0;
}

}//namespace parser

}//namespace tiger
