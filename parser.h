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
   
    bool Parse();
    scanner::Scanner* GetScanner(){return m_scanner;} 
    LoggerStdio* GetLogger(){return m_logger;}
    ~Parser();
    Exp* ParseExp(){return _ParseExp();}
    Exp* ParseExpSeq();
private:
    void _ParseVar();
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
    virtual Exp* Parse(Parser* parser){
        Exp* left;
        Exp* rest;
        
        left = Term::Parse(parser);
        rest = _ParseExpRest(left,parser);
        if(rest)
            return rest;
        else
            return left;
    }
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser){
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
        
        return 0;
    }
};
/* + or - */
class ExpAddSub:public ExpMulDiv{
public:
    virtual Exp* Parse(Parser* parser){
        Exp* left;
        Exp* rest;
        left=ExpMulDiv::Parse(parser);
        rest=_ParseExpRest(left,parser);
        if(rest)
            return rest;
        else
            return left;
    }
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser){
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
        
        return 0;
    }
};
/* < or > or <= or >= */
class ExpCompare:public ExpAddSub{
public:
    virtual Exp* Parse(Parser* parser){
        Exp* left;
        Exp* rest;
        left = ExpAddSub::Parse(parser);
        rest = _ParseExpRest(left,parser);
        if(rest)
            return rest;
        else
            return left;
    }
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser){
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
        return 0;
    }
};
/* = or <> */
class ExpEqualOrNot:public ExpCompare{
public:
    virtual Exp* Parse(Parser* parser){
        Exp* left,*rest;
        left=ExpCompare::Parse(parser);
        rest=_ParseExpRest(left,parser);
        if(rest)
            return rest;
        else
            return left;
    }
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser){
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
        return 0;
    }
};
/* & */
class ExpAnd:public ExpEqualOrNot{
public:
    virtual Exp* Parse(Parser* parser){
        Exp* left,*rest;
        left=ExpEqualOrNot::Parse(parser);
        rest=_ParseExpRest(left,parser);
        if(rest)
            return rest;
        else
            return left;
    }
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser){
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
        return 0;
    }
};
/* or */
class ExpOr:public ExpAnd{
public:
    virtual Exp* Parse(Parser* parser){
        Exp* left,*rest;
        left=ExpAnd::Parse(parser);
        rest=_ParseExpRest(left,parser);
        if(rest)
            return rest;
        else
            return left;
    }
private:
    Exp* _ParseExpRest(Exp* left,Parser* parser){
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
        return 0;
    }
};
class IdList{
public:
    virtual EFieldList* Parse(Parser* parser){
        EFieldNode* head,*rest;
        head = _ParseIdList(parser);
        rest = _ParseIdListRest(head,parser);
        if(rest)
            return new EFieldList(rest);
        else
            return new EFieldList(head);
    }
private:
    EFieldNode* _ParseIdList(Parser* parser){
        s32 v,v1,v2;
        Token t,t1,t2;
        EFieldNode* n;
        v = parser->GetScanner()->Next(&t);
        /* idlist->empty */
        if(v==kToken_RBRA){
            parser->GetScanner()->Back(&t);
            return 0;
        }
        assert(v==kToken_ID);
        if(v==kToken_ID){
            Exp* exp;
            v1 = parser->GetScanner()->Next(&t1);
            assert(v1==kToken_EQUAL);
            if(v1==kToken_EQUAL){
                exp = parser->ParseExp();
            }
            n = new EFieldNode;
            n->m_efield=new EField(new Symbol(t.u.name),exp);
            parser->GetLogger()->D("Get a new EFieldNode");
            return n;
        }
        return 0;
    }
    EFieldNode* _ParseIdListRest(EFieldNode* head,Parser* parser){
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
};
class TyDeclaration{
public:
    virtual Ty* Parse(Parser* parser){
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
            assert(v==kToken_RBRA);
            parser->GetLogger()->D("Get a new RecordTy");
            return new RecordTy(fields);
        }
        if(v==kToken_ARRAY){
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_OF);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_ID);
            parser->GetLogger()->D("Get a new ArrayTy");
            return new ArrayTy(new Symbol(t.u.name));
        }
        if(v!=kToken_EOT)
            parser->GetScanner()->Back(&t);
        
        return 0;
    }
private:
    FieldList* _ParseTyFields(FieldNode* head,Parser* parser){
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
            assert(v==kToken_COLON);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_ID);
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
};
class TyDec{
public:
    virtual Dec* Parse(Parser* parser){
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
private:
    NameTyPairNode* _ParseTyDec(NameTyPairNode* head,Parser* parser){
        s32 v;
        Token t;
        Symbol* id;
        Ty* id_ty;
        NameTyPairNode* n,*p,*q;
        parser->GetLogger()->D("Begin TyDec::_ParseTyDec()");
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_TYPE){
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_ID);
            id = new Symbol(t.u.name);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_EQUAL);
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
};
class VarDeclaration{
public:
    virtual Dec* Parse(Parser* parser){
        s32 v;
        Token t;
        Symbol* id,*type_id;
        Exp* init_exp;
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_VAR){
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_ID);
            id = new Symbol(t.u.name);
            v = parser->GetScanner()->Next(&t);
            if(v==kToken_ASSIGN){
                assert(v==kToken_ASSIGN);
                init_exp=parser->ParseExp();
                parser->GetLogger()->D("Get a new VarDec");
                return new VarDec(id,0,init_exp);
            }
            if(v==kToken_COLON){
                v = parser->GetScanner()->Next(&t);
                assert(v==kToken_ID);
                type_id = new Symbol(t.u.name);
                v = parser->GetScanner()->Next(&t);
                assert(v==kToken_ASSIGN);
                init_exp=parser->ParseExp();
                parser->GetLogger()->D("Get a new VarDec");
                return new VarDec(id,type_id,init_exp);
            }
            /* error */
            assert(1==0);
            return 0;
        }
        /* error */
        assert(1==0);
        return 0;
    }
};
class FunDeclaration{
public:
    virtual Dec* Parse(Parser* parser){
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
private:
    FunDecNode* _ParseFuncDec(FunDecNode* head,Parser* parser){
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
            assert(v==kToken_ID);
            id = new Symbol(t.u.name);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_LPAR); 
            fields = _ParseTyFields(0,parser);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_RPAR); 
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
                assert(v==kToken_ID);
                type_id = new Symbol(t.u.name);
                v = parser->GetScanner()->Next(&t);
                assert(v==kToken_EQUAL);
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
    FieldList* _ParseTyFields(FieldNode* head,Parser* parser){
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
            assert(v==kToken_COLON);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_ID);
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
        assert(1==0);
        return 0;
    }
};
class Declaration{
public:
    virtual Dec* Parse(Parser* parser){
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
};
}//namespace parser

}//namespace tiger

#endif
