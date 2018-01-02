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
    
    ~Parser();
    
private:
    bool _Match(s32 v);
    void _ParseVar();
    void _ParseExp();
    void _ParseExpSeq();
    void _ParseTerm();
    void _ParseLvalue();
    void _ParseLvalueTerm();
    void _ParseLvalueRest();
    /* logical exp */
    


    scanner::Scanner* m_scanner;

};
class Term{
public:
    Term(){m_parser=0;}
    Term(Parser* parser){m_parser=parser;}
    virtual void Parse();
    Parser* Parser(){return m_parser;}
private:
    Parser* m_parser;
};
class ExpMulDIv:public Term{
public:
    virtual void Parse(){
        Term::Parse();
        _ParseExpRest();
    }
private:
    void _ParseExpRest(){
        if(Parser()()->_Match(kToken_MUL)){
            Term::Parse();
            _ParseExpRest();
        }else if(Parser()()->_Match(kToken_DIV)){
            Term::Parse();
            _ParseExpRest();
        }else{
            
        }
    }
};


}//namespace parser

}//namespace tiger

#endif
