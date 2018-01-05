/* Coding: ANSI */
#ifndef SCANNER_H
#define SCANNER_H

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "token.h"



namespace tiger{

/* scanner related stuff */
namespace scanner{

enum{
    kSourceCodeStream_EOS=-1,
};

/* internal usage */
struct LineInfo{
    s32 line_size;
    
    LineInfo(){line_size = 0;prev=0;next=0;}
    
    struct LineInfo* prev;
    struct LineInfo* next;
};

/* base class for source stream */    
class SourceCodeStreamBase{
public:

    virtual s32 Next(){
        return kSourceCodeStream_EOS;
    }
    virtual void Back(s32 n){
    }
    virtual s32 Pos(){/*offset in line */
        return 0;
    }
	virtual s32 AbsPos(){/*offset in stream */
        return 0;
    }
    virtual void NewLine(){
    }
    virtual s32 Lineno(){
        return 0;
    }
};

class StringSourceCodeStream: public SourceCodeStreamBase{
public:
    StringSourceCodeStream(char* source);
    s32 Next();
    void Back(s32 n);
    s32 Pos();
	s32 AbsPos();
    s32 Lineno(){
        return m_lineno;
    }
    void NewLine(){
        m_lineno++;
        //m_off_prev = m_off;
        //m_off=0;
        /* record the last line info, line size etc. */
        LineInfo* lineinfo = new LineInfo;
        lineinfo->line_size = m_off;
        //std::cout<<"new line with size "<<m_off<<std::endl;
        m_off = 0;/* for the next line */
        
        lineinfo->prev = 0;
        lineinfo->next = m_line_info;
        if(m_line_info)
            m_line_info->prev = lineinfo;
        m_line_info = lineinfo;
        
    }
	~StringSourceCodeStream(){ FreeLineInfo(); }
private:
    char* m_string;
    s32   m_pos;
    s32   m_len;
    
    s32   m_lineno;
    s32   m_off;
    s32   m_off_prev;// only support cross most one line back
    
    LineInfo* m_line_info;
	void FreeLineInfo(){
		LineInfo* p;
		p = m_line_info;
		while(p){
			m_line_info = m_line_info->next;
			delete p;
			p = m_line_info;
		}
	}
};

class FileSourceCodeStream: public SourceCodeStreamBase{
public:
    FileSourceCodeStream(char* file);
    ~FileSourceCodeStream();
    s32 Next();
    void Back(s32 n);
    s32 Pos();
	s32 AbsPos();
    s32 Lineno(){
        return m_lineno;
    }
    void NewLine(){
        m_lineno++;
        //m_off_prev = m_off;
        //m_off=0;
        /* record the last line info, line size etc. */
        LineInfo* lineinfo = new LineInfo;
        lineinfo->line_size = m_off;
        //std::cout<<"new line with size "<<m_off<<std::endl;
        m_off = 0;/* for the next line */
        
        lineinfo->prev = 0;
        lineinfo->next = m_line_info;
        if(m_line_info)
            m_line_info->prev = lineinfo;
        m_line_info = lineinfo;
        
    }
private:
    FILE* m_file;
    s32   m_len;
    
    s32   m_lineno;
    s32   m_off;
    s32   m_off_prev;// only support cross most one line back
    
    LineInfo* m_line_info;
	void FreeLineInfo(){
		LineInfo* p;
		p = m_line_info;
		while(p){
			m_line_info = m_line_info->next;
			delete p;
			p = m_line_info;
		}
	}
};

class Scanner {
public:
    Scanner(SourceCodeStreamBase* stream);
    void   Init();
    s32 Next(Token* t);
    void Back(Token* t);
    SourceCodeStreamBase* GetSourceCodeStream(){return m_stream;}
    
private:
    void SkipSpace();
    bool SkipComment();
    s32 IsAlpha(s32 c);
    s32 IsDigit(s32 c);

    s32 lineno;
    
    SourceCodeStreamBase* m_stream;
};


} // namespace scanner

} // namespace tiger

#endif
