/* Coding: ANSI */
#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "absyn.h"

namespace tiger{

namespace parser{ 

class Parser{
friend class Term;
public:
    Parser(scanner::SourceCodeStreamBase* stream);
   
    bool Parse();
    scanner::Scanner* GetScanner(){return m_scanner;} 
    ~Parser();
    Exp* ParseExp(){return _ParseExp();}
    Exp* ParseExpSeq();
private:
    void _ParseVar();
    Exp* _ParseExp();
    ExpNode* _ParseExpSeq(ExpNode* head);
    Exp* _ParseTerm();
    Var* _ParseLvalue();
    Var* _ParseLvalueTerm();
    Var* _ParseLvalueRest(Var* var,s32* flag);
    DecList* _ParseDecs();
    ExpNode* _ParseParms(ExpNode* head);
    EFieldList* _ParseIdList();
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
            rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Mul),left,right),parser);
            return rest;
        }
        if(v==kToken_DIV){
            right = Term::Parse(parser);
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
        if(v==kToken_ADD || v==kToken_SUB){
            right=ExpMulDiv::Parse(parser);
            rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Add),left,right),parser);
            return rest;
        }
        if(v==kToken_SUB){
            right=ExpMulDiv::Parse(parser);
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
            rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Lt),left,right),parser);
            return rest;
        }
        if(v==kToken_GT){
            right = ExpAddSub::Parse(parser);
            rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Gt),left,right),parser);
            return rest;
        }
        if(v==kToken_LE){
            right = ExpAddSub::Parse(parser);
            rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Le),left,right),parser);
            return rest;
        }
        if(v==kToken_GE){
            right = ExpAddSub::Parse(parser);
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
            rest=_ParseExpRest(new OpExp(new Oper(Oper::kOper_Eq),left,right),parser);
            return rest;
        }
        if(v==kToken_NOTEQUAL){
            right=ExpCompare::Parse(parser);
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
            
            return n;
        }
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
        p = anode;
        q = anode;
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
        if(v==kToken_ID)
            return new NameTy(new Symbol(t.u.name));
        if(v==kToken_LBRA){
            fields = _ParseTyFields(0,parser);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_RBRA);
            return new RecordTy(fields);
        }
        if(v==kToken_ARRAY){
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_OF);
            v = parser->GetScanner()->Next(&t);
            assert(v==kToken_ID);
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
            /* link it */
            if(head){
                head->next=afield;
                afield->prev=head;
            }
            
            if(v==kToken_COMMA){
                return _ParseTyFields(0,parser);
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
            return new FieldList(q);
        }
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
        v = parser->GetScanner()->Next(&t);
        if(v==kToken_TYPE){
            parser->GetScanner()->Back(&t);
            head = _ParseTyDec(0,parser);
            return new TypeDec(new NameTyPairList(head));
        }
    }
private:
    NameTyPairNode* _ParseTyDec(NameTyPairNode* head,Parser* parser){
        s32 v;
        Token t;
        Symbol* id;
        Ty* id_ty;
        NameTyPairNode* n,*p,*q;
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
            
            if(head){
                head->next=n;
                n->prev=head;
            }
            
            v = parser->GetScanner()->Next(&t);
            if(v==kToken_TYPE){
                parser->GetScanner()->Back(&t);
                return _ParseTyDec(n,parser);
            }else{
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
            
        }
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
                return new VarDec(id,0,init_exp);
            }
            if(v==kToken_COLON){
                v = parser->GetScanner()->Next(&t);
                assert(v==kToken_ID);
                type_id = new Symbol(t.u.name);
                v = parser->GetScanner()->Next(&t);
                assert(v==kToken_ASSIGN);
                init_exp=parser->ParseExp();
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
            TyDec dec;
            parser->GetScanner()->Back(&t);
            return dec.Parse(parser);
        }
        if(v==kToken_VAR){
            VarDeclaration dec;
            parser->GetScanner()->Back(&t);
            return dec.Parse(parser);
        }
        if(v==kToken_FUNCTION){
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
