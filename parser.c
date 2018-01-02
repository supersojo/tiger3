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
    assert(v==kToken_EOT);
    
    
    return true;
}

void Parser::_ParseVar()
{
    
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
void Parser::_ParseLvalueRest()
{
    s32 v,v1;
    Token t,t1;
    
    /* lvalue' -> . lvalue' */
    v = m_scanner->Next(&t);
    if(v==kToken_DOT){
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_ID);
        _ParseLvalueRest();
    }
    /* lvalue' -> [ exp ] lvalue' */
    if(v==kToken_LSQB){
        _ParseExp();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RSQB);
        _ParseLvalueRest();
    }
    /* lvalue'-> empty */
    m_scanner->Back(&t);
    
}
void Parser::_ParseLvalue()
{

    
    _ParseLvalueTerm();
    _ParseLvalueRest();
    
}
bool Parser::_Match(s32 v)
{
    bool result=false;
    s32 v1;
    Token t;
    v1 = m_scanner->Next(&t);
    result = (v1==v);
    if(!result){
        m_scanner->Back(&t);
    }
    return result;
}
void Parser::_ParseTerm()
{
    s32 v,v1;
    Token t,t1;
    
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
    }
    /* exp -> lvalue */
    if(v==kToken_ID){
        m_scanner->Back(&t);
        _ParseLvalue();
        return;
    }
    
}

void Parser::_ParseExp()
{

}
Parser::~Parser(){
    delete m_scanner;
}

void Term::Parse()
{
    m_parser->_ParseTerm();
}


}//namespace parser

}//namespace tiger