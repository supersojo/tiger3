/* Coding: ANSI */
#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

namespace tiger{

namespace parser{ 

class Parser{
friend class Term;
public:
    Parser(scanner::SourceCodeStreamBase* stream);
   
    bool Parse();
    scanner::Scanner* GetScanner(){return m_scanner;} 
    ~Parser();
    
private:
    void _ParseVar();
    void _ParseExp();
    void _ParseExpSeq();
    void _ParseTerm();
    void _ParseLvalue();
    void _ParseLvalueTerm();
    void _ParseLvalueRest();
    void _ParseParms();
    /* logical exp */
    


    scanner::Scanner* m_scanner;

};
/*
 * basic term with highest priority
 *
 * Here we use c++ entends to parse expression.
 * When need high performance, use c function table instead. 
*/
class Term{
public:
    virtual void Parse(Parser* parser);
};
/* * or / */
class ExpMulDiv:public Term{
public:
    virtual void Parse(Parser* parser){
        Term::Parse(parser);
        _ParseExpRest(parser);
    }
private:
    void _ParseExpRest(Parser* parser){
        s32 v;
        Token t;
        
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_MUL || v==kToken_DIV){
            Term::Parse(parser);
            _ParseExpRest(parser);
            return;
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
    }
};
/* + or - */
class ExpAddSub:public ExpMulDiv{
public:
    virtual void Parse(Parser* parser){
        ExpMulDiv::Parse(parser);
        _ParseExpRest(parser);
    }
private:
    void _ParseExpRest(Parser* parser){
        s32 v;
        Token t;
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_ADD || v==kToken_SUB){
            ExpMulDiv::Parse(parser);
            _ParseExpRest(parser);
            return;
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
    }
};
/* < or > or <= or >= */
class ExpCompare:public ExpAddSub{
public:
    virtual void Parse(Parser* parser){
        ExpAddSub::Parse(parser);
        _ParseExpRest(parser);
    }
private:
    void _ParseExpRest(Parser* parser){
        s32 v;
        Token t;
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_LT || v==kToken_GT || v==kToken_LE || v==kToken_GE){
            
            ExpAddSub::Parse(parser);
            _ParseExpRest(parser);
            return;
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
    }
};
/* = or <> */
class ExpEqualOrNot:public ExpCompare{
public:
    virtual void Parse(Parser* parser){
        ExpCompare::Parse(parser);
        _ParseExpRest(parser);
    }
private:
    void _ParseExpRest(Parser* parser){
        s32 v;
        Token t;
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_EQUAL || v==kToken_NOTEQUAL){
            std::cout<<"-<>-"<<std::endl;
            ExpCompare::Parse(parser);
            _ParseExpRest(parser);
            return;
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
    }
};
/* & */
class ExpAnd:public ExpEqualOrNot{
public:
    virtual void Parse(Parser* parser){
        ExpEqualOrNot::Parse(parser);
        _ParseExpRest(parser);
    }
private:
    void _ParseExpRest(Parser* parser){
        s32 v;
        Token t;
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_AND){
            
            ExpEqualOrNot::Parse(parser);
            _ParseExpRest(parser);
            return;
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
    }
};
/* or */
class ExpOr:public ExpAnd{
public:
    virtual void Parse(Parser* parser){
        ExpAnd::Parse(parser);
        _ParseExpRest(parser);
    }
private:
    void _ParseExpRest(Parser* parser){
        s32 v;
        Token t;
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_OR){
            
            ExpAnd::Parse(parser);
            _ParseExpRest(parser);
            return;
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
    }
};


}//namespace parser

}//namespace tiger

#endif
