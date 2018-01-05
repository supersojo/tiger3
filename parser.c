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
void Parser::ParseExpSeq()
{
	_ParseExpSeq();
}
void Parser::_ParseExpSeq()
{
    s32 v;
    Token t;
    _ParseExp();
    
    v = m_scanner->Next(&t);
    if(v==kToken_SEMICOLON){
        /* expseq -> exp; expseq */
        _ParseExpSeq();
    }else{
        m_scanner->Back(&t);
    }
}
void Parser::_ParseLvalueTerm()
{
    s32 v;
    Token t;
    
    v = m_scanner->Next(&t);
    if(v==kToken_ID){
        return;
    }
}
void Parser::_ParseLvalueRest(s32 * flag)
{
    s32 v,v1;
    Token t,t1;
    /* lvalue' -> . lvalue' */
    v = m_scanner->Next(&t);
    if(v==kToken_DOT){
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_ID);
        _ParseLvalueRest(flag);
        return;
    }
	
    /* lvalue' -> [ exp ] lvalue' */
    if(v==kToken_LSQB){
        _ParseExp();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RSQB);
        *flag = *flag + 1;
        _ParseLvalueRest(flag);
        return;
    }
    
    /* id [ exp ] of exp */
    if(v==kToken_OF){
        if(*flag==1){
            /* ok */
            _ParseExp();
            return;
        }else{
            assert(1==0);
        }
    }
    /* lvalue'-> empty */
    if(v!=kToken_EOT)
        m_scanner->Back(&t);
    
}
void Parser::_ParseLvalue()
{
    s32 flag = 0;/* of expression */
    _ParseLvalueTerm();
    _ParseLvalueRest(&flag);
}

void Parser::_ParseParms()
{
    s32 v,v1;
    Token t,t1;
    v = m_scanner->Next(&t);
    /* parms -> empty */
    if(v==kToken_RPAR){
        m_scanner->Back(&t);
        return;
    }
    /* parms -> exp */
    /* parms -> exp , parms */
    if(v!=kToken_EOT){
        m_scanner->Back(&t);
        _ParseExp();
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_COMMA){
            _ParseParms();
            return;
        }
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
    }
}
void Parser::_ParseIdList()
{
    IdList idList;
    idList.Parse(this);
    
}
void Parser::_ParseDecs()
{
    s32 v;
    Token t;
    do{
        v = m_scanner->Next(&t);
        if(v==kToken_IN){
            m_scanner->Back(&t);
            return;
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        Dec dec;
        dec.Parse(this);
    }while(1);
}
void Parser::_ParseTerm()
{
    s32 v,v1,v2;
    Token t,t1,t2;
    
    v = m_scanner->Next(&t);
    /* exp -> num */
    if(v==kToken_NUM){
        return;
    }
    /* exp -> string */
    if(v==kToken_STR){
        return;
    }
    /* exp -> nil */
    if(v==kToken_NIL){
        return;
    }
    /* exp -> if exp then exp else exp or exp -> if exp then exp */
    if(v==kToken_IF){
        _ParseExp();
        v = m_scanner->Next(&t);
        assert(v==kToken_THEN);
        _ParseExp();
        v = m_scanner->Next(&t);
        if(v==kToken_ELSE){
            _ParseExp();
            return;
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        return;
    }
    
    /* exp -> while exp do exp */
    if(v==kToken_WHILE){
        _ParseExp();
        v = m_scanner->Next(&t);
        assert(v==kToken_DO);
        _ParseExp();
        return;
    }
    /* exp -> for id := exp to exp do exp */
    if(v==kToken_FOR){
        v = m_scanner->Next(&t);
        assert(v==kToken_ID);
        v = m_scanner->Next(&t);
        assert(v==kToken_ASSIGN);
        _ParseExp();
        v = m_scanner->Next(&t);
        assert(v==kToken_TO);
        _ParseExp();
        v = m_scanner->Next(&t);
        assert(v==kToken_DO);
        _ParseExp();
        return;
    }
    /* exp -> let decs in end or let decs in explist end */
    if(v==kToken_LET){
        _ParseDecs();
        v = m_scanner->Next(&t);
        assert(v==kToken_IN);
	v1 = m_scanner->Next(&t1);
	if(v1==kToken_END){
		return;	
	}
	if(v1!=kToken_EOT)
		m_scanner->Back(&t1);
	_ParseExpSeq();
        v = m_scanner->Next(&t);
        if(v==kToken_END){
            return;
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        return;
    } 
    /* exp -> break */
    if(v==kToken_BREAK){
        return;
    }
    if(v==kToken_LPAR){
        v1 = m_scanner->Next(&t1);
        /* exp -> () */
        if(v1==kToken_RPAR){
            return;
        }
        /* exp -> ( expseq ) */
        m_scanner->Back(&t1);
        _ParseExpSeq();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RPAR);
        return;
    }
    /* exp -> - exp */
    if(v==kToken_SUB){
        _ParseExp();
        return;
    }
    
    if(v==kToken_ID){
        /*  exp -> id () or exp -> id (parms)*/
        v1 = m_scanner->Next(&t1);
        if(v1==kToken_LPAR){
            _ParseParms();
            v2 = m_scanner->Next(&t2);
            assert(v2==kToken_RPAR);
            return;
        }
        /*  exp -> id {} or exp -> id{id=exp{,id=exp}}*/
        if(v1==kToken_LBRA){
            _ParseIdList();
            v2 = m_scanner->Next(&t2);
            assert(v2==kToken_RBRA);
            return;
        }
        
        if(v1!=kToken_EOT)
            m_scanner->Back(&t1);
        
        /* exp -> lvalue */
        m_scanner->Back(&t);
        _ParseLvalue();
        v = m_scanner->Next(&t);
        
        /* exp -> lvalue := exp */
        if(v==kToken_ASSIGN){
            _ParseExp();
            return;
        }
        if(v!=kToken_EOT)
            m_scanner->Back(&t);
        return;
    }
    
}

void Parser::_ParseExp()
{
    s32 v;
    Token t;
    
    ExpOr exp;
    exp.Parse(this);

}
Parser::~Parser(){
    delete m_scanner;
}

void Term::Parse(Parser* parser)
{
    parser->_ParseTerm();
}


}//namespace parser

}//namespace tiger
