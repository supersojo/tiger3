#include <string.h>
#include <stdio.h>
#include <iostream>

#include "scanner.h"

namespace tiger {
    
namespace scanner {

StringSourceCodeStream::StringSourceCodeStream(char* source)
{
    m_string = source;
    m_pos = 0;
    m_len = strlen(source);
    m_off = 0;/* always start from 0 */
    m_lineno = 1;/* always start from 1 */
    m_line_info = 0;/* must initialize first */
}
s32 StringSourceCodeStream::Next()
{
    s32 ret = *(m_string + m_pos);
    if(m_pos<=(m_len-1)){
        m_pos++;
        m_off++;
        return ret;
    }        
    else
        return kSourceCodeStream_EOS;
}
void StringSourceCodeStream::Back(s32 n)
{
    LineInfo* lineinfo;
    /*
    when we get end of stream, the m_pos == m_len
    0<=(m_pos-n)<=m_len
    */
    assert(n<=m_pos &&n>=(m_pos-m_len+0));
    m_pos = m_pos-n;
    m_off = m_off-n;
    while(m_off<0){
        m_lineno = m_lineno-1;
        //m_off = m_off_prev;
        assert(m_line_info);
        m_off = + m_line_info->line_size;
        lineinfo = m_line_info;
        
        m_line_info = m_line_info->next;
        m_line_info->prev = 0;
        delete lineinfo;
    }
}
s32 StringSourceCodeStream::Pos()
{
    return m_off;
}
s32 StringSourceCodeStream::AbsPos()
{
    return m_pos;
}
/*
 Note: 
 open file with flag "rb", which will open file in binary mode.
 Or else, when "\r\n" occur, fread only return "\n" on windows platform.
*/
FileSourceCodeStream::FileSourceCodeStream(char* file)
{
    FILE *fp = fopen(file,"rb");
    
    assert(fp!=0);
    
    m_file = fp;
    
    fseek(m_file,0,SEEK_END);
    m_len = ftell(m_file);
    
    fseek(m_file,0,SEEK_SET);
    
    m_off = 0;
    m_lineno = 1;/* always start from 1 */
    m_line_info = 0;
    
}
FileSourceCodeStream::~FileSourceCodeStream()
{
    fclose(m_file);
    
    FreeLineInfo();
}
s32 FileSourceCodeStream::Next()
{
    s32 v = 0;
    s32 ret = fread(&v,1,1,m_file);
    if(ret==0)/* read error or eof */
        return kSourceCodeStream_EOS;
    else{
        m_off++;
        return v;
    }
}
void FileSourceCodeStream::Back(s32 n)
{
    s32 pos = ftell(m_file);
    LineInfo* lineinfo;
    
    /*
    when we get end of stream, the m_pos == m_len
    0<=(m_pos-n)<=m_len
    */
    assert(n<=pos &&n>=(pos-m_len+0));
    
    pos = pos-n;
    m_off = m_off - n;
    
    while(m_off<0){
        m_lineno = m_lineno-1;
        //m_off = m_off_prev;
        assert(m_line_info);
        m_off = + m_line_info->line_size;
        lineinfo = m_line_info;
        
        m_line_info = m_line_info->next;
        m_line_info->prev = 0;
        delete lineinfo;
    }
    /* update file position */
    fseek(m_file,pos,SEEK_SET);
}
s32 FileSourceCodeStream::Pos()
{
    return m_off;
}
s32 FileSourceCodeStream::AbsPos()
{
    return ftell(m_file);
}
Scanner::Scanner(SourceCodeStreamBase* stream)
{
    m_stream = stream;
    
    Init();
}
void Scanner::Init()
{

}

void Scanner::SkipSpace()
{
    s32 v;
    do{
        v = m_stream->Next();
    }while((char)v==' ' ||
           (char)v=='\t');
    if(v!=kSourceCodeStream_EOS){
        m_stream->Back(1);
    }
}

bool Scanner::SkipComment()
{
    enum{
        kComment_Begin,
        kComment_End,
        kComment_Invalid
    };
    s32 flag = kComment_Invalid;
    s32 v;
    s32 off = 0;
    v = m_stream->Next();
    if(v!=kSourceCodeStream_EOS)/* if we occur the eof, it will back to normal char, so it's wrong action, just back only good chars */
        off++;
    if((char)v=='/'){
        v = m_stream->Next();
        if(v!=kSourceCodeStream_EOS)
            off++;
        if((char)v=='*'){
            flag = kComment_Begin;
        }
        
    }
    if(flag==kComment_Begin){
        
        do{
            v = m_stream->Next();
            if(v!=kSourceCodeStream_EOS)
                off++;
            if((char)v=='*'){
                v = m_stream->Next();
                if(v!=kSourceCodeStream_EOS)
                    off++;
                if((char)v=='/')
                    flag = kComment_End;
            }
            /* new line? */
            if((char)v=='\r'){
                v = m_stream->Next();
                if(v!=kSourceCodeStream_EOS)
                    off++;
                if((char)v=='\n'){
                    m_stream->NewLine();
                }
            }
            
            /* not find end tag */
            if(v==kSourceCodeStream_EOS){
                m_stream->Back(off);
                return false;
            }
        }while(flag==kComment_Begin);
        /* comment ok */
        return true;
    }else{
        /* not find begin tag */
        m_stream->Back(off);
        return false;
    }
    
}
s32 Scanner::IsAlpha(s32 c)
{
    return (((char)c>='a' && (char)c<='z') ||
             ((char)c>='A' && (char)c<='Z'));
}
s32 Scanner::IsDigit(s32 c)
{
    return ((char)c>='0' && (char)c<='9');
}
void Scanner::Back(Token* t)
{
    /* it's ok when we only back one token.if we back more than one token, the tokens are not one by one char, we get wrong */
    /* calc the real offset */
    /* aaa bbb ccc */
    /*     |   |   */
    /* (m_stream->AbsPos() - t->abs_pos + 1) */
    if( m_stream->AbsPos()==t->abs_pos )
        m_stream->Back(t->len);
    else
        m_stream->Back( (m_stream->AbsPos() - t->abs_pos + 1) );
}
s32 Scanner::Next(Token* t)
{
    char sval[TOKEN_MAX_LEN];
    s32 v;
    s32 i = 0;
    
    assert(t!=0);
        
    // !! free token related memory
    t->Clear();
    
    do{
        /* blank chars */
        SkipSpace();
                
        /* new line chars */
        v = m_stream->Next();
        if((char)v=='\r'){
            v = m_stream->Next();
            if((char)v=='\n'){
                m_stream->NewLine();
            }else{
                if(v!=kSourceCodeStream_EOS)
                    m_stream->Back(1);
            }
        }
        else
        {
            if(v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
        }
        
        /* blank space or new line */
        v = m_stream->Next();
        if((char)v==' '|| 
           (char)v=='\t'||
           (char)v=='\r')
        {
            m_stream->Back(1);
            continue;
        }
        
        if(v!=kSourceCodeStream_EOS)
            m_stream->Back(1);
        
        /* comment ? */
        if(SkipComment())
            continue;
        
        /* ok, we get a valid char */
        break;    
        
    }while(1);
    
    /* lexical chars */
    v = m_stream->Next();
    t->lineno = m_stream->Lineno();
    t->pos = m_stream->Pos();
    t->abs_pos = m_stream->AbsPos();
    t->len = 1;/* default length of string. if the token string such as := <>, we need change it to 2 */
    /* string? */
    if((char)v=='"'){
        do{
            v = m_stream->Next();
        }while(v!='"' && v!=kSourceCodeStream_EOS);
        assert(v=='"');
        return kToken_STR;
    } 
    /* id or keyword */
    if(IsAlpha(v)||
       (char)v=='_')
    {
        sval[i++]=(char)v;
        do{
            v = m_stream->Next();
            sval[i++]=(char)v;
        }while(IsAlpha(v) || 
               IsDigit(v) ||
               (char)v=='_');
        sval[i-1]='\0';
        
        //record the token string length, because we need back a token supporting
        t->len = i - 1 ;
        
        if(v!=kSourceCodeStream_EOS)
            m_stream->Back(1);
        if(token_is_keyword(sval))
            return keyword_type(sval);
        else{
            t->kind = kToken_ID;
            t->u.name=strdup(sval);/* Note: memory leak */
            return kToken_ID;
        }
    }
    /* num */
    if(IsDigit(v)){
        if((char)v=='0'){
            t->kind=kToken_NUM;
            t->u.ival = 0;
            return kToken_NUM;
        }else{/* 1~9 */
            sval[i++]=(char)v;
            do{
                v = m_stream->Next();
                sval[i++]=(char)v;
            }while(IsDigit(v));
            sval[i-1]='\0';
            
            //record the token string length, because we need back a token support
            t->len = i - 1;
            
            t->kind=kToken_NUM;
            t->u.ival = atoi(sval);
            if(v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
            return kToken_NUM;
        }
    }
    
    
    /* ( */
    if((char)v=='(')
        return kToken_LPAR;
    /* ) */
    if((char)v==')')
        return kToken_RPAR;
    /* { */
    if((char)v=='{')
        return kToken_LBRA;
    /* } */
    if((char)v=='}')
        return kToken_RBRA;
    /* [ */
    if((char)v=='[')
        return kToken_LSQB;
    /* ] */
    if((char)v==']')
        return kToken_RSQB;
    
    
    
    /* + */
    if((char)v=='+'){
        return kToken_ADD;
    }
    /* - */
    if((char)v=='-')
        return kToken_SUB;
    /* * */
    if((char)v=='*')
        return kToken_MUL;
    /* / */
    if((char)v=='/')
        return kToken_DIV;
    
    
    /* , */
    if((char)v==',')
        return kToken_COMMA;
    /* : or := */
    if((char)v==':'){
        v = m_stream->Next();
        if((char)v=='='){
             t->len = 2;
            return kToken_ASSIGN;
        }else{
            if((char)v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
            
            return kToken_COLON;
        }
    }
    /* ; */
    if((char)v==';')
        return kToken_SEMICOLON;
    
    /* . */
    if((char)v=='.')
        return kToken_DOT;
    
    /* = */
    if((char)v=='=')
        return kToken_EQUAL;
    /* < or <= or <> */
    if((char)v=='<'){
        v = m_stream->Next();
        if((char)v=='='){
             t->len = 2;
            return kToken_LE;
        }else if((char)v=='>'){
             t->len = 2;
            return kToken_NOTEQUAL;
        }else{
            if((char)v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
        }
        return kToken_LT;
    }
    /* > or >= */
    if((char)v=='>'){
        v = m_stream->Next();
        if((char)v=='='){
             t->len = 2;
            return kToken_GE;
        }else{
            if((char)v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
        }
        return kToken_GT;
    }

    
    /* & */
    if((char)v=='&')
        return kToken_AND;
    /* | */
    if((char)v=='|')
        return kToken_OR;
    /* ! */
    if((char)v=='!')
        return kToken_NOT;
    
    
    /* kToken_EOT */
    if(v==kSourceCodeStream_EOS){
        t->kind=kToken_EOT;
        return kToken_EOT;
    }
    
    /* Known token */
    return kToken_Unknown;
}

} //namespace scanner
    
} // namespace tiger
