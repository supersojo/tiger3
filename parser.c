#include "parser.h"

namespace tiger{

namespace parser{

Parser::Parser(scanner::SourceCodeStreamBase* stream){
    m_scanner = new scanner::Scanner(stream);
    m_logger = new LoggerStdio;
    m_logger->SetLevel(LoggerBase::kLogger_Level_Error);
}

bool Parser::Parse()
{
    s32 v;
    Token t;
    Exp* exp;
    
    m_logger->D("Begin Parse()");
    exp = _ParseExp();
    m_logger->D("End Parse()");
    v = m_scanner->Next(&t);
    m_logger->D("%s",token_string((TokenType)v));
    assert(v==kToken_EOT);
    m_logger->D("parse ok");
    
    return true;
}

void Parser::_ParseVar()
{
    
}
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
ExpNode* Parser::_ParseExpSeq(ExpNode* head)
{
    s32 v;
    Token t;
    Exp* exp;
    m_logger->D("Begin _ParseExpSeq()");
    ExpNode* anode,*p,*q;
    exp = _ParseExp();
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
    assert(1==0);
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
        assert(v1==kToken_ID);
        m_logger->D("Get a new field var with t1.u.name");
        return _ParseLvalueRest(new FieldVar(var,new Symbol(t1.u.name)));
    }
	
    /* lvalue' -> [ exp ] lvalue' */
    if(v==kToken_LSQB){
        Exp* exp;
        exp = _ParseExp();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RSQB);
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
    assert(1==0);
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
    do{
        v = m_scanner->Next(&t);
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
    
    assert(1==0);
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
        assert(v==kToken_THEN);
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
        assert(v==kToken_DO);
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
        assert(v==kToken_ID);
        id = new Symbol(t.u.name);
        v = m_scanner->Next(&t);
        assert(v==kToken_ASSIGN);
        lo_exp=_ParseExp();
        v = m_scanner->Next(&t);
        assert(v==kToken_TO);
        hi_exp=_ParseExp();
        v = m_scanner->Next(&t);
        assert(v==kToken_DO);
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
        assert(v==kToken_IN);
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_END){
            
            return new LetExp(declist,0);	
        }
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
        head = _ParseExpSeq(0);
        v = m_scanner->Next(&t);
        if(v==kToken_END){
            return new LetExp(declist,new SeqExp(new ExpList(head)));	
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        assert(1==0);
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
        assert(v1==kToken_RPAR);
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
            assert(v2==kToken_RPAR);
            m_logger->D("Get a new CallExp");
            return new CallExp(new Symbol(t.u.name),new ExpList(head));
        }
        /*  exp -> id {} or exp -> id{id=exp{,id=exp}}*/
        if(v1==kToken_LBRA){
            EFieldList* efields;
            efields = _ParseIdList();
            v2 = m_scanner->Next(&t2);
            assert(v2==kToken_RBRA);
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
            assert(v1==kToken_RSQB);
            v1 = m_scanner->Next(&t1);
            if(v1==kToken_OF){
                exp2 = _ParseExp();
                m_logger->D("Get a new ArrayExp");
                return new ArrayExp(id,exp1,exp2);
            }
            m_logger->D("Get a new subscript var");
            var1 = new SubscriptVar(new SimpleVar(id),exp1);
            delete id;
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
    /* should not reach */
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


}//namespace parser

}//namespace tiger
