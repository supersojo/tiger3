#include <stdarg.h>
#include <stdio.h>

#include <time.h>

#include "tiger_log.h"

namespace tiger{

char* LoggerBase::LevelStrings[] = {
    "Error",
    "Debug",
    "Warn",
    "Info",
    "Invalid"
};

bool LoggerBase::Log(s32 level,char* fmt,...)
{
    return true;
}
void LoggerStdio::LogHeader(s32 level)
{
    if(GetLevel()==kLogger_Level_Error){
        if(GetModule())
            fprintf(stderr,"[%s]",GetModule());
        else
            fprintf(stderr,"[]");
        fprintf(stderr,"%s: ",LoggerBase::GetLevelString(level));
    }else{
        if(GetModule())
            fprintf(stderr,"[%s]",GetModule());
        else
            fprintf(stderr,"[]");
        fprintf(stdout,"%s: ",LoggerBase::GetLevelString(level));
    }
}
void LoggerStdio::LogEnder()
{
    if(GetLevel()==kLogger_Level_Error)
        fprintf(stderr,"\n");
    else
        fprintf(stdout,"\n");
}
bool LoggerStdio::vlog(s32 level,char* fmt,va_list args){
    s32 ret = 0;
    /* check level */
    if(GetLevel()>level)
        return false;
    
    /* log header info string */
    LogHeader(level);
    
    if(GetLevel()==kLogger_Level_Error)
        ret = vfprintf(stderr,fmt,args);
    else
        ret = vfprintf(stdout,fmt,args);
    
    LogEnder();
    
    return (ret>=0);
}
bool LoggerStdio::Log(s32 level,char* fmt,...){
    bool ret = false;

    va_list args;
    va_start(args,fmt);
    ret=vlog(level,fmt,args);
    va_end(args);
    
    return ret;
}
bool LoggerStdio::D(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Debug,fmt,args);
    va_end(args);
    return ret;
}
bool LoggerStdio::I(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Info,fmt,args);
    va_end(args);
    return ret;
}
bool LoggerStdio::W(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Warn,fmt,args);
    va_end(args);
    return ret;
}
bool LoggerStdio::E(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Error,fmt,args);
    va_end(args);
    return ret;
}
LoggerFile::LoggerFile(char* filename)
{
    m_file = fopen(filename,"a");
    SetKind(kLogger_File);
    SetLevel(kLogger_Level_Default);
}

LoggerFile::LoggerFile()
{
    char filebuf[1024];
    time_t nowtime;
    struct tm* local;
    nowtime = time(0);
    local = localtime(&nowtime);
    
    sprintf(filebuf,"%d_%d_%d_%d_%d_%d.log",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
    
    m_file = fopen(filebuf,"a");
    SetKind(kLogger_File);
    SetLevel(kLogger_Level_Default);
}

void LoggerFile::LogHeader(s32 level)
{
    if(GetModule())
        fprintf(m_file,"[%s]",GetModule());
    else
        fprintf(m_file,"[]");
    fprintf(m_file,"%s: ",LoggerBase::GetLevelString(level));
}

void LoggerFile::LogEnder()
{
    fprintf(m_file,"\n");
}

bool LoggerFile::vlog(s32 level,char* fmt,va_list args){
    s32 ret = 0;
    /* check level */
    if(GetLevel()>level)
        return false;
    
    /* log header info string */
    LogHeader(level);
    

    ret = vfprintf(m_file,fmt,args);
    
    LogEnder();
    
    return (ret>=0);
}

bool LoggerFile::Log(s32 level,char* fmt,...){
    bool ret = false;

    va_list args;
    va_start(args,fmt);
    ret=vlog(level,fmt,args);
    va_end(args);
    
    return ret;
}
bool LoggerFile::D(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Debug,fmt,args);
    va_end(args);
    return ret;
}
bool LoggerFile::I(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Info,fmt,args);
    va_end(args);
    return ret;
}
bool LoggerFile::W(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Warn,fmt,args);
    va_end(args);
    return ret;
}
bool LoggerFile::E(char* fmt,...){
    bool ret=false;
    va_list args;
    va_start(args,fmt);
    ret=vlog(kLogger_Level_Error,fmt,args);
    va_end(args);
    return ret;
}
LoggerFile::~LoggerFile(){
    if(m_file)
        fclose(m_file);
}

}//namespace tiger
