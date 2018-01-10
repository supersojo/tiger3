/* Coding: ANSI */
#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "absyn.h"
#include "tiger_log.h"

/*
tiger syntax
-------------

program:    exp {  }
       ;

exp:lvalue {  }
   |NIL    {  }
   |LPAREN explist RPAREN {  }
   |LPAREN         RPAREN {}
   |INT {}
   |STRING {}
   |MINUS exp %prec UNARYMINUS {}
   |ID LPAREN RPAREN {}
   |ID LPAREN arglist RPAREN {}
   |exp PLUS exp {}
   |exp MINUS exp {}
   |exp TIMES exp {}
   |exp DIVIDE exp {}
   |exp EQ exp {}
   |exp NEQ exp {}
   |exp LT exp {}
   |exp LE exp {}
   |exp GT exp {}
   |exp GE exp {}
   |exp AND exp {}
   |exp OR exp {}
   |ID LBRACE RBRACE {}
   |ID LBRACE idlist RBRACE {}
   |ID LBRACK exp RBRACK OF exp {}
   |lvalue ASSIGN exp {}
   |IF exp THEN exp ELSE exp {}
   |IF exp THEN exp {}
   |WHILE exp DO exp {}
   |FOR ID ASSIGN exp TO exp DO exp {}
   |BREAK {}
   |LET decs IN END {}
   |LET decs IN explist END {}
   ;

lvalue: ID {}
      | lvalue DOT ID {}
      | lvalue LBRACK exp RBRACK {}
      ;

explist: exp {}
       | explist SEMICOLON exp {}
       ;

arglist:exp {}
       |exp COMMA arglist {}
       ;

idlist:ID EQ exp {}
      |ID EQ exp COMMA idlist {}
      ;

decs:dec {}
       |decs dec {}
       ;

dec:tydec {}
   |vardec {}
   |fundec {}
   ;

tydec:TYPE ID EQ ty {}
     ;

ty:ID {}
  |LBRACK tyfields RBRACK {}
  |ARRAY OF ID {}
  ;

tyfields:
        |notnulltyfields {}
        ;

notnulltyfields:ID COLON ID {}
               |ID COLON ID COMMA notnulltyfields {}
               ;

vardec:VAR ID ASSIGN exp {}
      |VAR ID COLON ID ASSIGN exp {}
      ;

fundec:FUNCTION ID LPAREN tyfields RPAREN EQ exp {}
      |FUNCTION ID LPAREN tyfields RPAREN COLON ID EQ exp {}
      ;
*/
namespace tiger{

namespace parser{ 

class Parser{
friend class Term;
public:
    Parser(scanner::SourceCodeStreamBase* stream);
   
    bool Parse(Exp** prog);
    scanner::Scanner* GetScanner(){return m_scanner;} 
    LoggerStdio* GetLogger(){return m_logger;}
    ~Parser();
    Exp* ParseExp(){return _ParseExp();}
    //Exp* ParseExpSeq();
private:
    Exp* _ParseExp();
    ExpNode* _ParseExpSeq(ExpNode* head);
    Exp* _ParseTerm();
    Var* _ParseLvalue(Var* head);
    Var* _ParseLvalueTerm();
    Var* _ParseLvalueRest(Var* var);
    DecList* _ParseDecs();
    ExpNode* _ParseParms(ExpNode* head);
    EFieldList* _ParseIdList();

    scanner::Scanner* m_scanner;
    LoggerStdio* m_logger;

};

/*
 * basic term with highest priority
 *
 * Here we use c++ entends to parse expression.
 * When need high performance, use c function table instead. 
*/
class Term{
public:
    virtual Exp* Parse(Parser* parser);
};

/* * or / */
class ExpMulDiv:public Term{
public:
    virtual Exp* Parse(Parser* parser);
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser);
};

/* + or - */
class ExpAddSub:public ExpMulDiv{
public:
    virtual Exp* Parse(Parser* parser);
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser);
};

/* < or > or <= or >= */
class ExpCompare:public ExpAddSub{
public:
    virtual Exp* Parse(Parser* parser);
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser);
};

/* = or <> */
class ExpEqualOrNot:public ExpCompare{
public:
    virtual Exp* Parse(Parser* parser);
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser);
};

/* & */
class ExpAnd:public ExpEqualOrNot{
public:
    virtual Exp* Parse(Parser* parser);
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser);
};

/* or */
class ExpOr:public ExpAnd{
public:
    virtual Exp* Parse(Parser* parser);
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser);
};

class IdList{
public:
    virtual EFieldList* Parse(Parser* parser);
private:
    EFieldNode* _ParseIdList(Parser* parser);
    EFieldNode* _ParseIdListRest(EFieldNode* head,Parser* parser);
};

class TyDeclaration{
public:
    virtual Ty* Parse(Parser* parser);
private:
    FieldList* _ParseTyFields(FieldNode* head,Parser* parser);
};

class TyDec{
public:
    virtual Dec* Parse(Parser* parser);
private:
    NameTyPairNode* _ParseTyDec(NameTyPairNode* head,Parser* parser);
};

/* var parsing */
class VarDeclaration{
public:
    virtual Dec* Parse(Parser* parser);
};

/* function parsing */
class FunDeclaration{
public:
    virtual Dec* Parse(Parser* parser);
private:
    FunDecNode* _ParseFuncDec(FunDecNode* head,Parser* parser);
    FieldList* _ParseTyFields(FieldNode* head,Parser* parser);
};

/* declaration parsing */
class Declaration{
public:
    virtual Dec* Parse(Parser* parser);
};


}//namespace parser

}//namespace tiger

#endif
