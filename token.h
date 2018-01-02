/* Coding: ANSI */
#ifndef TOKEN_H
#define TOKEN_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tiger_type.h"

namespace tiger{

/* 
 Token defination:
   a basic lexical unit
   
 type:
   identifier
   number
   string
   keyword
   ...
**/
typedef enum {
    kToken_base=100,
    kToken_ID,
    kToken_NUM,
    kToken_STR,
    kToken_LPAR,/* ( */
    kToken_RPAR,/* ) */
    kToken_LBRA,/* { */
    kToken_RBRA,/* } */
    kToken_LSQB,/* [ */
    kToken_RSQB,/* ] */
    
    /* add more tokens */
    kToken_ADD,/* + */
    kToken_SUB,/* - */
    kToken_MUL,/* * */
    kToken_DIV,/* / */
    kToken_ASSIGN,/* := */
    kToken_COMMA,/* , */
    kToken_COLON,/* : */
    kToken_SEMICOLON,/* ; */
    kToken_DOT,/* . */
    
    kToken_EQUAL,/* = */
    kToken_LT, /* < */
    kToken_GT, /* > */
    kToken_LE,/* <= */
    kToken_GE,/* >= */
    
    kToken_AND, /* & */
    kToken_OR,/*| */
    kToken_NOT, /* ! */
    
    
    /* keywords */
    kToken_TYPE,/* type */
    kToken_ARRAY,/* array */
    kToken_OF,/* of */
    kToken_VAR,/* var */
    kToken_FOR,/* for */
    kToken_TO,/* to */
    kToken_WHILE,/* while */
    kToken_DO,/* do */
    kToken_IF, /* if */
    kToken_THEN,/* then */
    kToken_ELSE,/* else */
    kToken_FUNCTION, /* function */
    kToken_END,/* end */
    kToken_LET,/* let */
    kToken_IN,/* in */
    kToken_NIL,/* nil */
    
    
    kToken_EOT,/* the end of token stream */
    kToken_Unknown,
    kToken_MAX,
}TokenType;

/* id or string max length limit */
#define TOKEN_MAX_LEN 1024

extern char * kTokenString[];

inline char* token_string(TokenType k)
{
    assert(k>=kToken_ID && k<kToken_MAX);
    return kTokenString[k-kToken_base-1];
}

inline s32 token_is_keyword(char* str)
{
    s32 i,start,end;
    start = kToken_TYPE;
    end = kToken_EOT-1;
    for(i=start;i<=end;i++)
        if(strcmp(token_string((TokenType)i),str)==0)
            return 1;
    return 0;
}
inline s32 keyword_type(char* str)
{
    s32 i,start,end;
    
    start = kToken_TYPE;
    end = kToken_EOT-1;
    
    assert(token_is_keyword(str));
    
    for(i=start;i<=end;i++)
        if(strcmp(token_string((TokenType)i),str)==0)
            return i;
    
    assert(i>=start && i<=end);
    
    /* not reached */
    return kToken_EOT;
}
struct Token{
    TokenType kind;
    union{
        char* name;
        char* sval;
        s32   ival;
    }u;
    s32 lineno;
    s32 pos;
    s32 len;/* token's string length */
    
    Token(){
        kind = kToken_Unknown;
    }
    void Clear(){
        if(kind==kToken_ID)
            free(u.name);
        if(kind==kToken_STR)
            free(u.sval);
    }
    ~Token(){
        Clear();
    }
};

} // namespace tiger

#endif
