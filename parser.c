#include "parser.h"

namespace tiger{

namespace parser{

Parser::Parser(scanner::SourceCodeStreamBase* stream){
    m_scanner = new scanner::Scanner(stream);
}

bool Parser::Parse()
{
    s32 v;
    Token t;
    
    _ParseExp();
    
    v = m_scanner->Next(&t);
    std::cout<<token_string((TokenType)v)<<std::endl;
    assert(v==kToken_EOT);
    
    
    return true;
}

void Parser::_ParseVar()
{
    
}
Exp* Parser::ParseExpSeq()
{
    ExpNode* head;
	head=_ParseExpSeq(0);
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
    ExpNode* anode,*p,*q;
    exp = _ParseExp();
    anode = new ExpNode;
    anode->m_exp = exp;
    if(head){
        head->next=anode;
        anode->prev=head;
    }
    v = m_scanner->Next(&t);
    if(v==kToken_SEMICOLON){
        /* expseq -> exp; expseq */
        return _ParseExpSeq(anode);
    }else{
        m_scanner->Back(&t);
        p = anode;
        q = anode;
        while(p){
            q = p;
            p = p->prev;
        }
        return q;
    }
}
Var* Parser::_ParseLvalueTerm()
{
    s32 v;
    Token t;
    
    v = m_scanner->Next(&t);
    if(v==kToken_ID){
        return new SimpleVar(new Symbol(t.u.name));
    }
    assert(1==0);
    return 0;
}
Var* Parser::_ParseLvalueRest(Var* var,s32 * flag)
{
    s32 v,v1;
    Token t,t1;
    /* lvalue' -> . lvalue' */
    v = m_scanner->Next(&t);
    if(v==kToken_DOT){
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_ID);
        return _ParseLvalueRest(new FieldVar(var,new Symbol(t1.u.name)),flag);
    }
	
    /* lvalue' -> [ exp ] lvalue' */
    if(v==kToken_LSQB){
        Exp* exp;
        exp = _ParseExp();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RSQB);
        *flag = *flag + 1;
        return _ParseLvalueRest(new SubscriptVar(var,exp),flag);
    }
    
    /* id [ exp ] of exp */
    if(v==kToken_OF){
        Exp* exp;
        if(*flag==1){
            /* ok */
            exp = _ParseExp();
            return new SubscriptVar(var,exp);
        }else{
            assert(1==0);
        }
    }
    /* lvalue'-> empty */
    if(v!=kToken_EOT)
        m_scanner->Back(&t);
   return var; 
}
Var* Parser::_ParseLvalue()
{
    s32 flag = 0;/* of expression */
    Var * var;
    var = _ParseLvalueTerm();
    assert(var!=0);
    return _ParseLvalueRest(var,&flag);
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
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_COMMA){
            return _ParseParms(anode);
        }
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
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
    do{
        v = m_scanner->Next(&t);
        if(v==kToken_IN){
            m_scanner->Back(&t);
            if(head)
                return new DecList(head);
            else
                return 0;
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        Declaration dec;
        adec = dec.Parse(this);
        if(adec){
            anode = new DecNode;
            anode->m_dec = adec;
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
        return new IntExp(t.u.ival);
    }
    /* exp -> string */
    if(v==kToken_STR){
        return new StringExp(t.u.sval);
    }
    /* exp -> nil */
    if(v==kToken_NIL){
        return new NilExp();
    }
    /* exp -> if exp then exp else exp or exp -> if exp then exp */
    if(v==kToken_IF){
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
        return new BreakExp();
    }
    if(v==kToken_LPAR){
        ExpNode* head;
        v1 = m_scanner->Next(&t1);
        /* exp -> () */
        if(v1==kToken_RPAR){
            return 0;
        }
        /* exp -> ( expseq ) */
        m_scanner->Back(&t1);
        head=_ParseExpSeq(0);
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RPAR);
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
            return new CallExp(new Symbol(t.u.name),new ExpList(head));
        }
        /*  exp -> id {} or exp -> id{id=exp{,id=exp}}*/
        if(v1==kToken_LBRA){
            EFieldList* efields;
            efields = _ParseIdList();
            v2 = m_scanner->Next(&t2);
            assert(v2==kToken_RBRA);
            return new RecordExp(new Symbol(t.u.name),efields);
        }
        
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
        
        /* exp -> lvalue */
        Var* var;
        m_scanner->Back(&t);
        var = _ParseLvalue();
        v = m_scanner->Next(&t);
        
        /* exp -> lvalue := exp */
        if(v==kToken_ASSIGN){
            Exp* exp;
            exp = _ParseExp();
            return new AssignExp(var,exp);
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        return 0;
    }
    
}

Exp* Parser::_ParseExp()
{
    s32 v;
    Token t;
    
    ExpOr exp;
    return exp.Parse(this);

}
Parser::~Parser(){
    delete m_scanner;
}

Exp* Term::Parse(Parser* parser)
{
    parser->_ParseTerm();
}


}//namespace parser

}//namespace tiger
